#include "GameCenter-ios.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)

#import <GameKit/GameKit.h>

#include "game/firservice/FirebaseService.h"
#include "LocalPlayer.h"

USING_NS;

@interface GKDelegateForwarder: NSObject <GKGameCenterControllerDelegate> {
    GameCenterImpl *m_handlerInstance;
}

- (id) initWithHandlerInstance:(GameCenterImpl*)handle;

- (void)gameCenterViewControllerDidFinish:(GKGameCenterViewController *)gameCenterViewController;

@end

@implementation GKDelegateForwarder

- (id) initWithHandlerInstance:(GameCenterImpl*)handle
{
    self = [super init];
    if (self) {
        m_handlerInstance = handle;
    }
    return self;
}

- (void)gameCenterViewControllerDidFinish:(GKGameCenterViewController *)gameCenterViewController
{
    if (m_handlerInstance)
        m_handlerInstance->handleGameCenterViewControllerDidFinish(gameCenterViewController);
}

- (void)dealloc
{
    m_handlerInstance = nil;
    [super dealloc];
}

@end


GameCenterError gameCenterErrorWithNSError(NSError *error)
{
    GameCenterError gcError;
    
    if([error localizedDescription])
        gcError.description = [error localizedDescription].UTF8String;
    
    if([error.domain isEqualToString:GKErrorDomain]) {
        switch(error.code) {
            case GKErrorCancelled:
                gcError.code = GAMECENTER_ERROR_CANCELED;
                break;
            case GKErrorUserDenied:
                gcError.code = GAMECENTER_ERROR_USER_DENIED;
                break;
            case GKErrorInvalidCredentials:
                gcError.code = GAMECENTER_ERROR_INVALID_CREDENTIALS;
                break;
            case GKErrorNotAuthenticated:
                gcError.code = GAMECENTER_ERROR_NOT_AUTHENTICATED;
                break;
            case GKErrorAuthenticationInProgress:
                gcError.code = GAMECENTER_ERROR_AUTHENTICATION_IN_PROGRESS;
                break;
            case GKErrorCommunicationsFailure:
                gcError.code = GAMECENTER_ERROR_COMMUNICATIONS_FAILURE;
                break;
            default:
                gcError.code = GAMECENTER_ERROR_UNKNOWN;
                break;
        }
    } else {
        gcError.code = GAMECENTER_ERROR_UNKNOWN;
    }
    
    return gcError;
}

NS_BEGIN

GameCenterImpl::GameCenterImpl() :
    m_localPlayer(new LocalPlayer()),
    m_isEnteringForeground(false),
    m_isShowAuthDialog(true),
    m_gkDelegateForwarder(nullptr)
{
    m_gkDelegateForwarder = [[GKDelegateForwarder alloc] initWithHandlerInstance:this];
}

GameCenterImpl::~GameCenterImpl()
{
    CC_SAFE_DELETE(m_localPlayer);
    if(m_gkDelegateForwarder)
    {
        [(id)m_gkDelegateForwarder release];
        m_gkDelegateForwarder = nullptr;
    }
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


void GameCenterImpl::authLocalPlayer() {
    GKLocalPlayer *localPlayer = [GKLocalPlayer localPlayer];
    localPlayer.authenticateHandler = ^(UIViewController *viewController, NSError *error) {
        NSMutableString *authLog = [NSMutableString stringWithFormat:@"authenticated: %d displayName: %@ playerID: %@ underage: %d",
                                localPlayer.authenticated,
                                localPlayer.displayName,
                                localPlayer.playerID,
                                localPlayer.underage];
        if (@available(iOS 13.0, *)) {
            [authLog appendFormat:@" scopedIDsArePersistent: %d", localPlayer.scopedIDsArePersistent];
        }
        if (@available(iOS 12.4, *)) {
            [authLog appendFormat:@" gamePlayerID: %@", localPlayer.gamePlayerID];
            [authLog appendFormat:@" teamPlayerID: %@", localPlayer.teamPlayerID];
        }
        NSLog(@"%@", authLog);
        
        if (viewController != nil) {
            if(m_isShowAuthDialog) {
                UIViewController *rootViewController = [UIApplication sharedApplication].keyWindow.rootViewController;
                [rootViewController presentViewController:viewController animated:YES completion:nil];
            }
        }
        else if (localPlayer.isAuthenticated) {
            NSString* gamePlayerID = @"";
            NSString* teamPlayerID = @"";
            BOOL scopedIDsArePersistent;
            
            if (@available(iOS 13.0, *)) {
				// In iOS 13 and later versions, when scopedIDsArePersistent=NO, using gamePlayerID and teamPlayerID will result in an "Unavailable Player Identification." 
				// The current handling method is to ignore the verification result in this case.
                scopedIDsArePersistent = localPlayer.scopedIDsArePersistent;
            }
            else {
                scopedIDsArePersistent = YES;
            }
            if (@available(iOS 12.4, *)) {
                if(scopedIDsArePersistent) {
                    if(localPlayer.gamePlayerID)
                        gamePlayerID = localPlayer.gamePlayerID;
                    if(localPlayer.teamPlayerID)
                        teamPlayerID = localPlayer.teamPlayerID;
                }
            }
            
            if(scopedIDsArePersistent)  {
                NSString* oldPlayerID = [NSString stringWithUTF8String:m_localPlayer->getPlayerID().c_str()];
                NSString* playerID = localPlayer.playerID == nil ? @"" : localPlayer.playerID;
                NSString* displayName = localPlayer.alias == nil ? @"" : localPlayer.alias;
                
				// It is a different player logging in
                bool diffPlayer = false;
                if([teamPlayerID length] != 0)
                    diffPlayer = ![oldPlayerID isEqualToString: teamPlayerID];
                else
                    diffPlayer = ![oldPlayerID isEqualToString: playerID];
                
                if(diffPlayer || !m_localPlayer->isAuthenticated()) {
                    for(GameCenterListener* listen: m_listeners)
                        listen->onAuthStarted(AuthOperation::SIGN_IN);
                    
                    // Player switch
                    if(diffPlayer && !m_localPlayer->isGuest() && [oldPlayerID length] != 0) {
                        AuthResponse resp;
                        resp.state = AUTH_STATE_FAIL;
                        resp.error.code = GAMECENTER_ERROR_ACCOUNT_CHANGED;
                        for(GameCenterListener* listen: m_listeners)
                            listen->onAuthFinished(AuthOperation::SIGN_IN, resp);
                    }
                    else {
                        m_localPlayer->setupDefaultValues();
                        if([teamPlayerID length] != 0) {
                            m_localPlayer->setPlayerID(teamPlayerID.UTF8String);
                            m_localPlayer->setOriginalPlayerID(playerID.UTF8String);
                        }
                        else
                           m_localPlayer->setPlayerID(playerID.UTF8String);
                        
                        m_localPlayer->setAuthenticated(true);
                        m_localPlayer->setDisplayName(displayName.UTF8String);
                        m_localPlayer->setUnderage(localPlayer.underage);
                        
                        sAnalytics->setSignUpMethod(kSignUpMethodApple);
                        m_localPlayer->loadDataAsync([this](bool ret) {
                            sAnalytics->setUserId(m_localPlayer->getPlayerID().c_str());
                            sAnalytics->logLogin();
                            if(!m_listeners.empty()) {
                                AuthResponse resp;
                                if(ret) {
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
            }
        }
        else {
			// Keep loaded local player data when not logged in to GameCenter
            bool diffPlayer = m_localPlayer->getPlayerID().empty();
            if(diffPlayer) {
                for(GameCenterListener* listen: m_listeners)
                    listen->onAuthStarted(AuthOperation::SIGN_IN);
                
                m_localPlayer->setupDefaultValues();
            }

            m_localPlayer->setAuthenticated(false);
            
            GameCenterError gcError;
            if(error)
                gcError = gameCenterErrorWithNSError(error);
            else
                gcError.code = GAMECENTER_ERROR_UNKNOWN;
            if(diffPlayer) {
                sAnalytics->setSignUpMethod(kSignUpMethodNone);
                m_localPlayer->loadDataAsync([this, gcError](bool ret) {
                    sAnalytics->setUserId(m_localPlayer->getPlayerID().c_str());
                    if(!m_listeners.empty()) {
                        AuthResponse resp;
                        resp.state = AUTH_STATE_FAIL;
                        if(ret)
                            resp.error = gcError;
                        else
                            resp.error.code = GAMECENTER_ERROR_LOAD_DATA_FAILED;
                        for(GameCenterListener* listen: m_listeners)
                            listen->onAuthFinished(AuthOperation::SIGN_IN, resp);
                    }
                });
            }
        }
    };
}

void GameCenterImpl::showLeaderboard(std::string const& leaderboardID)
{
    GKGameCenterViewController *gameCenterController = [[GKGameCenterViewController alloc] init];
    if (gameCenterController != nil)
    {
        UIViewController *rootViewController = [UIApplication sharedApplication].keyWindow.rootViewController;
        gameCenterController.gameCenterDelegate =(GKDelegateForwarder*)m_gkDelegateForwarder;
        gameCenterController.viewState = GKGameCenterViewControllerStateLeaderboards;
        gameCenterController.leaderboardTimeScope = GKLeaderboardTimeScopeToday;
        gameCenterController.leaderboardIdentifier = [NSString stringWithUTF8String:leaderboardID.c_str()];
        [rootViewController presentViewController: gameCenterController animated: YES completion:nil];
    }
    [gameCenterController release];
}

void GameCenterImpl::handleGameCenterViewControllerDidFinish(void* gameCenterViewController)
{
    GKGameCenterViewController *gameCenterController = (GKGameCenterViewController *)gameCenterViewController;
    [gameCenterController dismissViewControllerAnimated:YES completion:nil];

}

bool GameCenterImpl::reportScores(std::string const& leaderboardID)
{
    if(!m_localPlayer->isAuthenticated())
        return false;
    
    int32 scoreValue = m_localPlayer->getScoreValue(leaderboardID);
    if(scoreValue <= 0 || m_localPlayer->isScoreReported(leaderboardID))
        return false;
    
    NSString *identifier = [NSString stringWithUTF8String:leaderboardID.c_str()];
    GKScore *scoreReporter = [[GKScore alloc] initWithLeaderboardIdentifier: identifier];
    scoreReporter.value = scoreValue;
    scoreReporter.context = 0;

    NSArray *scores = @[scoreReporter];
    std::string _leaderboardID = leaderboardID;
    [GKScore reportScores:scores withCompletionHandler:^(NSError *error) {
        if(error == nil)
        {
            m_localPlayer->setScoreReported(_leaderboardID, true);
            m_localPlayer->saveDataAsync([this](bool ret){
                if(!m_listeners.empty()) {
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
        }
        else
        {
            for(GameCenterListener* listen: m_listeners)
                listen->onReportScoreFail(gameCenterErrorWithNSError(error));
        }
    }];
    
    return true;
}

NS_END

#endif // CC_TARGET_PLATFORM == CC_PLATFORM_IOS
