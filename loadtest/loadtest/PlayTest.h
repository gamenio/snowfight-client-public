#ifndef __PLAY_TEST_H__
#define __PLAY_TEST_H__

#include "common/Common.h"

#include "common/utils/NotificationService.h"
#include "common/network/NetworkError.h"
#include "game/World.h"
#include "game/MessageEnum.h"
#include "TestBase.h"


USING_NS;

class PlayTest : public TestBase, WorldLifecycleListener,
								  WorldInitListener,
								  LogonListener, 
								  WorldAuthListener, 
								  MyCharacterListener, 
								  NetworkStatusListener,
								  BattleUpdateListener
{
public:
	PlayTest(float duration = 0.f);
	~PlayTest();

	bool runTest() override;
	void update(float dt) override;
	void finish() override;

	std::string getTitle() const override { return "Play Test"; }

private:
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
	void onBattleResult(BattleResult const& result) override;

	// WorldLifecycleListener
	void onWorldStarted(World* world) override;
	void onWorldStopped(World* world) override;

	// NetworkStatusListener
	void onNetworkError(NetworkError const& error) override;

	// BattleUpdateListener
	void onBattleUpdate(BattleUpdate const& update) override;

	void runSubTests();
	void finishSubTests();

	AuthSession::LogonConfig createLogonConfig();
	WorldSession::PlayerConfig createPlayerConfig();

	std::vector<std::shared_ptr<TestBase>> m_subTests;
	DelayTimer m_logoutTimer;
	bool m_isLogingOut;

	std::shared_ptr<World> m_world;
	DelayTimer m_joinTheaterTimer;
};

#endif // __PLAY_TEST_H__
