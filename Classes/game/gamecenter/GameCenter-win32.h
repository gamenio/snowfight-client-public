#ifndef __GAME_CENTER_WIN32_H__
#define __GAME_CENTER_WIN32_H__

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
    virtual LocalPlayer* getLocalPlayer() const override;
    virtual void setShowAuthDialog(bool isShowAuthDialog) override;
    
    virtual void showLeaderboard(std::string const& leaderboardID) override;
	virtual bool reportScores(std::string const& leaderboardID) override;

	virtual void addListener(GameCenterListener* listener) override;
	virtual void removeListener(GameCenterListener* listener) override;
    
    virtual void setEnteringForeground(bool isEnteringForeground) override;
private:
    LocalPlayer* m_localPlayer;
	std::vector<GameCenterListener*> m_listeners;
};


NS_END



#endif // __GAME_CENTER_WIN32_H__
