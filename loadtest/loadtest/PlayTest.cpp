#include "PlayTest.h"

#include "game/client/protocol/pb/TheaterInfo.pb.h"
#include "game/client/protocol/pb/BattleUpdate.pb.h"
#include "game/client/protocol/pb/BattleResult.pb.h"

#include "common/utils/TimeUtil.h"
#include "game/entities/DataPlayer.h"
#include "game/ClientConfig.h"
#include "game/LocaleMgr.h"
#include "Common.h"
#include "common/Machine.h"
#include "TestService.h"
#include "CombatTest.h"
#include "MovementTest.h"
#include "game/client/AuthSession.h"
#include "game/ObjectMgr.h"
#include "game/ShopMgr.h"

#define PLAYER_ID "2A67E871-428F-4326-92D9-5A42C76A50A4"

static cocos2d::Map<uint32, MapData*> s_mapDataMap;
static bool s_isTemplateLoaded = false;

MapData* createOrGetMapData(uint32 mapId)
{
	MapData* data = nullptr;
	auto it = s_mapDataMap.find(mapId);
	if (it == s_mapDataMap.end())
	{
		data = new MapData();
		s_mapDataMap.insert(mapId, data);
		CC_SAFE_RELEASE(data);
		bool ret = data->loadData(mapId);
		if (!ret)
			TESTLOG("Map(ID: %d) data load failed.", mapId);

		TEST_ASSERT_LOG(ret, "Map data load failed.");
	}
	else
		data = (*it).second;

	return data;
}

PlayTest::PlayTest(float duration) :
	m_isLogingOut(false),
	m_world(nullptr)
{
	m_logoutTimer.setDuration(duration);
}

PlayTest::~PlayTest()
{
	m_world = nullptr;
}

bool PlayTest::runTest()
{
	FileUtils::getInstance()->setPopupNotify(false);

	if (!s_isTemplateLoaded)
	{
		ObjectMgr::instance()->load();
		ShopMgr::instance()->load();
		s_isTemplateLoaded = true;
	}
		
	m_world = std::make_shared<World>();
	m_world->setLogonListener(this);
	m_world->setWorldAuthListener(this);
	m_world->setNetworkStatusListener(this);
	m_world->setWorldLifecycleListener(this);
	m_world->setWorldInitListener(this);
	m_world->addMyCharacterListener(this);
	m_world->setBattleUpdateListener(this);
	WorldConfig config;
	config.serverAddr = AUTH_SERVER_ADDR;
	config.serverPort = AUTH_SERVER_PORT;
	m_world->configure(config)->start();

	return true;
}

void PlayTest::update(float dt)
{
	TestBase::update(dt);

	bool worldUpdate = true;

	if (m_logoutTimer.getDuration() > 0)
	{
		m_logoutTimer.update(dt);
		// If the test completes then the logout operation is performed
		if (!m_isLogingOut && m_logoutTimer.passed())
		{
			this->finishSubTests();
			m_isLogingOut = true;
		}
	}

	for (auto it = m_subTests.begin(); it != m_subTests.end(); )
	{
		auto subTest = *it;
		if (!subTest->isFinished())
		{
			subTest->update(dt);
			worldUpdate &= !subTest->isAsyncRunning();
			++it;
		}
		else
		{
			it = m_subTests.erase(it);
		}

		// Stop the world when all test subitems and their asynchronous tasks are complete
		if (m_subTests.empty())
			m_world->stopDelayed();
	}

	// The reason the world needs to perform updates when the test subitem does not have an asynchronous 
	// task is that the test subitem will manipulate the world object in asynchronous processing.
	if (worldUpdate)
	{
		m_world->update(dt);
	}

	if (m_joinTheaterTimer.getDuration() > 0)
	{
		m_joinTheaterTimer.update(dt);
		if (m_joinTheaterTimer.passed())
		{
			m_world->sendJoinTheater();
			m_joinTheaterTimer.setDuration(0);
		}
	}
}


void PlayTest::finish()
{
	if (m_isFinished)
		return;

	if (!m_subTests.empty())
		this->finishSubTests();
	else if(m_world)
		m_world->stopDelayed();

	TestBase::finish();
}

void PlayTest::onAuthSessionOpened(AuthSession* session)
{
	//outlog("PlayTest::onAuthSessionOpened()");
	auto config = this->createLogonConfig();
	m_world->sendLogon(config);
}

void PlayTest::onLogonSucceeded()
{
	//outlog("PlayTest::onLogonSucceeded()");
	m_world->sendGetRealmList();
}

void PlayTest::onLogonFailed(LogonResult::ErrorCode errorCode)
{
	outlog("Logon failed.");
	m_world->stopDelayed();
}

void PlayTest::onFetchRealmListSucceeded(Realm const& preferredRealm)
{
}

void PlayTest::onFetchRealmlistFailed()
{
	outlog("Fetch realmlist failed.");
	m_world->stopDelayed();
}

void PlayTest::onWorldSessionOpened(WorldSession* session)
{
	//outlog("PlayTest::onWorldSessionOpened()");
	uint32 capabilities = 0;
	session->setTimeoutEnabled(true);
	capabilities |= WorldSession::REQUIRES_ALLOW_PLAYER_TO_RESTORE;

	bool ret = m_world->sendAuthProof(/* "PLAYER" */ "GM", PLAYER_ID, "", capabilities);
	TEST_ASSERT(ret);
}

void PlayTest::onWorldSessionTimedout()
{
	outlog("WorldSession has timed out.");
	if (!m_subTests.empty())
		this->finishSubTests();
	else
		m_world->stopDelayed();
}

void PlayTest::onWorldAuthSucceeded()
{
	auto config = this->createPlayerConfig();
	m_world->sendPlayerLogin(config);
	//outlog("PlayTest::onWorldAuthSucceeded()");
}

void PlayTest::onWorldWaitQueue(int32 waitPos)
{
	outlog("Waiting to join realm. Position in queue: %d", waitPos);
}

void PlayTest::onWorldAuthFailed()
{
	outlog("World auth failed.");
	m_world->stopDelayed();
}

void PlayTest::onTheaterInfo(TheaterInfo const& info)
{
	//outlog("PlayTest::onTheaterInfo()");
	MapData* data = createOrGetMapData(info.map_id());
	m_world->createMapIfNotExist(data, info.combat_grade());

	float dur = random(1.f, 10.f);
	m_joinTheaterTimer.setDuration(dur);
	outlog("Join the theater (id=%d, map_id=%d) in %f seconds.", info.theater_id(), info.map_id(), dur);
}

void PlayTest::onWaitForPlayers(WaitForPlayers const& waitForPlayers)
{
	outlog("Waiting for other players.");
}

void PlayTest::onInitSelfCompleted(DataPlayer* myChar)
{
	TEST_ASSERT(myChar != nullptr);
	outlog("Enter world.");
}

void PlayTest::onBattleResult(BattleResult const& result)
{
	outlog("End of battle.");
	this->finishSubTests();
}

void PlayTest::onWorldStarted(World* world)
{
}

void PlayTest::onWorldStopped(World* world)
{
	//outlog("PlayTest::onWorldStopped()");
	TestBase::finish();
}

void PlayTest::onNetworkError(NetworkError const& error)
{
	outlog("Network error! opcode:%d errorcode:%d message: %s", error.getOpcode(), error.getErrorCode(), error.getMessage().c_str());
	if (!m_subTests.empty())
		this->finishSubTests();
	else
		m_world->stopDelayed();
}

void PlayTest::onBattleUpdate(BattleUpdate const& update)
{
	if ((update.update_flags() & BATTLE_UPDATEFLAG_STATE) != 0)
	{
		switch (update.state())
		{
		case BATTLE_STATE_PREPARING:
			outlog("Preparing for battle.");
			break;
		case BATTLE_STATE_IN_PROGRESS:
			outlog("Start battle.");
			this->runSubTests();
			break;
		case BATTLE_STATE_ENDING:
			break;
		default:
			break;
		}
	}
}

void PlayTest::runSubTests()
{
	m_subTests.emplace_back(new MovementTest(m_world));
	m_subTests.emplace_back(new CombatTest(m_world));

	for (auto it = m_subTests.begin(); it != m_subTests.end(); ++it)
	{
		(*it)->runTest();
	}
}

void PlayTest::finishSubTests()
{
	for (auto it = m_subTests.begin(); it != m_subTests.end(); ++it)
	{
		(*it)->finish();
	}
}

AuthSession::LogonConfig PlayTest::createLogonConfig()
{
	AuthSession::LogonConfig config;
	config.version = APP_VERSION;
	config.build = BUILD_NUMBER;
	config.deviceModel = Machine::instance()->getModel();
	config.os = Machine::instance()->getOS();
	// config.country = sLocaleMgr->getCountryCode();
	config.country = "US";
	config.requiredCapabilities = AuthSession::REQUIRES_DISABLE_REGION_MAPPING_WITH_GEOIP;
	config.platform = CC_TARGET_PLATFORM;
	config.lang = sLocaleMgr->getLangTag();
	config.timezone = sLocaleMgr->getTimeZone();
	config.playerId = PLAYER_ID;
	config.networkType = ConnectivityHelper::getNetworkType();

	return config;
}

WorldSession::PlayerConfig PlayTest::createPlayerConfig()
{
	WorldSession::PlayerConfig config;
	config.charName = cocos2d::StringUtils::format("test%d", this->getId());
	config.country = "CN";
	config.lang = "zh-CN";
	config.winSize = WIN_SIZE;
	config.controllerType = snowfight::CONTROLLER_TYPE_DUAL_STICKS;
	config.isMoveEnabled = true;
	config.level = 1; // 0 for the new player
	config.isTrainee = false;

	//config.charId = HeroID::HERO_BOY;
	//config.statStageList = {};

	// Randomized character and statistic stages
	std::vector<HeroTemplate> const* heroTmplList = ShopMgr::instance()->getHeroTemplateList();
	int32 index = random(0, (int32)(heroTmplList->size() - 1));
	HeroTemplate const& heroTmpl = (*heroTmplList)[index];
	config.charId = heroTmpl.id;
	PlayerTemplate const* playerTmpl = ObjectMgr::instance()->getPlayerTemplate(heroTmpl.id);
	StatStageList statStageList = {};
	for (uint8 statType = 0; statType < MAX_STAT_TYPES; statType++)
	{
		int16 maxStage = 0;
		for (int16 stage = 1; stage <= playerTmpl->getMaxStage(); ++stage)
		{
			int32 price = heroTmpl.getStatUpgradePrice(stage, (StatType)statType);
			if (price > 0)
				maxStage++;
			else
				break;
		}
		statStageList[statType] = random((int16)0, maxStage);
	}
	statStageList[STAT_HEALTH_REGEN_RATE] = statStageList[STAT_MAX_HEALTH];
	statStageList[STAT_STAMINA_REGEN_RATE] = statStageList[STAT_MAX_STAMINA];
	config.statStageList = statStageList;

	return config;
}
