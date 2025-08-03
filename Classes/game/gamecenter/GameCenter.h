//
//  GameCenter.h
//  snowfight
//
//  Created by Luthier on 2019/5/2.
//

#ifndef __GAME_CENTER_H__
#define __GAME_CENTER_H__

#include "common/Common.h"
#include "GameCenterProtocol.h"
#include "LocalPlayer.h"

#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS
#include "GameCenter-ios.h"
#elif CC_TARGET_PLATFORM == CC_PLATFORM_MAC
#include "GameCenter-mac.h"
#elif CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
#include "GameCenter-android.h"
#elif CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
#include "GameCenter-win32.h"
#else
#error "GameCenter: Target platform not supported."
#endif



USING_NS_CC;

NS_BEGIN

#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS
#define LEADERBOARD_ID_TOTAL_KILLS         "total_kills"
#elif CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
#define LEADERBOARD_ID_TOTAL_KILLS         "CgkIqIPK6ZYdEAIQAw"
#else
#define LEADERBOARD_ID_TOTAL_KILLS          ""
#endif

class GameCenter: public GameCenterProtocol
{
public:
    GameCenter();
    virtual ~GameCenter();
    
    static GameCenter* instance();
    
    virtual void authLocalPlayer() override;
    virtual LocalPlayer* getLocalPlayer() const override;
    virtual void setShowAuthDialog(bool isShowAuthDialog) override;
    
    virtual void showLeaderboard(std::string const& leaderboardID) override;
    virtual bool reportScores(std::string const& leaderboardID) override;

    virtual void addListener(GameCenterListener* listener) override;
    virtual void removeListener(GameCenterListener* listener) override;
    
    virtual void setEnteringForeground(bool isEnteringForeground) override;

#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID

    virtual bool startAuthorizationUI() override;
    virtual void signOut() override;

#endif // CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
    
private:
    GameCenterProtocol* m_gameCenterImpl;
    
};

NS_END

#define sGameCenter GameCenter::instance()

#endif // __GAME_CENTER_H__
