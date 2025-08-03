#include "GameCenter-android.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)

#include "platform/android/jni/JniHelper.h"
#include "common/Machine.h"
#include "game/GoogleServicesUtils.h"
#include "game/firservice/FirebaseService.h"
#include "LocalPlayer.h"

NS_BEGIN

#define GPG_REQUEST_CODE                    4673607
#define ACTIVITY_RESULT_CANCELED            0
#define ACTIVITY_RESULT_OK                  -1
#define GOOGLE_PLAY_GAMES_PACKAGE_NAME      "com.google.android.play.games"

static const std::string gameCenterHelperClassName = "io/gamen/snowfight/GameCenterHelper";

static gpg::AndroidPlatformConfiguration s_platformConfig;
static GameCenterImpl* s_instance = nullptr;

extern "C" {
    JNIEXPORT void
    JNICALL Java_io_gamen_snowfight_GameCenterHelper_nativeOnActivityCreate(JNIEnv* env, jobject thiz, jobject activity, jobject saved_instance_state) {
        s_platformConfig.SetActivity(activity);
        NS_ASSERT_LOG(s_platformConfig.Valid(), "Not providing required values for AndroidPlatformConfiguration.");
    }

    JNIEXPORT void
    JNICALL Java_io_gamen_snowfight_GameCenterHelper_nativeOnActivityResult(JNIEnv *env, jobject thiz, jobject activity, jint requestCode, jint resultCode, jobject data) {
        if (requestCode == GPG_REQUEST_CODE)
            if(resultCode == ACTIVITY_RESULT_CANCELED)
                if (s_instance)
                    s_instance->setAuthProgress(GameCenterImpl::AUTH_PROGRESS_CANCELED);

        gpg::AndroidSupport::OnActivityResult(env, activity, requestCode, resultCode, data);
    }
} // extern "C"


GameCenterError getGameCenterErrorWithAuthStatus(gpg::AuthStatus status)
{
    GameCenterError gcError;

    switch(status) {
        case gpg::AuthStatus::VALID:
            gcError.code = GAMECENTER_ERROR_NONE;
            break;
        case gpg::AuthStatus::ERROR_INTERRUPTED:
            gcError.code = GAMECENTER_ERROR_INTERRUPTED;
            break;
        case gpg::AuthStatus::ERROR_NOT_AUTHORIZED:
            gcError.code = GAMECENTER_ERROR_NOT_AUTHENTICATED;
            break;
        case gpg::AuthStatus::ERROR_NETWORK_OPERATION_FAILED:
        case gpg::AuthStatus::ERROR_NO_DATA:
            gcError.code = GAMECENTER_ERROR_COMMUNICATIONS_FAILURE;
            break;
        case gpg::AuthStatus::ERROR_TIMEOUT:
            gcError.code = GAMECENTER_ERROR_TIMEOUT;
            break;
        case gpg::AuthStatus::ERROR_VERSION_UPDATE_REQUIRED:
            gcError.code = GAMECENTER_ERROR_GPS_VERSION_UPDATE_REQUIRED;
            break;
        default:
            gcError.code = GAMECENTER_ERROR_UNKNOWN;
            break;
    }

    return gcError;
}

AuthOperation getAuthOperationWithGPGAuthOperation(gpg::AuthOperation op)
{
    AuthOperation authOp;
    switch (op) {
        case gpg::AuthOperation::SIGN_IN:
            authOp = AuthOperation::SIGN_IN;
            break;
        default: // gpg::AuthOperation::SIGN_OUT
            authOp = AuthOperation::SIGN_OUT;
            break;
    }
    return authOp;
}


GameCenterError getGameCenterErrorWithResponseStatus(gpg::ResponseStatus status)
{
    GameCenterError gcError;

    switch(status) {
        case gpg::ResponseStatus::VALID:
            gcError.code = GAMECENTER_ERROR_NONE;
            break;
        case gpg::ResponseStatus::ERROR_INTERRUPTED:
            gcError.code = GAMECENTER_ERROR_INTERRUPTED;
            break;
        case gpg::ResponseStatus::ERROR_NOT_AUTHORIZED:
            gcError.code = GAMECENTER_ERROR_NOT_AUTHENTICATED;
            break;
        case gpg::ResponseStatus::ERROR_NETWORK_OPERATION_FAILED:
        case gpg::ResponseStatus::ERROR_NO_DATA:
        case gpg::ResponseStatus::VALID_BUT_STALE:
        case gpg::ResponseStatus::DEFERRED:
            gcError.code = GAMECENTER_ERROR_COMMUNICATIONS_FAILURE;
            break;
        case gpg::ResponseStatus::ERROR_TIMEOUT:
            gcError.code = GAMECENTER_ERROR_TIMEOUT;
            break;
        case gpg::ResponseStatus::ERROR_VERSION_UPDATE_REQUIRED:
            gcError.code = GAMECENTER_ERROR_GPS_VERSION_UPDATE_REQUIRED;
            break;
        default:
            gcError.code = GAMECENTER_ERROR_UNKNOWN;
            break;
    }

    return gcError;
}

GameCenterImpl::GameCenterImpl() :
    m_localPlayer(new LocalPlayer()),
    m_authProgress(AUTH_PROGRESS_NONE)
{
    s_instance = this;
}

GameCenterImpl::~GameCenterImpl()
{
    CC_SAFE_DELETE(m_localPlayer);
    s_instance = nullptr;
}

void GameCenterImpl::authLocalPlayer()
{
    int32 errorCode = GoogleServicesUtils::checkGooglePlayServices();
    if(errorCode == ServicesConnectionResult::SERVICE_SUCCESS)
    {
        this->initServices();
    }
    else
    {
        sAnalytics->setSignUpMethod(kSignUpMethodNone);
        m_localPlayer->loadDataAsync([this](bool ret) {
            sAnalytics->setUserId(m_localPlayer->getPlayerID().c_str());
            if(!m_listeners.empty())
            {
                AuthResponse resp;
                if (ret) {
                    resp.state = AUTH_STATE_SUCCESS;
                } else {
                    resp.state = AUTH_STATE_FAIL;
                    resp.error.code = GAMECENTER_ERROR_LOAD_DATA_FAILED;
                }

                for(GameCenterListener* listen: m_listeners)
                    listen->onAuthFinished(AuthOperation::SIGN_IN, resp);
            }
        });
    }
}

LocalPlayer *GameCenterImpl::getLocalPlayer() const
{
	return m_localPlayer;
}

void GameCenterImpl::setShowAuthDialog(bool isShowAuthDialog)
{
}

void GameCenterImpl::showLeaderboard(std::string const& leaderboardID)
{
    if(m_gameServices)
    {
        m_gameServices->Leaderboards().ShowUI(leaderboardID, gpg::LeaderboardTimeSpan::DAILY, [leaderboardID](gpg::UIStatus status)
        {
            CCLOG("Leaderboards ID: %s, UIStatus: %d", leaderboardID.c_str(), status);
        });
    }
}

bool GameCenterImpl::reportScores(std::string const& leaderboardID)
{
    if(!m_gameServices|| !m_gameServices->IsAuthorized())
        return false;

    int32 scoreValue = m_localPlayer->getScoreValue(leaderboardID);
    if(scoreValue <= 0 || m_localPlayer->isScoreReported(leaderboardID))
        return false;

    m_gameServices->Leaderboards().SubmitScore(leaderboardID, scoreValue);
    m_localPlayer->setScoreReported(leaderboardID, true);
    m_localPlayer->saveDataAsync([this](bool ret){
        if (!m_listeners.empty()) {
            if(ret)
            {
                for(GameCenterListener* listen: m_listeners)
                    listen->onReportScoreSuccess();
            }
            else {
                GameCenterError error;
                error.code = GAMECENTER_ERROR_SAVE_DATA_FAILED;
                for(GameCenterListener* listen: m_listeners)
                    listen->onReportScoreFail(error);
            }
        }
    });

	return true;
}

void GameCenterImpl::addListener(GameCenterListener* listener)
{
    auto it = std::find(m_listeners.begin(), m_listeners.end(), listener);
    if(it == std::end(m_listeners))
        m_listeners.push_back(listener);
}

void GameCenterImpl::removeListener(GameCenterListener* listener)
{
    m_listeners.erase(std::remove(m_listeners.begin(), m_listeners.end(), listener), m_listeners.end());
}

void GameCenterImpl::setEnteringForeground(bool isEnteringForeground)
{
}

void GameCenterImpl::onAuthActionStarted(gpg::AuthOperation op)
{
    m_authProgress = AUTH_PROGRESS_STARTED;
    AuthOperation authOp = getAuthOperationWithGPGAuthOperation(op);
    Director::getInstance()->getScheduler()->performFunctionInCocosThread([this, authOp]() {
        for(GameCenterListener* listen: m_listeners)
            listen->onAuthStarted(authOp);
    });
}

void GameCenterImpl::onAuthSuccess(AuthOperation op, gpg::PlayerManager::FetchSelfResponse const& selfResp)
{
    std::string oldPlayerID = m_localPlayer->getPlayerID();
    std::string playerID = selfResp.data.Id();
    std::string displayName = selfResp.data.Name();

    // 是不同的玩家登录
    bool diffPlayer = oldPlayerID != playerID;
    // 玩家切换
    if(diffPlayer && !m_localPlayer->isGuest() && !oldPlayerID.empty()) {
        AuthResponse resp;
        resp.state = AUTH_STATE_FAIL;
        resp.error.code = GAMECENTER_ERROR_ACCOUNT_CHANGED;
        for(GameCenterListener* listen: m_listeners)
            listen->onAuthFinished(AuthOperation::SIGN_IN, resp);
    }
    else {
        m_localPlayer->setupDefaultValues();

        m_localPlayer->setAuthenticated(true);
        m_localPlayer->setDisplayName(displayName);
        m_localPlayer->setPlayerID(playerID);

        sAnalytics->setSignUpMethod(kSignUpMethodGoogle);
        m_localPlayer->loadDataAsync([this, op](bool ret) {
            sAnalytics->setUserId(m_localPlayer->getPlayerID().c_str());
            sAnalytics->logLogin();
            if (!m_listeners.empty()) {
                AuthResponse resp;
                if (ret) {
                    resp.state = AUTH_STATE_SUCCESS;
                } else {
                    resp.state = AUTH_STATE_FAIL;
                    resp.error.code = GAMECENTER_ERROR_LOAD_DATA_FAILED;
                }
                for(GameCenterListener* listen: m_listeners)
                    listen->onAuthFinished(op, resp);
            }
        });
    }
}

void GameCenterImpl::onAuthFailed(AuthOperation op, GameCenterError error)
{
    // 未登录PlayGames状态下保留已加载的本地玩家数据
    bool diffPlayer = m_localPlayer->getPlayerID().empty();
    if(diffPlayer) {
        m_localPlayer->setupDefaultValues();
    }

    m_localPlayer->setAuthenticated(false);

    if(diffPlayer) {
        sAnalytics->setSignUpMethod(kSignUpMethodNone);
        m_localPlayer->loadDataAsync([this, op, error](bool ret) {
            sAnalytics->setUserId(m_localPlayer->getPlayerID().c_str());
            if (!m_listeners.empty()) {
                AuthResponse resp;
                resp.state = AuthState::AUTH_STATE_FAIL;
                if (ret)
                    resp.error = error;
                else
                    resp.error.code = GAMECENTER_ERROR_LOAD_DATA_FAILED;
                for(GameCenterListener* listen: m_listeners)
                    listen->onAuthFinished(op, resp);
            }
        });
    } else {
        if (!m_listeners.empty()) {
            AuthResponse resp;
            resp.state = AuthState::AUTH_STATE_FAIL;
            resp.error = error;
            for(GameCenterListener* listen: m_listeners)
                listen->onAuthFinished(op, resp);
        }
    }
}

void GameCenterImpl::onAuthActionFinished(gpg::AuthOperation op, gpg::AuthStatus status)
{
    CCLOG("Sign %s finished with a result of %d", op == gpg::AuthOperation::SIGN_IN ? "in" : "out", status);
    AuthOperation authOp = getAuthOperationWithGPGAuthOperation(op);
    if(gpg::IsSuccess(status)) {
        CCLOG("authorized: %d",  m_gameServices->IsAuthorized());
        m_gameServices->Players().FetchSelf([this, authOp](gpg::PlayerManager::FetchSelfResponse const& selfResp) {
            m_authProgress = AUTH_PROGRESS_FINISHED;
            if (gpg::IsSuccess(selfResp.status)) {
                CCLOG("player name: %s, id: %s valid: %d", selfResp.data.Name().c_str(),selfResp.data.Id().c_str(), selfResp.data.Valid());
                Director::getInstance()->getScheduler()->performFunctionInCocosThread([this, selfResp, authOp](){
                    if(selfResp.data.Valid())
                        this->onAuthSuccess(authOp, selfResp);
                    else
                    {
                        GameCenterError error;
                        error.code = GAMECENTER_ERROR_PLAYER_INVALIDED;
                        this->onAuthFailed(authOp, error);
                    }
                });
            } else {
                Director::getInstance()->getScheduler()->performFunctionInCocosThread([this, selfResp, authOp](){
                    this->onAuthFailed(authOp, getGameCenterErrorWithResponseStatus(selfResp.status));
                });
            }
        });
    } else {
        GameCenterError gcError;
        if(m_authProgress == AUTH_PROGRESS_CANCELED)
            gcError.code = GAMECENTER_ERROR_CANCELED;
        else {
            gcError = getGameCenterErrorWithAuthStatus(status);
            m_authProgress = AUTH_PROGRESS_FINISHED;
        }
        Director::getInstance()->getScheduler()->performFunctionInCocosThread([this, gcError, authOp](){
            this->onAuthFailed(authOp, gcError);
        });
    }
}

void GameCenterImpl::initServices()
{
    if (!m_gameServices) {
        CCLOG("Initializing GameServices");
        m_gameServices = gpg::GameServices::Builder()
#if NS_DEBUG
                .SetOnLog(gpg::DEFAULT_ON_LOG, gpg::LogLevel::VERBOSE)
#endif // NS_DEBUG
                .SetOnAuthActionStarted([this](gpg::AuthOperation op)
                {
                    this->onAuthActionStarted(op);
                })
                .SetOnAuthActionFinished([this](gpg::AuthOperation op, gpg::AuthStatus status)
                {
                    this->onAuthActionFinished(op, status);
                })
                .SetEnableSilentSignInResolution(true)
                .Create(s_platformConfig);
    }
}

void GameCenterImpl::signOut()
{
    if(m_gameServices)
        if(m_gameServices->IsAuthorized())
            m_gameServices->SignOut();
}

bool GameCenterImpl::startAuthorizationUI()
{
    int32 errorCode = GoogleServicesUtils::checkGooglePlayServices();
    if(errorCode == ServicesConnectionResult::SERVICE_SUCCESS)
    {
        if(m_gameServices)
        {
            if(!m_localPlayer->isAuthenticated() && !this->isAuthorizing())
            {
                if(m_gameServices->IsAuthorized())
                {
                    this->onAuthActionStarted(gpg::AuthOperation::SIGN_IN);
                    this->onAuthActionFinished(gpg::AuthOperation::SIGN_IN, gpg::AuthStatus::VALID);
                }
                else
                    m_gameServices->StartAuthorizationUI();

                return true;
            }
        }
        else
        {
            this->initServices();
            return true;
        }

    }

    return false;
}

NS_END

#endif // CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID