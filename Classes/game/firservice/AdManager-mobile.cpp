//
//  AdManager-mobile.cpp
//  snowfight
//
// Created by Luthier on 2020/8/12.
//

#include "AdManager-mobile.h"

#include "firebase/future.h"

#include "audio/include/AudioEngine.h"
#include "FirebaseHelper.h"
#include "common/utils/TimeUtil.h"

USING_NS_CC;

NS_BEGIN

// 激活测试广告单元
#define ENABLE_TEST_AD_UNITS         0

#if NS_DEBUG

// 测试设备ID
// 注：只有列表中的设备可以点击正式广告。
//     如果非测试设备点击过多正式广告，则可能会导致帐号因为无效活动而被举报。
static const std::vector<std::string> TEST_DEVICE_IDS = {
};

#else

#if ENABLE_TEST_AD_UNITS
#warning "Test Ad units are not in debug mode."

#endif // ENABLE_TEST_AD_UNITS

#endif // NS_DEBUG


#if ENABLE_TEST_AD_UNITS
// 测试广告单元ID
// 注：测试广告可以随意点击。
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
const char* INTERSTITIAL_AD_UNIT = "ca-app-pub-3940256099942544/1033173712";
const char* REWARDED_VIDEO_AD_UNIT = "ca-app-pub-3940256099942544/5224354917";
#else
const char* INTERSTITIAL_AD_UNIT = "ca-app-pub-3940256099942544/4411468910";
const char* REWARDED_VIDEO_AD_UNIT = "ca-app-pub-3940256099942544/1712485313";
#endif

#else
// 正式广告单元ID
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
const char* INTERSTITIAL_AD_UNIT = "ca-app-pub-9483765979334448/6537779885";
static std::unordered_map<int32, char const*> s_rewardedVideoAdUnits = {
        { RewardedAdConfig::AD_TYPE_FREE_GOLDS,       "ca-app-pub-9483765979334448/4423253796" },
        { RewardedAdConfig::AD_TYPE_DAILY_REWARD,     "ca-app-pub-9483765979334448/1766114529" },
        { RewardedAdConfig::AD_TYPE_BATTLE_OUTCOME,   "ca-app-pub-9483765979334448/7948379498" },
};
#else
const char* INTERSTITIAL_AD_UNIT = "ca-app-pub-9483765979334448/7011356944";
static std::unordered_map<int32, char const*> s_rewardedVideoAdUnits = {
        { RewardedAdConfig::AD_TYPE_FREE_GOLDS,       "ca-app-pub-9483765979334448/9267536305" },
        { RewardedAdConfig::AD_TYPE_DAILY_REWARD,     "ca-app-pub-9483765979334448/9999827761" },
        { RewardedAdConfig::AD_TYPE_BATTLE_OUTCOME,   "ca-app-pub-9483765979334448/6060582753" },
};
#endif

#endif // ENABLE_TEST_AD_UNITS


// 描述当前用户活动的一组关键字或短语
static std::vector<const char*> KEYWORDS = {
        "Snowball fight",
        "Winter game",
        "Action game",
};

class RewardedVideoStateListener :  public firebase::gma::FullScreenContentListener,
                                    public firebase::gma::UserEarnedRewardListener
{
public:
    RewardedVideoStateListener(AdManagerImpl* impl) :
        m_impl(impl)
    {
    }
    
    ~RewardedVideoStateListener()
    {
        m_impl = nullptr;
    }

    // 对于GoogleAds，所有OnUserEarnedReward()调用都发生在OnAdDismissedFullScreenContent()之前。
    // 对于通过中介投放的广告，由第三方广告联盟SDK的实现情况决定回调顺序。对于为单个委托方法提供奖励信息的广告联盟SDK，
    // 中介适配器会在调用OnAdDismissedFullScreenContent()之前调用OnUserEarnedReward()。
    // https://developers.google.com/admob/ios/rewarded#faq
    void OnUserEarnedReward(firebase::gma::AdReward const& reward) override
    {
        CCLOG("AdManagerImpl: Rewarding user with %" PRId64 " %s", reward.amount(), reward.type().c_str());
        cocos2d::Director::getInstance()->getScheduler()->performFunctionInCocosThread([this, reward]() {
            if(m_impl)
                m_impl->onUserEarnedReward(reward);
        });
    }

    void OnAdClicked() override
    {
        CCLOG("AdManagerImpl: RewardedVideoStateListener::OnAdClicked()");
    }

    void OnAdDismissedFullScreenContent() override
    {
        CCLOG("AdManagerImpl: RewardedVideoStateListener::OnAdDismissedFullScreenContent()");
        FirebaseHelper::resumeGame();
        cocos2d::Director::getInstance()->getScheduler()->performFunctionInCocosThread([this]() {
            if(m_impl)
                m_impl->onRewardedVideoDismissed();
        });
    }

    void OnAdFailedToShowFullScreenContent(firebase::gma::AdError const& adError) override
    {
        CCLOG("AdManagerImpl: RewardedVideoStateListener::OnAdFailedToShowFullScreenContent() code: %d message: %s", adError.code(), adError.message().c_str());
        cocos2d::Director::getInstance()->getScheduler()->performFunctionInCocosThread([this, adError]() {
            if(m_impl)
                m_impl->onRewardedVideoFailedToShow(adError);
        });
    }

    void OnAdImpression() override
    {
        CCLOG("AdManagerImpl: RewardedVideoStateListener::OnAdImpression()");
    }

    void OnAdShowedFullScreenContent() override
    {
        CCLOG("AdManagerImpl: RewardedVideoStateListener::OnAdShowedFullScreenContent()");
        FirebaseHelper::pauseGame();
    }

private:
    AdManagerImpl* m_impl;
};

class InterstitialAdStateListener : public firebase::gma::FullScreenContentListener
{
public:
    InterstitialAdStateListener(AdManagerImpl* impl) :
            m_impl(impl)
    {
    }

    ~InterstitialAdStateListener()
    {
        m_impl = nullptr;
    }

    void OnAdClicked() override
    {
        CCLOG("AdManagerImpl: InterstitialAdStateListener::OnAdClicked()");
    }

    void OnAdDismissedFullScreenContent() override
    {
        CCLOG("AdManagerImpl: InterstitialAdStateListener::OnAdDismissedFullScreenContent()");
        FirebaseHelper::resumeGame();
        cocos2d::Director::getInstance()->getScheduler()->performFunctionInCocosThread([this]() {
            if(m_impl)
                m_impl->onInterstitialAdDismissed();
        });
    }

    void OnAdFailedToShowFullScreenContent(firebase::gma::AdError const& adError) override
    {
        CCLOG("AdManagerImpl: InterstitialAdStateListener::OnAdFailedToShowFullScreenContent() code: %d message: %s", adError.code(), adError.message().c_str());
        cocos2d::Director::getInstance()->getScheduler()->performFunctionInCocosThread([this, adError]() {
            if(m_impl)
                m_impl->onInterstitialAdFailedToShow(adError);
        });
    }

    void OnAdImpression() override
    {
        CCLOG("AdManagerImpl: InterstitialAdStateListener::OnAdImpression()");
    }

    void OnAdShowedFullScreenContent() override
    {
        CCLOG("AdManagerImpl: InterstitialAdStateListener::OnAdShowedFullScreenContent()");
        FirebaseHelper::pauseGame();
    }

private:
    AdManagerImpl* m_impl;
};

AdManagerImpl::AdManagerImpl() :
    m_isInitialized(false),
    m_isInitializing(false),
    m_isRewardedVideoInProgress(false),
    m_isEarnedReward(false),
    m_rewardedAd(nullptr),
    m_rewardedVideoStateListener(nullptr),
    m_isInterstitialAdLoaded(false),
    m_interstitialAd(nullptr),
    m_interstitialAdStateListener(nullptr)
{

}

AdManagerImpl::~AdManagerImpl()
{
    CC_SAFE_DELETE(m_rewardedAd);
    CC_SAFE_DELETE(m_rewardedVideoStateListener);
    CC_SAFE_DELETE(m_interstitialAd);
    CC_SAFE_DELETE(m_interstitialAdStateListener);

    firebase::gma::Terminate();
}

void AdManagerImpl::init(firebase::App* app)
{
    if(m_isInitialized || m_isInitializing)
        return;

    m_isInitializing = true;

    // 初始化GMA
    firebase::InitResult result;
    firebase::gma::Initialize(*app, &result);
    if (result == firebase::kInitResultSuccess)
    {
        // 向广告请求提供目标信息
        https://developers.google.com/admob/cpp/targeting
        firebase::gma::RequestConfiguration configuration = firebase::gma::GetRequestConfiguration();
        configuration.max_ad_content_rating = firebase::gma::RequestConfiguration::kMaxAdContentRatingUnspecified;
        configuration.tag_for_child_directed_treatment = firebase::gma::RequestConfiguration::kChildDirectedTreatmentUnspecified;
        configuration.tag_for_under_age_of_consent = firebase::gma::RequestConfiguration::kUnderAgeOfConsentUnspecified;
#if NS_DEBUG
        configuration.test_device_ids = TEST_DEVICE_IDS;
#endif
        firebase::gma::SetRequestConfiguration(configuration);

        firebase::gma::InitializeLastResult().OnCompletion(handleResultInCocosThread<firebase::gma::AdapterInitializationStatus>([this](int32 error, const char* message) {
           if (error == firebase::gma::kAdErrorCodeNone)
           {
               m_isInitializing = false;
               m_isInitialized = true;
           }
           else
           {
               CCLOG("AdManagerImpl: Failed to initialize GMA.");
           }
       }));
    }
    else // firebase::kInitResultFailedMissingDependency
    {
        CCLOG("AdManagerImpl: GMA failed to initialize due to missing dependency.");
    }
}

void AdManagerImpl::addRewardedAdListener(RewardedAdConfig::AdType adType, RewardedAdListener* listener)
{
    m_rewardedAdListeners[adType] = listener;
}

void AdManagerImpl::removeRewardedAdListener(RewardedAdListener* listener)
{
    for(auto it = m_rewardedAdListeners.begin(); it != m_rewardedAdListeners.end();)
    {
        if((*it).second == listener)
            it = m_rewardedAdListeners.erase(it);
        else
            ++it;
    }
}

AdErrorCode AdManagerImpl::getAdErrorCode(int32 gmaAdErrorCode)
{
    AdErrorCode errorCode;
    switch(gmaAdErrorCode)
    {
    case firebase::gma::kAdErrorCodeNone:
        errorCode = AD_ERROR_NONE;
        break;
    case firebase::gma::kAdErrorCodeUninitialized:
        errorCode = AD_ERROR_UNINITIALIZED;
        break;
    case firebase::gma::kAdErrorCodeInternalError:
        errorCode = AD_ERROR_INTERNAL_ERROR;
        break;
    case firebase::gma::kAdErrorCodeInvalidRequest:
        errorCode = AD_ERROR_INVALID_REQUEST;
        break;
    case firebase::gma::kAdErrorCodeNetworkError:
        errorCode = AD_ERROR_NETWORK_ERROR;
        break;
    case firebase::gma::kAdErrorCodeNoFill:
        errorCode = AD_ERROR_NO_FILL;
        break;
    case firebase::gma::kAdErrorCodeNoWindowToken:
        errorCode = AD_ERROR_NO_WINDOW_TOKEN;
        break;
    case firebase::gma::kAdErrorCodeAdNetworkClassLoadError:
        errorCode = AD_ERROR_AD_NETWORK_CLASS_LOAD_ERROR;
        break;
    case firebase::gma:: kAdErrorCodeServerError:
        errorCode = AD_ERROR_SERVER_ERROR;
        break;
    case firebase::gma::kAdErrorCodeOSVersionTooLow:
        errorCode = AD_ERROR_OS_VERSION_TOO_LOW;
        break;
    case firebase::gma::kAdErrorCodeTimeout:
        errorCode = AD_ERROR_TIMEOUT;
        break;
    case firebase::gma::kAdErrorCodeInterstitialAlreadyUsed:
        errorCode = AD_ERROR_INTERSTITIAL_ALREADY_USED;
        break;
    case firebase::gma::kAdErrorCodeMediationDataError:
        errorCode = AD_ERROR_MEDIATION_DATA_ERROR;
        break;
    case firebase::gma::kAdErrorCodeMediationAdapterError:
        errorCode = AD_ERROR_MEDIATION_ADAPTER_ERROR;
        break;
    case firebase::gma::kAdErrorCodeMediationInvalidAdSize:
        errorCode = AD_ERROR_MEDIATION_INVALID_AD_SIZE;
        break;
    case firebase::gma::kAdErrorCodeInvalidArgument:
        errorCode = AD_ERROR_INVALID_ARGUMENT;
        break;
    case firebase::gma::kAdErrorCodeReceivedInvalidResponse:
        errorCode = AD_ERROR_RECEIVED_INVALID_RESPONSE;
        break;
    case firebase::gma::kAdErrorCodeRewardedAdAlreadyUsed:
        errorCode = AD_ERROR_REWARDED_AD_ALREADY_USED;
        break;
    case firebase::gma::kAdErrorCodeMediationNoFill:
        errorCode = AD_ERROR_MEDIATION_NO_FILL;
        break;
    case firebase::gma::kAdErrorCodeAdAlreadyUsed:
        errorCode = AD_ERROR_AD_ALREADY_USED;
        break;
    case firebase::gma::kAdErrorCodeApplicationIdentifierMissing:
        errorCode = AD_ERROR_APPLICATION_IDENTIFIER_MISSING;
        break;
    case firebase::gma::kAdErrorCodeInvalidAdString:
        errorCode = AD_ERROR_INVALID_AD_STRING;
        break;
    case firebase::gma::kAdErrorCodeAppNotInForeground:
        errorCode = AD_ERROR_APP_NOT_IN_FOREGROUND;
        break;
    case firebase::gma::kAdErrorCodeMediationShowError:
        errorCode = AD_ERROR_MEDIATION_SHOW_ERROR;
        break;
    case firebase::gma::kAdErrorCodeAdNotReady:
        errorCode = AD_ERROR_AD_NOT_READY;
        break;
    case firebase::gma::kAdErrorCodeAdTooLarge:
        errorCode = AD_ERROR_AD_TOO_LARGE;
        break;
    case firebase::gma::kAdErrorCodeNotMainThread:
        errorCode = AD_ERROR_NOT_MAIN_THREAD;
        break;
    case firebase::gma::kAdErrorCodeNotInTestMode:
        errorCode = AD_ERROR_NOT_IN_TEST_MODE;
        break;
    case firebase::gma::kAdErrorCodeInspectorFailedToLoad:
        errorCode = AD_ERROR_INSPECTOR_FAILED_TO_LOAD;
        break;
    case firebase::gma::kAdErrorCodeInsepctorAlreadyOpen:
        errorCode = AD_ERROR_INSEPCTOR_ALREADY_OPEN;
        break;
    case firebase::gma::kAdErrorCodeUnknown:
        errorCode = AD_ERROR_UNKNOWN;
        break;
    }
    
    return errorCode;
}

firebase::gma::AdRequest AdManagerImpl::createAdRequest()
{
    firebase::gma::AdRequest request;
    for(auto keyword: KEYWORDS)
        request.add_keyword(keyword);

    return request;
}

bool AdManagerImpl::requestShowRewardedVideo(RewardedAdConfig const& adConfig)
{
    if(m_isRewardedVideoInProgress)
        return false;

    m_isEarnedReward = false;
    m_isRewardedVideoInProgress = true;
    m_rewardedAdConfig = adConfig;

    if(m_rewardedAd)
        CC_SAFE_DELETE(m_rewardedAd);
    if(m_rewardedVideoStateListener)
        CC_SAFE_DELETE(m_rewardedVideoStateListener);

    m_rewardedAd = new firebase::gma::RewardedAd();
    firebase::gma::AdParent adParent = static_cast<firebase::gma::AdParent>(FirebaseHelper::getAdParent());
    m_rewardedAd->Initialize(adParent);
    m_rewardedAd->InitializeLastResult().OnCompletion(handleResultInCocosThread<void>([this](int32 error, const char* message){
        if(error == firebase::gma::kAdErrorCodeNone)
        {
            this->loadRewardedVideo();
        }
        else
        {
            CCLOG("AdManagerImpl: Failed to initialize rewarded video. error: %d msg: %s", error, message);
            m_isRewardedVideoInProgress = false;

            this->handleRewardedVideoError(error, message);
        }
    }));

    return true;
}

void AdManagerImpl::addInterstitialAdListener(InterstitialAdListener* listener)
{
    auto it = std::find(m_interstitialAdListeners.begin(), m_interstitialAdListeners.end(), listener);
    if(it == m_interstitialAdListeners.end())
        m_interstitialAdListeners.push_back(listener);
}

void AdManagerImpl::removeInterstitialAdListener(InterstitialAdListener* listener)
{
    auto it = std::find(m_interstitialAdListeners.begin(), m_interstitialAdListeners.end(), listener);
    if(it != m_interstitialAdListeners.end())
        m_interstitialAdListeners.erase(it);
}

bool AdManagerImpl::requestInterstitialAd()
{
    if(m_interstitialAd)
        CC_SAFE_DELETE(m_interstitialAd);
    if(m_interstitialAdStateListener)
        CC_SAFE_DELETE(m_interstitialAdStateListener);

    m_interstitialAd = new firebase::gma::InterstitialAd();
    m_interstitialAd->Initialize(static_cast<firebase::gma::AdParent>(FirebaseHelper::getAdParent()));
    m_interstitialAd->InitializeLastResult().OnCompletion(handleResultInCocosThread<void>([this](int32 error, const char* message) {
        if (error == firebase::gma::kAdErrorCodeNone)
        {
            this->loadInterstitialAd();
        }
        else
        {
            CCLOG("AdManagerImpl: Failed to initialize interstitial ad. error: %d msg: %s", error, message);
            this->handleInterstitialAdError(error, message);
        }
    }));

    return true;
}

bool AdManagerImpl::showInterstitialAd()
{
    if(!m_isInterstitialAdLoaded)
        return false;

    m_interstitialAd->Show();
    m_interstitialAd->ShowLastResult().OnCompletion(handleResultInCocosThread<void>([this](int32 error, const char* message) {
        if (error != firebase::gma::kAdErrorCodeNone)
        {
            CCLOG("AdManagerImpl: Failed to show interstitial ad. error: %d msg: %s", error, message);
            m_isInterstitialAdLoaded = false;
            this->handleInterstitialAdError(error, message);
        }
    }));

    return true;
}

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)

bool AdManagerImpl::isNeedTrackingAuthorization()const
{
    return FirebaseHelper::isNeedTrackingAuthorization();
}

void AdManagerImpl::requestIDFA(std::function<void(bool)> const& callback)
{
    FirebaseHelper::requestIDFA(callback);
}

#endif

void AdManagerImpl::loadRewardedVideo()
{
    if(!m_rewardedAd)
        return;

    if(!m_rewardedVideoStateListener)
        m_rewardedVideoStateListener = new RewardedVideoStateListener(this);
    m_rewardedAd->SetFullScreenContentListener(m_rewardedVideoStateListener);

    firebase::gma::AdRequest request = this->createAdRequest();
    char const* adUnit = nullptr;
#if ENABLE_TEST_AD_UNITS
    adUnit = REWARDED_VIDEO_AD_UNIT;
#else
    adUnit = s_rewardedVideoAdUnits[m_rewardedAdConfig.adType];
#endif
    m_rewardedAd->LoadAd(adUnit, request);
    m_rewardedAd->LoadAdLastResult().OnCompletion(handleResultInCocosThread<firebase::gma::AdResult>([this](int32 error, const char* message) {
        if (error == firebase::gma::kAdErrorCodeNone)
        {
            this->showRewardedVideo();
        }
        else
        {
            CCLOG("AdManagerImpl: Failed to load rewarded video. error: %d msg: %s", error, message);
            m_isRewardedVideoInProgress = false;

            this->handleRewardedVideoError(error, message);
        }
    }));
}

void AdManagerImpl::showRewardedVideo()
{
    if(!m_rewardedAd)
        return;

    NS_ASSERT(m_rewardedVideoStateListener);

    m_rewardedAd->Show(m_rewardedVideoStateListener);
    m_rewardedAd->ShowLastResult().OnCompletion(handleResultInCocosThread<void>([this](int32 error, const char* message) {
        if (error != firebase::gma::kAdErrorCodeNone)
        {
            CCLOG("AdManagerImpl: Failed to show rewarded video. error: %d msg: %s", error, message);
            m_isRewardedVideoInProgress = false;

            this->handleRewardedVideoError(error, message);
        }
    }));
}

void AdManagerImpl::handleRewardedVideoError(int32 error, std::string const& message)
{
    if(error == firebase::gma::kAdErrorCodeAlreadyInitialized)
    {
        CCLOG("AdManagerImpl: The ad is already initialized (repeat call).");
        return;
    }
    
    if(error == firebase::gma::kAdErrorCodeLoadInProgress)
    {
        CCLOG("AdManagerImpl: A call has failed because an ad is currently loading.");
        return;
    }

    auto it = m_rewardedAdListeners.find(m_rewardedAdConfig.adType);
    if(it == m_rewardedAdListeners.end())
        return;
    
    AdError adError;
    adError.errorCode = getAdErrorCode(error);
    adError.errorMsg = message;
    (*it).second->onRewardedAdRequestFailed(m_rewardedAdConfig, adError);
}

void AdManagerImpl::onUserEarnedReward(firebase::gma::AdReward const& reward)
{
    m_isEarnedReward = true;

    auto it = m_rewardedAdListeners.find(m_rewardedAdConfig.adType);
    if (it != m_rewardedAdListeners.end())
        (*it).second->onUserEarnedReward(m_rewardedAdConfig);
}

void AdManagerImpl::onRewardedVideoDismissed()
{
    m_isRewardedVideoInProgress = false;

    auto it = m_rewardedAdListeners.find(m_rewardedAdConfig.adType);
    if (it != m_rewardedAdListeners.end())
        (*it).second->onRewardedAdClosed(m_rewardedAdConfig, m_isEarnedReward);
    m_isEarnedReward = false;
}

void AdManagerImpl::onRewardedVideoFailedToShow(firebase::gma::AdError const& adError)
{
    m_isRewardedVideoInProgress = false;

    this->handleRewardedVideoError(adError.code(), adError.message());
}

void AdManagerImpl::loadInterstitialAd()
{
    if(!m_interstitialAd)
        return;

    if(!m_interstitialAdStateListener)
        m_interstitialAdStateListener = new InterstitialAdStateListener(this);
    m_interstitialAd->SetFullScreenContentListener(m_interstitialAdStateListener);

    firebase::gma::AdRequest request = this->createAdRequest();
    m_interstitialAd->LoadAd(INTERSTITIAL_AD_UNIT, request);
    m_interstitialAd->LoadAdLastResult().OnCompletion(handleResultInCocosThread<firebase::gma::AdResult>([this](int32 error, const char* message) {
        if (error == firebase::gma::kAdErrorCodeNone)
        {
            m_isInterstitialAdLoaded = true;
            for(auto listener: m_interstitialAdListeners)
                listener->onInterstitialAdLoaded();
        }
        else
        {
            CCLOG("AdManagerImpl: Failed to load interstitial ad. error: %d msg: %s", error, message);
            m_isInterstitialAdLoaded = false;
            this->handleInterstitialAdError(error, message);
        }
    }));
}

void AdManagerImpl::handleInterstitialAdError(int32 error, std::string const& message)
{
    if(error == firebase::gma::kAdErrorCodeAlreadyInitialized)
    {
        CCLOG("AdManagerImpl: The ad is already initialized (repeat call).");
        return;
    }

    if(error == firebase::gma::kAdErrorCodeLoadInProgress)
    {
        CCLOG("AdManagerImpl: A call has failed because an ad is currently loading.");
        return;
    }

    if(m_interstitialAdListeners.empty())
        return;

    AdError adError;
    adError.errorCode = getAdErrorCode(error);
    adError.errorMsg = message;
    for(auto listener: m_interstitialAdListeners)
        listener->onInterstitialAdRequestFailed(adError);
}

void AdManagerImpl::onInterstitialAdDismissed()
{
    m_isInterstitialAdLoaded = false;

    for(auto listener: m_interstitialAdListeners)
        listener->onInterstitialAdClosed();
}

void AdManagerImpl::onInterstitialAdFailedToShow(firebase::gma::AdError const& adError)
{
    m_isInterstitialAdLoaded = false;

    this->handleInterstitialAdError(adError.code(), adError.message());
}

NS_END

