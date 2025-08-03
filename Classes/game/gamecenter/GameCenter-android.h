#ifndef __GAME_CENTER_ANDROID_H__
#define __GAME_CENTER_ANDROID_H__

#include "gpg/gpg.h"

#include "common/Common.h"
#include "GameCenterProtocol.h"

USING_NS_CC;

NS_BEGIN


class GameCenterImpl: public GameCenterProtocol
{
public:
	enum AuthProgress
	{
		AUTH_PROGRESS_NONE,
		AUTH_PROGRESS_STARTED,
		AUTH_PROGRESS_CANCELED,
		AUTH_PROGRESS_FINISHED,
	};

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

    virtual bool startAuthorizationUI() override;
    virtual void signOut() override;

	void setAuthProgress(AuthProgress progress) { m_authProgress = progress; }
	bool isAuthorizing() const { return  m_authProgress == AUTH_PROGRESS_STARTED; }

private:
	void initServices();
    void onAuthActionStarted(gpg::AuthOperation op);
    void onAuthActionFinished(gpg::AuthOperation op, gpg::AuthStatus status);
    void onAuthFailed(AuthOperation op, GameCenterError error);
    void onAuthSuccess(AuthOperation op, gpg::PlayerManager::FetchSelfResponse const& selfResp);

    LocalPlayer* m_localPlayer;
	std::vector<GameCenterListener*> m_listeners;
	std::unique_ptr<gpg::GameServices> m_gameServices;
	AuthProgress m_authProgress;

};


NS_END 

#endif // __GAME_CENTER_ANDROID_H__