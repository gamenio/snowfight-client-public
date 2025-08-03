#ifndef __GAME_LOAD_SCENE_H__
#define __GAME_LOAD_SCENE_H__

#include "game/client/protocol/pb/TheaterInfo.pb.h"

#include "common/Common.h"
#include "game/firservice/AdManagerProtocol.h"
#include "game/WorldListeners.h"
#include "scene/BaseScene.h"
#include "scene/battle/BattleSceneCommon.h"
#include "battle/dangerzone/SafeZone.h"

USING_NS_CC;
using namespace cocos2d::ui;

NS_BEGIN

class GameLoadScene : public BaseScene, InterstitialAdListener, 
										WorldLifecycleListener,
										WorldInitListener,
										LogonListener, 
										WorldAuthListener, 
										MyCharacterListener, 
										NetworkStatusListener
{
	enum LoadingStep
	{
		LOADING_STEP_NONE,
		LOADING_STEP_LOAD_BASIC,
		LOADING_STEP_LOAD_GAMBLE,
		LOADING_STEP_LOAD_MAPDATA,
		LOADING_STEP_LOAD_MAPTILESETS,
		LOADING_STEP_ENTER
	};

	enum LoadingPercent
	{
		PERCENT_AUTH_SESSION_OPENED			= 10,
		PERCENT_LOGON_SUCCEEDED				= 15,
		PERCENT_WORLD_SESSION_OPENED		= 25,
		PERCENT_WORLD_AUTH_OK				= 30,
		PERCENT_BASIC_LOADED				= 40,
		PERCENT_SPRITEATLAS_LOADED			= 60,
		PERCENT_GOT_THEATERINFO				= 65,
		PERCENT_MAPDATA_LOADED				= 80,
		PERCENT_MAPTILESETS_LOADED			= 100,
	};

public:
	virtual bool init(PlayerProfile const& playerProfile, bool isEnableInterstitialAd);
	static GameLoadScene* create(PlayerProfile const& playerProfile, bool isEnableInterstitialAd);

	Scene* addToScene();

	void onEnterTransitionDidFinish() override;

private:
	GameLoadScene();
	~GameLoadScene();

	void update(float delta) override;
	void updateBugPosition();

	void setNextStep(LoadingStep step);
	void performLoadingStep(LoadingStep step);

	void initWorld();
	void startGame();

	// LogonListener
	void onAuthSessionOpened(AuthSession* session) override;
	void onLogonSucceeded() override;
	void onLogonFailed(LogonResult::ErrorCode errorCode) override;
	void onFetchRealmListSucceeded(Realm const& preferredRealm) override;
	void onFetchRealmlistFailed() override;

	// WorldAuthListener
	void onWorldSessionOpened(WorldSession* session) override;
	void onWorldSessionTimedout() override;
	void onWorldAuthSucceeded() override;
	void onWorldWaitQueue(int32 waitPos) override;
	void onWorldAuthFailed() override;

	// MyCharacterListener
	void onTheaterInfo(TheaterInfo const& info) override;
	void onWaitForPlayers(WaitForPlayers const& waitForPlayers) override;
	void onInitSelfCompleted(DataPlayer* myChar) override;

	// WorldLifecycleListener
	void onWorldStarted(World* world) override;
	void onWorldStopped(World* world) override;

	// NetworkStatusListener
	void onNetworkError(NetworkError const& error) override;

	void startBattleScene();
	void startFuncScene();

	void startCountdownForWaitingPlayers(int32 remainingTime);
	void stopCountdownForWaitingPlayers();

	// InterstitialAdListener
	void onInterstitialAdClosed() override;
	void onInterstitialAdRequestFailed(AdError const& error) override;

	PlayerProfile m_playerProfile;
	bool m_isEnableInterstitialAd;
	TheaterInfo m_theaterInfo;
	std::string m_realmName;
	SafeZone* m_safeZone;
	bool m_exitAppAfterWorldStopped;
	bool m_isAuthenticated;
	bool m_isShowingInterstitialAd;

	Label* m_promptLabel;
	Sprite* m_progressBg;
	ProgressTimer* m_progress;
	Sprite* m_bugSp;
	NSTime m_trainingPromptStartTime;

	std::vector<LoadingStep> m_stepQueue;
	
};

NS_END

#endif // __GAME_LOAD_SCENE_H__
