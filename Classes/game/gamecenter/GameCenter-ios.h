#ifndef __GAME_CENTER_IOS_H__
#define __GAME_CENTER_IOS_H__

#include "common/Common.h"
#include "GameCenterProtocol.h"

USING_NS_CC;

NS_BEGIN


class GameCenterImpl: public GameCenterProtocol
{
public:
    GameCenterImpl();
    virtual ~GameCenterImpl();
    
    virtual void authLocalPlayer() override;
    virtual LocalPlayer* getLocalPlayer() const override { return m_localPlayer; }
    virtual void setShowAuthDialog(bool isShowAuthDialog) override { m_isShowAuthDialog = isShowAuthDialog;  }
    
    virtual void showLeaderboard(std::string const& leaderboardID) override;
    virtual bool reportScores(std::string const& leaderboardID) override;

    virtual void addListener(GameCenterListener* listener) override;
    virtual void removeListener(GameCenterListener* listener) override;

    virtual void setEnteringForeground(bool isEnteringForeground) override { m_isEnteringForeground = isEnteringForeground; }
    
    void handleGameCenterViewControllerDidFinish(void* gameCenterViewController);
private:
    std::vector<GameCenterListener*> m_listeners;
    LocalPlayer* m_localPlayer;
    bool m_isEnteringForeground;
    bool m_isShowAuthDialog;
    void* m_gkDelegateForwarder;
};


NS_END



#endif // __GAME_CENTER_IOS_H__
