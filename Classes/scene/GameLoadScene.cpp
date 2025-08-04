#include "GameLoadScene.h"

#include "common/Machine.h"
#include "game/firservice/FirebaseService.h"
#include "game/client/WorldSession.h"
#include "game/World.h"
#include "scene/AssetsLoader.h"
#include "scene/gui/ModalDialog.h"
#include "battle/BattleScene.h"
#include "func/FuncScene.h"
#include "SoundMgr.h"
#include "Utils.h"


NS_BEGIN

// Scene atlas
#define LOAD_PLIST								RES_IMAGE("load.plist")
#define LOAD_ATLAS								RES_IMAGE("load.pvr.ccz")
#define IMG_BG									"load_bg.pvr.ccz"
#define IMG_BG_NARROW							"load_bg_narrow.pvr.ccz"

// Scene element frame name
#define FRAMENAME_PROGRESS_BG					"load_progress_bg.png"
#define FRAMENAME_PROGRESS_FG					"load_progress_fg.png"

// Bug walking animation configuration
#define BUG_WALKING_FRAMES						6
#define BUG_WALKING_DEFAULT_FRAME_INDEX			5
#define BUG_WALKING_FRAMENAME_FORMAT			"load_bug_walking%.2d.png"

#define DIALOG_NAME_WAIT_QUEUE					"WAIT_QUEUE_DIALOG"
#define SCHEDULE_KEY_WAIT_FOR_PLAYERS			"WaitForPlayers"
#define TRANSITION_SCENE_DURATION				0.5f

#define SCHEDULE_KEY_JOIN_THEATER_DELAYED		"JoinTheaterDelayed"
#define TRAINING_PROMPT_MIN_DURATION			5.0f

GameLoadScene* GameLoadScene::create(PlayerProfile const& playerProfile, bool isEnableInterstitialAd)
{
	GameLoadScene *pRet = new(std::nothrow) GameLoadScene();
	if (pRet && pRet->init(playerProfile, isEnableInterstitialAd))
	{
		pRet->autorelease();
		return pRet;
	}
	else
	{
		CC_SAFE_DELETE(pRet);
		return nullptr;
	}
}

bool GameLoadScene::init(PlayerProfile const& playerProfile, bool isEnableInterstitialAd)
{
	if (!BaseScene::init())
	{
		return false;
	}

    sAnalytics->setCurrentScreen("GameLoadScene");

	m_playerProfile = playerProfile;
	m_isEnableInterstitialAd = isEnableInterstitialAd;

	sAdManager->addInterstitialAdListener(this);

	SpriteFrameCache::getInstance()->addSpriteFramesWithFile(LOAD_PLIST);

	auto visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();

	// Scene background
	Sprite* bgSp = Sprite::create();
	bgSp->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y + visibleSize.height / 2));
	if (Machine::instance()->isNarrowScreen())
		bgSp->setTexture(this->autoUncacheImage(IMG_BG_NARROW, Texture2D::PixelFormat::RGBA4444));
	else
		bgSp->setTexture(this->autoUncacheImage(IMG_BG, Texture2D::PixelFormat::RGBA4444));
	this->addChild(bgSp);

	// Progress bar background
	m_progressBg = Sprite::createWithSpriteFrameName(FRAMENAME_PROGRESS_BG);
	m_progressBg->setAnchorPoint(Point::ANCHOR_MIDDLE_BOTTOM);
	m_progressBg->setPosition(Vec2(origin.x + visibleSize.width / 2, 51));
	this->addChild(m_progressBg);

	// Progress bar
	Sprite* progSprite = Sprite::createWithSpriteFrameName(FRAMENAME_PROGRESS_FG);
	m_progress = ProgressTimer::create(progSprite);
	m_progress->setAnchorPoint(Point::ANCHOR_MIDDLE_BOTTOM);
	m_progress->setType(ProgressTimer::Type::BAR);
	m_progress->setBarChangeRate(Vec2(1, 0));
	m_progress->setMidpoint(Vec2(0, 0));
	m_progress->setPosition(m_progressBg->getPosition());
	m_progress->setPercentage(0);
	this->addChild(m_progress);

	// Prompt label
	m_promptLabel = Label::createWithSystemFont("PromptLabel", DEFAULT_SYSTEM_FONT, 11);
	m_promptLabel->setAnchorPoint(Point::ANCHOR_MIDDLE_TOP);
	m_promptLabel->setAlignment(TextHAlignment::CENTER);
	m_promptLabel->setTextColor(Color4B(229, 222, 205, 255));
	Utils::enableBoldForLabel(m_promptLabel);
	m_promptLabel->setPosition(Vec2(origin.x + visibleSize.width / 2, m_progress->getBoundingBox().getMinY() - 4));
	this->addChild(m_promptLabel);

	// Bug walking animation
	Vector<SpriteFrame*> frameSeq;
	for (int32 i = 0; i < BUG_WALKING_FRAMES; ++i)
	{
		std::string framename = StringUtils::format(BUG_WALKING_FRAMENAME_FORMAT, i);
		SpriteFrame* sf = SpriteFrameCache::getInstance()->getSpriteFrameByName(framename);
		NS_ASSERT(sf != nullptr);
		frameSeq.pushBack(sf);
	}
	Animation* animation = Animation::createWithSpriteFrames(frameSeq, ANIM_NORMAL_FRAME_DELAY);
	animation->setLoops(UINT_MAX);
	Animate* animate = Animate::create(animation);
	m_bugSp = Sprite::createWithSpriteFrame(frameSeq.at(BUG_WALKING_DEFAULT_FRAME_INDEX));
    m_bugSp->setAnchorPoint(Point::ANCHOR_BOTTOM_RIGHT);
	this->addChild(m_bugSp);
	m_bugSp->runAction(animate);
	this->updateBugPosition();

	// Play background music
	if(!sSoundMgr->isPlaying(SOUND_FUNC_MUSIC))
	{
		sSoundMgr->play(SOUND_FUNC_MUSIC, true, 0.0f);
		sSoundMgr->fadeInVolume(SOUND_FUNC_MUSIC, 1.0f);
	}

	// Initialize the game world
	this->initWorld();

	return true;
}

Scene* GameLoadScene::addToScene()
{
	auto scene = Scene::create();
	scene->addChild(this);
	return TransitionCrossFade::create(TRANSITION_SCENE_DURATION, scene);
}

void GameLoadScene::onEnterTransitionDidFinish()
{
	BaseScene::onEnterTransitionDidFinish();

	if(!sGameCenter->getLocalPlayer()->isAdsRemoved()
		&& m_isEnableInterstitialAd
		&& sAdManager->isInterstitialAdLoaded())
	{
		if(sAdManager->showInterstitialAd())
			m_isShowingInterstitialAd = true;
	}

	if(!m_isShowingInterstitialAd)
		this->startGame();
}

GameLoadScene::GameLoadScene() :
	m_isEnableInterstitialAd(false),
	m_safeZone(nullptr),
	m_exitAppAfterWorldStopped(false),
	m_isAuthenticated(false),
	m_isShowingInterstitialAd(false),
	m_promptLabel(nullptr),
	m_progress(nullptr),
	m_progressBg(nullptr),
	m_bugSp(nullptr),
	m_trainingPromptStartTime(0)
{

}

GameLoadScene::~GameLoadScene()
{
	this->unscheduleUpdate();
	this->stopCountdownForWaitingPlayers();
	sAdManager->removeInterstitialAdListener(this);

	SpriteFrameCache::getInstance()->removeSpriteFramesFromFile(LOAD_PLIST);
	Director::getInstance()->getTextureCache()->removeTextureForKey(LOAD_ATLAS);

	CC_SAFE_RELEASE_NULL(m_safeZone);

	m_promptLabel = nullptr;
	m_progress = nullptr;
	m_progressBg = nullptr;
	m_bugSp = nullptr;
}

void GameLoadScene::update(float delta)
{
	this->updateBugPosition();

	if (!m_stepQueue.empty())
	{
		LoadingStep step = m_stepQueue.front();
		m_stepQueue.erase(m_stepQueue.begin());

		this->performLoadingStep(step);

	}
}

void GameLoadScene::updateBugPosition()
{
    m_bugSp->setPosition(m_progress->getBoundingBox().getMinX() + 50 + (m_progress->getBoundingBox().size.width - 40) * (m_progress->getPercentage() / 100.0f),
		m_progress->getBoundingBox().getMaxY() - 3);
}

void GameLoadScene::setNextStep(LoadingStep step)
{
	m_stepQueue.push_back(step);

	if (!sGameCenter->getLocalPlayer()->isTrainee())
	{
		switch (step)
		{
		case LOADING_STEP_LOAD_BASIC:
		case LOADING_STEP_LOAD_GAMBLE:
			m_promptLabel->setString(sLocaleMgr->getString("login_status_load_assets"));
			break;
		case LOADING_STEP_LOAD_MAPDATA:
		case LOADING_STEP_LOAD_MAPTILESETS:
			m_promptLabel->setString(sLocaleMgr->getString("login_status_load_map"));
			break;
		case LOADING_STEP_ENTER:
			break;
		default:
			break;
		}
	}
}

void GameLoadScene::performLoadingStep(LoadingStep step)
{
	World* world = World::getInstance();
	switch (step)
	{
	case LOADING_STEP_LOAD_BASIC:
        sAssetsLoader->unloadFunctional();
        sAssetsLoader->loadWorldBasic([this]() {
            m_progress->setPercentage(PERCENT_BASIC_LOADED);
            this->setNextStep(LOADING_STEP_LOAD_GAMBLE);
        });
        break;
	case LOADING_STEP_LOAD_GAMBLE:
        sAssetsLoader->loadWorldGamble([this, world](){
            m_progress->setPercentage(PERCENT_SPRITEATLAS_LOADED);

			sSoundMgr->fadeOutVolume(SOUND_FUNC_MUSIC, 1.5f, [this](std::string const& resName){
				sSoundMgr->unload(SOUND_FUNC_MUSIC);
			});

			WorldSession::PlayerConfig config;
			config.charName = m_playerProfile.nickname;
			config.lang = m_playerProfile.lang;
			config.country = m_playerProfile.country;
			config.charId = m_playerProfile.charId;
			config.winSize = m_playerProfile.winSize;
			config.controllerType = m_playerProfile.controllerType;
			config.isMoveEnabled = false;

			LocalPlayer* localPlayer = sGameCenter->getLocalPlayer();
			localPlayer->getStatStageList(m_playerProfile.charId, config.statStageList);
			int32 totalKills = localPlayer->getScoreValue(LEADERBOARD_ID_TOTAL_KILLS);
			if (totalKills > 0)
				config.totalKills = totalKills;
			config.property = localPlayer->getProperty();
			config.level = localPlayer->getLevel();
			config.experience = localPlayer->getExperience();
			config.isTrainee = localPlayer->isTrainee();
			config.rewardStage = localPlayer->getRewardStage();
			config.dailyRewardDays = localPlayer->getDailyRewardDays();

			world->sendPlayerLogin(config);
        });
		break;
	case LOADING_STEP_LOAD_MAPDATA:
    {
        MapData* mapData = new MapData();
        mapData->loadDataAsync((uint16)m_theaterInfo.map_id(), [this, mapData, world](bool success){
            NS_ASSERT_LOG(success, "Map data load failed.");
            if(success)
            {
				world->createMapIfNotExist(mapData, (uint8)m_theaterInfo.combat_grade());
				if (m_theaterInfo.safezone_radius() > 0)
				{
					TileCoord center;
					center.x = m_theaterInfo.safezone_center_x();
					center.y = m_theaterInfo.safezone_center_y();
					CC_ASSERT(!m_safeZone);
					m_safeZone = new SafeZone(mapData, center, m_theaterInfo.safezone_radius(), m_theaterInfo.battle_duration());
					m_safeZone->createClouds();
				}
                m_progress->setPercentage(PERCENT_MAPDATA_LOADED);
                this->setNextStep(LOADING_STEP_LOAD_MAPTILESETS);
            }
            CC_SAFE_RELEASE(mapData);
        });
        break;
    }
	case LOADING_STEP_LOAD_MAPTILESETS:
		if(world->getMap())
		{
			sAssetsLoader->preloadMapTilesets(world->getMap()->getMapData(), [this, world]() {
                m_progress->setPercentage(PERCENT_MAPTILESETS_LOADED);

				auto callback = [this, world](float)
				{
					world->sendJoinTheater();
				};
				if (sGameCenter->getLocalPlayer()->isTrainee())
				{
					NSTime diff = time_util::getUptimeMillis() - m_trainingPromptStartTime;
					float delay = TRAINING_PROMPT_MIN_DURATION - diff / 1000.f;
					delay = std::max(0.f, delay);
					this->scheduleOnce(callback, delay, SCHEDULE_KEY_JOIN_THEATER_DELAYED);
				}
				else
				{
					m_promptLabel->setString(sLocaleMgr->getString("login_status_join_theater"));
					callback(0.f);
				}
			});
		}
		break;
	case LOADING_STEP_ENTER:
		this->startBattleScene();
		break;
    default:
        break;
	}
}

void GameLoadScene::initWorld()
{
	if (sGameCenter->getLocalPlayer()->isTrainee())
	{
		m_promptLabel->setString(sLocaleMgr->getString("login_status_training"));
		m_trainingPromptStartTime = time_util::getUptimeMillis();
	}
	else
		m_promptLabel->setString(sLocaleMgr->getString("login_status_connecting"));

	World* world = World::getInstance();
	world->setLogonListener(this);
	world->setWorldAuthListener(this);
	world->setNetworkStatusListener(this);
	world->setWorldLifecycleListener(this);
	world->setWorldInitListener(this);
	world->addMyCharacterListener(this);
}

void GameLoadScene::startGame()
{
	// Wait a moment, then start loading the game.
	this->scheduleOnce([this](float dt) {
		WorldConfig config;
		config.serverAddr = m_playerProfile.serverAddr;
		config.serverPort = m_playerProfile.serverPort;
		World::getInstance()->configure(config)->start();

		// Schedule an update to load the game
		this->scheduleUpdate();
	}, 0.5f, "LoadGame");
}

void GameLoadScene::onAuthSessionOpened(AuthSession* session)
{
	if (!sGameCenter->getLocalPlayer()->isTrainee())
		m_promptLabel->setString(sLocaleMgr->getString("login_status_logging_on"));

	LocalPlayer* localPlayer = sGameCenter->getLocalPlayer();
	AuthSession::LogonConfig config;
	config.version = APP_VERSION;
	config.build = BUILD_NUMBER;
	config.deviceModel = Machine::instance()->getModel();
	config.os = Machine::instance()->getOS();
#if NS_DEBUG
	if(m_playerProfile.isRealmByRegion)
		config.requiredCapabilities = AuthSession::REQUIRES_DISABLE_REGION_MAPPING_WITH_GEOIP;
#endif // NS_DEBUG
	config.country = m_playerProfile.country;
	config.platform = CC_TARGET_PLATFORM;
	config.lang = sLocaleMgr->getLangTag();
	config.timezone = sLocaleMgr->getTimeZone();
	config.playerId = localPlayer->getPlayerID();
	config.originalPlayerId = localPlayer->getOriginalPlayerID();
	config.networkType = ConnectivityHelper::getNetworkType();
	config.channelId = CHANNEL_ID;
	World::getInstance()->sendLogon(config);

	if (sTimeService->getTimeState() == TimeService::TIME_NONE)
		sTimeService->syncTime();

	m_progress->setPercentage(PERCENT_AUTH_SESSION_OPENED);
}

void GameLoadScene::onLogonSucceeded()
{
	m_isAuthenticated = true;
	if (!sGameCenter->getLocalPlayer()->isTrainee())
		m_promptLabel->setString(sLocaleMgr->getString("login_status_fetch_realmlist"));
	World::getInstance()->sendGetRealmList();
	m_progress->setPercentage(PERCENT_LOGON_SUCCEEDED);
}

void GameLoadScene::onLogonFailed(LogonResult::ErrorCode errorCode)
{
	if (errorCode == LogonResult::ERROR_LOW_CLIENT_VERSION)
	{
		MessageDialog* dialog = MessageDialog::create();
		dialog->setTitle(sLocaleMgr->getString("login_dlg_title_low_client_version"));
		dialog->setMessage(sLocaleMgr->getString("login_dlg_msg_low_client_version"));
		dialog->addPositiveButton(sLocaleMgr->getString("login_dlg_btn_upgrade"), [this](Ref* sender) {
			Application::getInstance()->openURL(STORE_APP_URL);
			m_exitAppAfterWorldStopped = true;
			World::getInstance()->stopDelayed();
		});
		dialog->show();
	}
	else
	{
		MessageDialog* dialog = MessageDialog::create();
		dialog->setTitle(sLocaleMgr->getString("login_dlg_title_logon_failed"));
		dialog->setMessage(sLocaleMgr->getString("login_dlg_msg_logon_failed"));
		dialog->addOkButton([this](Ref* sender) {
			World::getInstance()->stopDelayed();
		});
		dialog->show();
	}
}

void GameLoadScene::onFetchRealmListSucceeded(Realm const& preferredRealm)
{
	m_realmName = preferredRealm.name();
}

void GameLoadScene::onFetchRealmlistFailed()
{
	MessageDialog* dialog = MessageDialog::create();
	dialog->setTitle(sLocaleMgr->getString("login_dlg_title_realmlist_failed"));
	dialog->setMessage(sLocaleMgr->getString("login_dlg_msg_realmlist_failed"));
	dialog->addOkButton([this](Ref* sender) {
		World::getInstance()->stopDelayed();
	});
	dialog->show();
}

void GameLoadScene::onWorldSessionOpened(WorldSession* session)
{
	if (!sGameCenter->getLocalPlayer()->isTrainee())
		m_promptLabel->setString(sLocaleMgr->getString("login_status_logon_realm"));

	uint32 capabilities = 0;
	if (m_playerProfile.isSessionTimeoutDisabled)
		session->setTimeoutEnabled(false);
	else
	{
		session->setTimeoutEnabled(true);
		capabilities |= WorldSession::REQUIRES_ALLOW_PLAYER_TO_RESTORE;
	}

	std::string playerID = sGameCenter->getLocalPlayer()->getPlayerID();
	std::string originalPlayerID = sGameCenter->getLocalPlayer()->getOriginalPlayerID();
	World::getInstance()->sendAuthProof(m_playerProfile.authProof, playerID, originalPlayerID, capabilities);

	m_progress->setPercentage(PERCENT_WORLD_SESSION_OPENED);
}

void GameLoadScene::onWorldSessionTimedout()
{
	MessageDialog* dialog = MessageDialog::create();
	dialog->setTitle(sLocaleMgr->getString("login_dlg_title_session_timedout"));
	dialog->setMessage(sLocaleMgr->getString("login_dlg_msg_session_timedout"));
	dialog->addOkButton([this](Ref* sender) {
		World::getInstance()->stopDelayed();
	});
	dialog->show();
}

void GameLoadScene::onWorldAuthSucceeded()
{
	MessageDialog* dialog = dynamic_cast<MessageDialog*>(ModalDialog::getDialogByName(DIALOG_NAME_WAIT_QUEUE));
	if (dialog)
		dialog->dismiss();
	this->setNextStep(LOADING_STEP_LOAD_BASIC);
	m_progress->setPercentage(PERCENT_WORLD_AUTH_OK);
}

void GameLoadScene::onWorldWaitQueue(int32 waitPos)
{
	MessageDialog* dialog = MessageDialog::create();
	dialog->setTitle(sLocaleMgr->getString("login_dlg_title_wait_queue"));
	dialog->setName(DIALOG_NAME_WAIT_QUEUE);
	dialog->setMessage(StringUtils::format(sLocaleMgr->getString("login_dlg_msg_wait_queue").c_str(), waitPos));
	dialog->addCancelButton([this](Ref* sender) {
		World::getInstance()->stopDelayed();
	});
	dialog->show();
}

void GameLoadScene::onWorldAuthFailed()
{
}

void GameLoadScene::onTheaterInfo(TheaterInfo const& info)
{
	this->setNextStep(LOADING_STEP_LOAD_MAPDATA);
	m_theaterInfo = info;
	if (!sGameCenter->getLocalPlayer()->isTrainee()
		&& !sGameCenter->getLocalPlayer()->isAdsRemoved()
		&& m_theaterInfo.request_battle_end_ad())
	{
		sAdManager->requestInterstitialAd();
	}
	m_progress->setPercentage(PERCENT_GOT_THEATERINFO);
}

void GameLoadScene::onWaitForPlayers(WaitForPlayers const& waitForPlayers)
{
	int32 remainingTime = waitForPlayers.duration() - (time_util::getUptimeMillis() - waitForPlayers.start_time());
	if (remainingTime > 0)
		this->startCountdownForWaitingPlayers(remainingTime);
}

void GameLoadScene::onInitSelfCompleted(DataPlayer* myChar)
{
	World* world = World::getInstance();
	world->setLogonListener(nullptr);
	world->setWorldAuthListener(nullptr);
	world->setNetworkStatusListener(nullptr);
	world->removeMyCharacterListener(this);
	world->setWorldLifecycleListener(nullptr);
	world->setWorldInitListener(nullptr);

	this->performLoadingStep(LOADING_STEP_ENTER);
}

void GameLoadScene::onWorldStarted(World* world)
{
}

void GameLoadScene::onWorldStopped(World* world)
{
	m_stepQueue.clear();
	World::destoryInstance();

	if (m_exitAppAfterWorldStopped)
		Utils::exitApp();
	else
	{
		sSoundMgr->stop(SOUND_FUNC_MUSIC);
		sAssetsLoader->unloadWorld();
		sAssetsLoader->loadFunctional([this]() {
			this->startFuncScene();
		});
	}
}

void GameLoadScene::onNetworkError(NetworkError const& error)
{
	m_stepQueue.clear();
	this->stopCountdownForWaitingPlayers();

	MessageDialog* dialog = MessageDialog::create();
	dialog->setTitle(sLocaleMgr->getString("login_dlg_title_connect_failed"));
	if (error.getErrorCode() == NetworkError::CONNECTION_FAILED
		|| error.getErrorCode() == NetworkError::CONNECTION_TIMED_OUT
		|| error.getErrorCode() == NetworkError::CONNECTION_RESOLVE_FAILED)
	{
		if (!m_isAuthenticated)
			dialog->setMessage(sLocaleMgr->getString("login_dlg_msg_connect_server_failed"));
		else
			dialog->setMessage(sLocaleMgr->getString("login_dlg_msg_connect_realm_failed"));
	}
	else
		dialog->setMessage(StringUtils::format(sLocaleMgr->getString("login_dlg_msg_network_error").c_str(), (int32)error.getErrorCode()));
	dialog->addOkButton([this](Ref* sender) {
		World::getInstance()->stopDelayed();
	});
	dialog->show();
	CCLOG("Network error! opcode:%d errorcode:%d message: %s", error.getOpcode(), error.getErrorCode(), error.getMessage().c_str());
}

void GameLoadScene::startBattleScene()
{
	RegionalInfo regionalInfo;
	regionalInfo.theaterId = m_theaterInfo.theater_id();
	regionalInfo.mapId = m_theaterInfo.map_id();
	regionalInfo.combatGrade = (uint8)m_theaterInfo.combat_grade();
	regionalInfo.realmName = m_realmName;
	regionalInfo.isAppReviewModeEnabled = m_theaterInfo.enable_app_review_mode();
	auto scene = BattleScene::create(regionalInfo, m_playerProfile, m_safeZone)->addToScene();
	Director::getInstance()->replaceScene(scene);
}

void GameLoadScene::startFuncScene()
{
	auto scene = FuncScene::createScene();
	Director::getInstance()->replaceScene(scene);
}

void GameLoadScene::startCountdownForWaitingPlayers(int32 remainingTime)
{
	int32 remainingSec = (int32)std::ceil(remainingTime / 1000.0f);
	m_promptLabel->setString(StringUtils::format(sLocaleMgr->getString("login_status_wait_for_players").c_str(), remainingSec));

	int32 numTimes = remainingSec - 1;
	if (numTimes >= 0)
	{
		float delay = time_util::toGameTimeSeconds(remainingTime % 1000);
		//CCLOG("numTimes: %d delay: %f", numTimes, delay);
		NSTime startTime = time_util::getUptimeMillis();
		this->schedule([this, startTime, remainingTime](float delta) {
			int32 elapsedTime = time_util::getUptimeMillis() - startTime;
			int32 remainingSec = (int32)std::ceil((remainingTime - elapsedTime) / 1000.0f);
			m_promptLabel->setString(StringUtils::format(sLocaleMgr->getString("login_status_wait_for_players").c_str(), remainingSec));
			//CCLOG("Waiting for players...(%ds)", remainingSec);
		}, 1.0f, numTimes, delay, SCHEDULE_KEY_WAIT_FOR_PLAYERS);
	}
}

void GameLoadScene::stopCountdownForWaitingPlayers()
{
	if (this->isScheduled(SCHEDULE_KEY_WAIT_FOR_PLAYERS))
		this->unschedule(SCHEDULE_KEY_WAIT_FOR_PLAYERS);

}

void GameLoadScene::onInterstitialAdClosed()
{
	if(m_isShowingInterstitialAd)
		this->startGame();
}


void GameLoadScene::onInterstitialAdRequestFailed(AdError const& error)
{
	if(m_isShowingInterstitialAd)
		this->startGame();
}

NS_END
