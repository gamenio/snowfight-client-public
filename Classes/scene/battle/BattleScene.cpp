#include "BattleScene.h"

#include "common/debugging/DebugDrawer.h"
#include "common/utils/SharedPtrUtils.h"
#include "common/utils/MathTools.h"
#include "common/Machine.h"
#include "game/LocaleMgr.h"
#include "game/GameConfig.h"
#include "game/entities/DataRobot.h"
#include "game/behaviors/MyCharacter.h"
#include "game/maps/BattleMap.h"
#include "game/ObjectMgr.h"
#include "game/ClientConfig.h"
#include "scene/gamble/MyHero.h"
#include "scene/gamble/Snowball.h"
#include "scene/gamble/Footprint.h"
#include "scene/gamble/Prop.h"
#include "scene/gamble/Guidepost.h"
#include "scene/tilemap/FastTMXLayer.h"
#include "scene/func/FuncScene.h"
#include "scene/AssetsLoader.h"
#include "scene/SoundMgr.h"
#include "scene/gui/ModalDialog.h"
#include "scene/Utils.h"
#include "scene/GameLoadScene.h"
#include "gmconsole/TheaterStatusWicket.h"
#include "gmconsole/GMCommandWicket.h"
#include "BattleResultDialog.h"

using namespace cocos2d::ui;

NS_BEGIN

#define WORLD_STATUS_UPDATE_INTERVAL			5.0f // World status update interval, unit: seconds
#define SCHEDULE_KEY_TOGGLE_DEBUG_INFO			"ToggleDebugInfo"

// Network latency, unit: milliseconds
#define NETWORK_GOOD							60 // 0-60
#define NETWORK_NORMAL							100 // 61-100
#define NETWORK_BAD								150 // 101-150
#define NETWORK_VERY_BAD						// >150

// Scene element frame name
#define FRAMENAME_BUTTON_EQUIP					"battle_btn_equip.png"
#define FRAMENAME_BUTTON_QUIT					"battle_btn_quit.png"

// Debugging information format
#define WORLD_STATUS_FORMAT						"Online %d/%d players, %d players queued, %d theaters, Update diff: %d ms"
// SP(Snowball Pool), FP(Footprint Pool)
#define MY_STATUS_FORMAT						"FP: %d/%d GPS: [%d,%d,%.2f], [%d,%d] GM: %s"
#define REGIONAL_INFO_FORMAT					"%s, TheaterID %d, MapID %d, CombatGrade %d"

// Scene dialog name
#define DIALOG_NAME_NETWORK_ERROR				"NETWORK_ERROR_DIALOG"
#define DIALOG_NAME_QUIT						"QUIT_DIALOG"

// The maximum number of times to automatically restore the connection
#define MAX_AUTO_RESTORE_CONNECTIONS			3

// Battle event icon
#define BATTLE_EVENT_ICON_DANGER_ALERT			"battle_event_icon_danger_alert.png"
#define	BATTLE_EVENT_ICON_SHOWDOWN				"battle_event_icon_showdown.png"

BattleScene* BattleScene::create(RegionalInfo const& regionalInfo, PlayerProfile const& playerProfile, SafeZone* safeZone)
{
	BattleScene *pRet = new(std::nothrow) BattleScene();
	if (pRet && pRet->init(regionalInfo, playerProfile, safeZone))
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

bool BattleScene::init(RegionalInfo const& regionalInfo, PlayerProfile const& playerProfile, SafeZone* safeZone)
{
    if (!BaseScene::init())
    {
        return false;
    }

    sAnalytics->setCurrentScreen("BattleScene");

	m_playerProfile = playerProfile;
	m_regionalInfo = regionalInfo;
	m_safeZone = safeZone;
	CC_SAFE_RETAIN(safeZone);

	auto keyboardListener = EventListenerKeyboard::create();
	keyboardListener->onKeyReleased = CC_CALLBACK_2(BattleScene::onKeyReleased, this);
	this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(keyboardListener, this);

    sGameCenter->setShowAuthDialog(false);
    sGameCenter->addListener(this);
    
	auto visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();

	// Set screen safe area
	Rect safeArea = Machine::instance()->getSafeAreaRect();
	m_safeInsetLeft = Utils::getSafeInsetLeft(safeArea);
	m_safeInsetRight = Utils::getSafeInsetRight(safeArea);

	// Is it a training ground
	auto localPlayer = sGameCenter->getLocalPlayer();
	m_isTraining = localPlayer->isTrainee();
	if (m_isTraining)
	{
		if (localPlayer->getLevel() == 0)
			sAnalytics->logTutorialBegin();
	}

	// Game viewport
	m_viewportLayer = Layer::create();
	this->addChild(m_viewportLayer);

	// Network latency
	m_latencyLabel = Label::createWithSystemFont(StringUtils::format(sLocaleMgr->getString("battle_network_latency").c_str(), 0), DEFAULT_SYSTEM_FONT, 10);
    m_latencyLabel->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
	m_latencyLabel->setPosition(origin.x + (m_safeInsetLeft ? m_safeInsetLeft : 10.f), origin.y + 8);
	this->updateLabelWithLatency(0);
	this->addChild(m_latencyLabel);

	// Character status bar
	if (!m_isTraining)
	{
		m_statusBar = StatusBar::create();
		m_statusBar->setAnchorPoint(Point::ANCHOR_TOP_LEFT);
		m_statusBar->setPosition(origin.x + (m_safeInsetLeft ? m_safeInsetLeft : 10.f), origin.y + visibleSize.height - 3);
		if (localPlayer->getTutorialProcess() < TUTORIAL_PROCESS_COLLECT_MAGIC_BEANS)
			m_statusBar->setVisible(false);
		this->addChild(m_statusBar);
	}

	// Toaster
	m_toaster = Toaster::create();
	m_toaster->setAnchorPoint(Point::ANCHOR_TOP_LEFT);
	m_toaster->setPosition(origin.x + 7, origin.y + visibleSize.height - 42);
	this->addChild(m_toaster);

	// Message bar
	m_messageBar = MessageBar::create();
	m_messageBar->setAnchorPoint(Point::ANCHOR_MIDDLE_TOP);
	m_messageBar->setPosition(origin.x +  visibleSize.width / 2, origin.y + visibleSize.height - 15);
	this->addChild(m_messageBar);

	// Signal indicator
	m_signalIndicator = SignalIndicator::create();
	m_signalIndicator->setAnchorPoint(Point::ANCHOR_MIDDLE_BOTTOM);
	m_signalIndicator->setPosition(origin.x + visibleSize.width / 2, origin.y + 56);
	this->addChild(m_signalIndicator);

	// Prepare the battle countdown timer
	if (!m_isTraining)
	{
		m_preparationTimer = PreparationTimer::create();
		m_preparationTimer->setPosition(origin.x + visibleSize.width / 2, origin.y + visibleSize.height / 2);
		this->addChild(m_preparationTimer);
	}

	// Game pad
	m_gamePad = GamePad::create(m_playerProfile.controllerType);
	m_gamePad->setGamePadListener(this);
	if (m_isTraining && m_gamePad->getAttackStick())
		m_gamePad->getAttackStick()->setVisible(false);
	this->addChild(m_gamePad);

	// Quit button
	m_quitBtn = Button::create(FRAMENAME_BUTTON_QUIT, "", "", Widget::TextureResType::PLIST);
	m_quitBtn->setAnchorPoint(Point::ANCHOR_TOP_RIGHT);
	m_quitBtn->addClickEventListener(CC_CALLBACK_1(BattleScene::buttonQuitCallback, this));
	m_quitBtn->setPosition(Vec2(origin.x + visibleSize.width - m_safeInsetRight, origin.y + visibleSize.height));
	m_quitBtn->setVisible(false);
	this->addChild(m_quitBtn);
 	if (m_regionalInfo.isAppReviewModeEnabled && m_isTraining)
		m_quitBtn->setVisible(true);

	// Equipment button
	m_equipmentBtn = Button::create(FRAMENAME_BUTTON_EQUIP, "", "", Widget::TextureResType::PLIST);
	m_equipmentBtn->setAnchorPoint(Point::ANCHOR_BOTTOM_RIGHT);
	m_equipmentBtn->addClickEventListener(CC_CALLBACK_1(BattleScene::buttonEquipmentCallback, this));
	m_equipmentBtn->setPosition(Vec2(origin.x + visibleSize.width - (m_safeInsetRight ? m_safeInsetRight - 7 : 0), origin.y + 135));
	if (localPlayer->getTutorialProcess() < TUTORIAL_PROCESS_SHOW_EQUIPMENT_BAR)
		m_equipmentBtn->setVisible(false);
#if USE_DEBUG_OPTION
	m_equipmentBtn->addTouchEventListener(CC_CALLBACK_2(BattleScene::buttonEquipmentTouchCallback, this));
	if (m_isTraining)
		m_equipmentBtn->setVisible(true);
#endif // USE_DEBUG_OPTION
	this->addChild(m_equipmentBtn);

	// Smiley box
	if (!m_isTraining)
	{
		m_smileyBox = SmileyBox::create();
		m_smileyBox->setAnchorPoint(Point::ANCHOR_BOTTOM_RIGHT);
		m_smileyBox->setPosition(m_equipmentBtn->getBoundingBox().getMaxX(), m_equipmentBtn->getBoundingBox().getMaxY());
		m_smileyBox->setSelectEventListener(CC_CALLBACK_2(BattleScene::smileyBoxSelectCallback, this));
		if (localPlayer->getTutorialProcess() < TUTORIAL_PROCESS_SEND_EMO)
			m_smileyBox->setVisible(false);
		this->addChild(m_smileyBox);
	}

	// Inventory bar
	m_inventoryBar = InventoryBar::create();
	m_inventoryBar->setAnchorPoint(Point::ANCHOR_BOTTOM_RIGHT);
	m_inventoryBar->setPosition(origin.x + visibleSize.width - (m_safeInsetRight ? m_safeInsetRight : 6.f), origin.y + 3);
	if (m_isTraining)
		m_inventoryBar->setVisible(false);
	this->addChild(m_inventoryBar);

	// Screen glow border
	m_screenGlowBorder = ScreenGlowBorder::create();
	m_screenGlowBorder->setAnchorPoint(Point::ANCHOR_MIDDLE);
	m_screenGlowBorder->setPosition(origin.x + visibleSize.width / 2, origin.y + visibleSize.height / 2);
	m_screenGlowBorder->setVisible(false);
	this->addChild(m_screenGlowBorder);

	// Minimap
	m_minimap = Minimap::create();
	if (m_minimap)
	{
		m_minimap->setAnchorPoint(Point::ANCHOR_TOP_RIGHT);
		m_minimap->setPosition(origin.x + visibleSize.width - (m_safeInsetRight ? m_safeInsetRight + 3 : 9.f), origin.y + visibleSize.height - 4);
		Vec2 zoomedInPosition;
		zoomedInPosition.x = origin.x + visibleSize.width - m_safeInsetRight - m_minimap->getPositionX() + 11;
		zoomedInPosition.y = origin.y + visibleSize.height - m_minimap->getPositionY() + 11;
		m_minimap->setZoomedInPosition(zoomedInPosition);
		this->addChild(m_minimap);
	}

	// Game tutorial
	if (!localPlayer->isTutorialCompleted())
	{
		m_tutorialLayer = TutorialLayer::create(this);
		this->addChild(m_tutorialLayer);
	}

#if USE_DEBUG_OPTION
	// My status
	m_myStatusLabel = Label::createWithSystemFont(MY_STATUS_FORMAT, DEFAULT_SYSTEM_FONT, 10);
	m_myStatusLabel->setAnchorPoint(Point::ANCHOR_TOP_LEFT);
	m_myStatusLabel->setTextColor(Color4B::WHITE);
	m_myStatusLabel->enableShadow(Color4B::BLACK, Size(1, -1));
	m_myStatusLabel->setPosition(Point(origin.x + 3.0f, origin.y + visibleSize.height - 5.0f));
	m_myStatusLabel->setVisible(false);
	this->addChild(m_myStatusLabel);

	// Regional information
	m_regionalInfoLabel = Label::createWithSystemFont(StringUtils::format(REGIONAL_INFO_FORMAT, m_regionalInfo.realmName.c_str(), m_regionalInfo.theaterId, m_regionalInfo.mapId, m_regionalInfo.combatGrade), DEFAULT_SYSTEM_FONT, 10);
	m_regionalInfoLabel->setAnchorPoint(Point::ANCHOR_TOP_LEFT);
	m_regionalInfoLabel->setTextColor(Color4B::WHITE);
	m_regionalInfoLabel->enableShadow(Color4B::BLACK, Size(1, -1));
	m_regionalInfoLabel->setPosition(Point(origin.x + 3.0f, m_myStatusLabel->getBoundingBox().getMinY() - 1.0f));
	m_regionalInfoLabel->setVisible(false);
	this->addChild(m_regionalInfoLabel);

	// World status
	m_worldStatusLabel = Label::createWithSystemFont(StringUtils::format(WORLD_STATUS_FORMAT, 0, 0, 0, 0,0), DEFAULT_SYSTEM_FONT, 10);
	m_worldStatusLabel->setAnchorPoint(Point::ANCHOR_TOP_LEFT);
	m_worldStatusLabel->setTextColor(Color4B::WHITE);
	m_worldStatusLabel->enableShadow(Color4B::BLACK, Size(1, -1));
	m_worldStatusLabel->setPosition(Point(origin.x + 3.0f, m_regionalInfoLabel->getBoundingBox().getMinY() - 1.0f));
	m_worldStatusLabel->setVisible(false);
	this->addChild(m_worldStatusLabel);

	// Theater status button
	m_theaterStatusBtn = Button::create();
	m_theaterStatusBtn->setTitleFontSize(12);
	m_theaterStatusBtn->setAnchorPoint(Point::ANCHOR_TOP_LEFT);
	m_theaterStatusBtn->setPosition(Vec2(origin.x + 3.0f, m_worldStatusLabel->getBoundingBox().getMinY() - 10.0f));
	m_theaterStatusBtn->setTitleColor(Color3B(63, 183, 0));
	m_theaterStatusBtn->setTitleText("<THEATER STATUS>");
	Utils::enableBoldForLabel(m_theaterStatusBtn->getTitleLabel());
	m_theaterStatusBtn->addClickEventListener(CC_CALLBACK_1(BattleScene::buttonTheaterStatusCallback, this));
	m_theaterStatusBtn->setVisible(false);
	this->addChild(m_theaterStatusBtn);

	// GM command button
	m_gmCommandBtn = Button::create();
	m_gmCommandBtn->setTitleFontSize(12);
	m_gmCommandBtn->setAnchorPoint(Point::ANCHOR_TOP_LEFT);
	m_gmCommandBtn->setPosition(Vec2(origin.x + 3.0f, m_theaterStatusBtn->getBoundingBox().getMinY() - 10.0f));
	m_gmCommandBtn->setTitleColor(Color3B(63, 183, 0));
	m_gmCommandBtn->setTitleText("<GM COMMAND>");
	Utils::enableBoldForLabel(m_gmCommandBtn->getTitleLabel());
	m_gmCommandBtn->addClickEventListener(CC_CALLBACK_1(BattleScene::buttonGMCommandCallback, this));
	m_gmCommandBtn->setVisible(false);
	this->addChild(m_gmCommandBtn);

	// Battle timer
    m_battleTimer = BattleTimer::create();
    m_battleTimer->setAnchorPoint(Point::ANCHOR_MIDDLE_TOP);
    m_battleTimer->setPosition(origin.x + visibleSize.width / 2, origin.y + visibleSize.height - 5.0f);
    m_battleTimer->setVisible(false);
    this->addChild(m_battleTimer);

#endif // USE_DEBUG_OPTION

	this->initBattleground();

    this->scheduleUpdate();

    sSoundMgr->play(SOUND_BATTLE_AMBIENT, true);

    return true;
}

Scene* BattleScene::addToScene()
{
	auto scene = Scene::create();
	scene->addChild(this);
	return scene;
}

BattleScene::BattleScene() :
	m_prevProjection(Director::Projection::DEFAULT),
	m_safeInsetLeft(0),
	m_safeInsetRight(0),
	m_isTraining(false),
	m_smileyBox(nullptr),
	m_equipmentBtn(nullptr),
	m_viewportLayer(nullptr),
	m_gameMapLayer(nullptr),
	m_latencyLabel(nullptr),
	m_statusBar(nullptr),
	m_toaster(nullptr),
	m_messageBar(nullptr),
	m_gamePad(nullptr),
	m_preparationTimer(nullptr),
	m_inventoryBar(nullptr),
	m_screenGlowBorder(nullptr),
	m_minimap(nullptr),
	m_safeZone(nullptr),
	m_dangerZone(nullptr),
	m_tutorialLayer(nullptr),
	m_hasTryRestoreConn(false),
	m_autoRestoreConnCount(0),
	m_loadingView(nullptr),
	m_isShowDebugInfo(false),
	m_myStatusLabel(nullptr),
	m_worldStatusLabel(nullptr),
	m_regionalInfoLabel(nullptr),
	m_quitBtn(nullptr),
	m_theaterStatusBtn(nullptr),
	m_gmCommandBtn(nullptr),
	m_battleTimer(nullptr),
	m_isQueryingWorldStatus(false)
{
}

BattleScene::~BattleScene()
{
	CC_SAFE_RELEASE_NULL(m_safeZone);

	m_smileyBox = nullptr;
	m_equipmentBtn = nullptr;
    m_viewportLayer = nullptr;
	m_gameMapLayer = nullptr;
    m_latencyLabel = nullptr;
	m_statusBar = nullptr;
	m_toaster = nullptr;
	m_messageBar = nullptr;
	m_gamePad = nullptr;
	m_preparationTimer = nullptr;
	m_inventoryBar = nullptr;
	m_screenGlowBorder = nullptr;
	m_minimap = nullptr;
	m_dangerZone = nullptr;
	m_tutorialLayer = nullptr;
	m_loadingView = nullptr;
	m_myStatusLabel = nullptr;
	m_worldStatusLabel = nullptr;
	m_regionalInfoLabel = nullptr;
	m_quitBtn = nullptr;
	m_theaterStatusBtn = nullptr;
	m_gmCommandBtn = nullptr;
	m_battleTimer = nullptr;
}

void BattleScene::onEnter()
{
	BaseScene::onEnter();

	m_prevProjection = Director::getInstance()->getProjection();
	Director::getInstance()->setProjection(Director::Projection::_2D);
}

void BattleScene::onExit()
{
	Director::getInstance()->setProjection(m_prevProjection);
	sAssetsLoader->unloadWorldBasic();

	BaseScene::onExit();
}

void BattleScene::onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event)
{
	switch (keyCode)
	{
	case EventKeyboard::KeyCode::KEY_ENTER:
	{
#if USE_DEBUG_OPTION
		GMCommandWicket* wicket = GMCommandWicket::create(this);
		wicket->show();
#endif
		break;
	}
	default:
		break;
	}
}

void BattleScene::stopWorldAndPlayNext()
{
	this->stopWorld([this]() {
		this->playNext();
	});
}

void BattleScene::initBattleground()
{
	World* world = World::getInstance();
	world->setWorldLifecycleListener(this);
	world->setWorldInitListener(this);
	world->setNetworkStatusListener(this);
	world->setWorldAuthListener(this);
	world->setBattleUpdateListener(this);
	world->addMyCharacterListener(this);
	world->addMessageListener(this);
	world->addItemApplicationListener(this);

	// Set the character status bar
	if (m_statusBar)
	{
		DataPlayer* myChar = world->getMyself();
		NS_ASSERT(myChar != nullptr);
		m_statusBar->setData(myChar);
	}

	this->setupViewport();
}

void BattleScene::onWorldStopped(World* world)
{
	World::destoryInstance();

	if (m_onWorldStoppedCallback)
		m_onWorldStoppedCallback();
}

void BattleScene::onNetworkError(NetworkError const& error)
{
	CCLOG("Network error! opcode:%d errorcode:%d message: %s", error.getOpcode(), error.getErrorCode(), error.getMessage().c_str());

	if (!this->tryRestoreConnection())
	{
		m_signalIndicator->hide();
		this->dismissLoadingView();

		switch (error.getErrorCode())
		{
		case NetworkError::CONNECTION_FAILED:
		case NetworkError::CONNECTION_TIMED_OUT:
		case NetworkError::CONNECTION_RESOLVE_FAILED:
			this->showNetworkErrorDialog(sLocaleMgr->getString("battle_dlg_title_connection_failed"), sLocaleMgr->getString("battle_dlg_msg_connection_failed"), sLocaleMgr->getString("battle_dlg_btn_connect"));
			break;
		default:
			this->showNetworkErrorDialog(sLocaleMgr->getString("battle_dlg_title_connection_failed"), StringUtils::format(sLocaleMgr->getString("battle_dlg_msg_network_error").c_str(), error.getErrorCode()), sLocaleMgr->getString("battle_dlg_btn_connect"));
			break;
		}
	}
}

void BattleScene::onNetworkRestored()
{
	World::getInstance()->resendAuthProof();
}

void BattleScene::onWorldSessionTimedout()
{
	CCLOG("Session timed out.");
	if (!this->tryRestoreConnection())
	{
		m_signalIndicator->hide();
		this->dismissLoadingView();

		this->showNetworkErrorDialog(sLocaleMgr->getString("battle_dlg_title_session_timedout"), sLocaleMgr->getString("battle_dlg_msg_session_timedout"), sLocaleMgr->getString("battle_dlg_btn_logon"));
	}
}

void BattleScene::onWorldAuthSucceeded()
{
	World::getInstance()->sendJoinTheater();
}

void BattleScene::onWorldSessionExpired()
{
	CCLOG("Session expired.");
	m_signalIndicator->hide();
	this->dismissLoadingView();
	this->showNetworkErrorDialog(sLocaleMgr->getString("battle_dlg_title_session_expired"), sLocaleMgr->getString("battle_dlg_msg_session_expired"));
}

void BattleScene::onWorldRecvQueueFulL()
{
	CCLOG("Receive queue is full.");

	if (!this->tryRestoreConnection())
	{
		m_signalIndicator->hide();
		this->dismissLoadingView();

		this->showNetworkErrorDialog(sLocaleMgr->getString("battle_dlg_title_connection_failed"), StringUtils::format(sLocaleMgr->getString("battle_dlg_msg_network_error").c_str(), -1), sLocaleMgr->getString("battle_dlg_btn_connect"));
	}
}

void BattleScene::onFlashMessage(FlashMessage const& flashMsg)
{
	MessageToast::Severity severity = static_cast<MessageToast::Severity>(flashMsg.severity());
	m_toaster->addToast(flashMsg.message(), severity);
}

void BattleScene::onPlayerActionMessage(PlayerActionMessage const& message)
{
	switch (message.type())
	{
	case PlayerActionMessage::PLAYER_LOGGED_IN:
	{
		ActionToast* toast = ActionToast::create(ActionToast::ACTION_TYPE_ACTOR1_LOGGED_IN);
		toast->setActor1(message.name(), ObjectGuid::EMPTY);
		m_toaster->addToast(toast);
		break;
	}
	default:
		break;
	}
}

void BattleScene::onDeathMessage(DeathMessage const& message)
{
	DataPlayer* myChar = World::getInstance()->getMyself();
	if (!myChar)
		return;

	ObjectGuid killerGuid(message.killer().guid());
	if (!killerGuid.isEmpty())
	{
		ActionToast* toast = ActionToast::create(ActionToast::ACTION_TYPE_ACTOR1_KILL_ACTOR2);
		toast->setActor1(message.killer().name(), killerGuid);
		toast->setActor2(message.victim().name(), ObjectGuid(message.victim().guid()));
		m_toaster->addToast(toast);
	}
	else
	{
		ActionToast* toast = ActionToast::create(ActionToast::ACTION_TYPE_ACTOR1_DIED);
		toast->setActor1(message.victim().name(), ObjectGuid(message.victim().guid()));
		m_toaster->addToast(toast);
	}
}

void BattleScene::onBattleUpdate(BattleUpdate const& update)
{
	MyCharacter* myChar = World::getInstance()->getMyCharacter();
	if (!myChar)
		return;

	if ((update.update_flags() & BATTLE_UPDATEFLAG_STATE) != 0)
	{
		switch (update.state())
		{
		case BATTLE_STATE_PREPARING:
		{
			if (m_preparationTimer)
			{
				NSTime timeLeft = update.preparation_duration() - (time_util::getUptimeMillis() - update.start_time());
				if (timeLeft > 0)
					m_preparationTimer->start(timeLeft);
			}
#if USE_DEBUG_OPTION
			m_battleTimer->setDuration(update.battle_duration());
#endif // USE_DEBUG_OPTION
			break;
		}
		case BATTLE_STATE_IN_PROGRESS:
		{
#if USE_DEBUG_OPTION
			NSTime timeLeft = update.battle_duration() - (time_util::getUptimeMillis() - update.start_time());
			if (timeLeft > 0)
			{
				m_battleTimer->setDuration(timeLeft);
				m_battleTimer->start();
			}
#endif // USE_DEBUG_OPTION

			if (m_tutorialLayer)
				m_tutorialLayer->startTutorial();

			if (!myChar->isMoveEnabled())
			{
				MyHero* myHero = m_gameMapLayer->getGameObject<MyHero>(myChar->getData()->getGuid());
				myHero->showArrow(0.5f, 2.f);

				myChar->setMoveEnabled(true);
			}
			if (m_dangerZone && m_dangerZone->isStopped())
				m_dangerZone->start(update.start_time());
			break;
		}
		case BATTLE_STATE_ENDING:
		{
			TutorialService* service = TutorialService::getInstance();
			if (service->isEnabled() && m_isTraining)
				service->triggerEvent(TUTORIAL_EVENT_BATTLE_ENDING);
			break;
		}
		default:
			break;
		}

		CCLOG("BATTLE STATE UPDATE state: %d, start_time: %d, preparation_duration: %d, battle_duration: %d ", update.state(), update.start_time(), update.preparation_duration(), update.battle_duration());
	}


	if ((update.update_flags() & BATTLE_UPDATEFLAG_ALIVE_COUNT) != 0)
	{
		if (m_minimap)
		{
			m_minimap->getPlayerCountBar()->setAliveCount(update.alive_count());

			if (!m_triggeredBattleEvents.test(BATTLE_EVENT_SHOWDOWN) && update.alive_count() <= 2)
			{
				if (update.alive_count() <= 2)
				{
					m_messageBar->show(sLocaleMgr->getString("battle_event_showdown"), MessageBar::ALERT, BATTLE_EVENT_ICON_SHOWDOWN, MESSAGEBAR_DURATION_LONG);
					m_triggeredBattleEvents.set(BATTLE_EVENT_SHOWDOWN);
				}
			}
		}

		CCLOG("ALIVE COUNT UPDATE alive_count: %d", update.alive_count());
	}
}

void BattleScene::onInitSelfCompleted(DataPlayer* myChar)
{
	if (m_autoRestoreConnCount < MAX_AUTO_RESTORE_CONNECTIONS)
		m_hasTryRestoreConn = false;
	m_signalIndicator->hide();
	this->dismissLoadingView();
}

void BattleScene::onBattleResult(BattleResult const& result)
{
	World* world = World::getInstance();
	if (!world->isLocalPlayerAuthed(sGameCenter->getLocalPlayer()))
		return;

	bool needToSave = false;
	LocalPlayer* localPlayer = sGameCenter->getLocalPlayer();
	if (result.extra_money() > 0)
	{
		localPlayer->addMoney(result.extra_money());
		needToSave = true;
	}
	if (result.level() > 0)
	{
		localPlayer->setLevel(result.level());
		localPlayer->setExperience(result.experience());
		needToSave = true;
	}
	if (result.kill_count() > 0)
	{
		int32 totalKills = localPlayer->getScoreValue(LEADERBOARD_ID_TOTAL_KILLS);
        totalKills += result.kill_count();
		localPlayer->setScoreValue(LEADERBOARD_ID_TOTAL_KILLS, totalKills);
		needToSave = true;
	}

	if (world->getSession())
	{
		WorldSession* session = world->getSession()->asWorldSession();
		if (session)
			session->closeDelayed();
	}

	if(m_dangerZone)
		m_dangerZone->stop();

	BattleResultDialog* dialog = BattleResultDialog::create();
	dialog->setBattleOutcome(static_cast<BattleOutcome>(result.outcome()));
	dialog->setMoney(result.money(), result.extra_money());
	dialog->setKillCount(result.kill_count());
	dialog->setExtraXP(result.extra_xp());
	dialog->setRankNo(result.rank_no());
	bool isBackHomeSuggestionAccepted = localPlayer->isSuggestionAccepted(SUGGESTION_BACK_HOME);
	int32 amount = result.money() + result.extra_money();
	dialog->setHomeButtonCallback([this, isBackHomeSuggestionAccepted](Ref* sender)
	{
		this->stopWorld([this, isBackHomeSuggestionAccepted]() {
			this->backToHome(RewardedAdConfig(),isBackHomeSuggestionAccepted);
		});
	});
	dialog->setWatchAdButtonCallback([this, amount](Ref* sender)
	{
		this->stopWorld([this, amount]() {
			RewardedAdConfig adConfig;
			adConfig.value1 = amount;
			adConfig.adType = RewardedAdConfig::AD_TYPE_BATTLE_OUTCOME;
			this->backToHome(adConfig, true);
		});
	});
	if (amount <= 0)
		dialog->setWatchAdButtonVisible(false);
	if (!isBackHomeSuggestionAccepted)
	{
		dialog->setShowFingerTap(true);
		dialog->setWatchAdButtonVisible(false);
		localPlayer->setSuggestionAccepted(SUGGESTION_BACK_HOME, true);
		needToSave = true;
	}

	dialog->show();

	if (needToSave)
		localPlayer->saveDataAsync();

	CCLOG("BATTLE RESULT outcome: %d", result.outcome());
}

void BattleScene::onItemApplicationUpdate(ItemApplicationUpdate const& update)
{
	ObjectGuid targetGuid(update.target());
	if (targetGuid == World::getInstance()->getMyself()->getGuid())
	{
		auto const& info = update.app();
		this->handleSelfItemApplicationInfo(info);
	}
}

void BattleScene::onItemApplicationUpdateAll(ItemApplicationUpdateAll const& updateAll)
{
	ObjectGuid targetGuid(updateAll.target());
	if (targetGuid == World::getInstance()->getMyself()->getGuid())
	{
		auto const& appList = updateAll.app_list();
		for (auto it = appList.begin(); it != appList.end(); ++it)
		{
			auto const& info = *it;
			this->handleSelfItemApplicationInfo(info);
		}
	}
}

void BattleScene::onSafeZoneUpdated(int32 currRadius)
{
	if(!m_safeZone)
		return;

	DataPlayer* myChar = World::getInstance()->getMyself();
	if (!myChar)
		return;

	if (m_triggeredBattleEvents.test(BATTLE_EVENT_DANGER_ALERT))
		return;

	MapData* mapData = myChar->getMapData();
	int32 mapWidth = (int32)mapData->getMapSize().width;
	int32 mapHeight = (int32)mapData->getMapSize().height;
	TileCoord const& center = m_safeZone->getCenter();
	int32 top = center.y;
	int32 bottom = mapHeight - center.y - 1;
	int32 left = center.x;
	int32 right = mapWidth - center.x - 1;
	int32 alertRadius = std::max(right, std::max(left, std::max(top, bottom))) - (MAP_MARGIN_IN_TILES - 1);
	if (currRadius <= alertRadius)
	{
		m_messageBar->show(sLocaleMgr->getString("battle_event_danger_alert"), MessageBar::ALERT, BATTLE_EVENT_ICON_DANGER_ALERT, MESSAGEBAR_DURATION_LONG);
		m_triggeredBattleEvents.set(BATTLE_EVENT_DANGER_ALERT);
	}
}

void BattleScene::handleSelfItemApplicationInfo(ItemApplicationInfo const& info)
{
	ItemTemplate const* tmpl = sObjectMgr->getItemTemplate(info.item_id());
	NS_ASSERT(tmpl);
	ItemApplicationTemplate const* appTmpl = sObjectMgr->getItemApplicationTemplate(tmpl->appId);
	NS_ASSERT(appTmpl);

	if (info.apply())
		this->applyItemForSelf(appTmpl, info.duration(), info.remaining_time());
	else
		this->unapplyItemForSelf(appTmpl);
}

void BattleScene::applyItemForSelf(ItemApplicationTemplate const* appTmpl, int32 duration, int32 remainingTime)
{
	switch (appTmpl->visualId)
	{
	case ITEM_VISUAL_HIGHLIGHT_HIDING_SPOTS:
		m_screenGlowBorder->setVisible(true);
		break;
	case ITEM_VISUAL_ATTACK_STICK_CHARGE_ENABLE:
		m_gamePad->setChargeEnabled(true);
	default:
		break;
	}
}

void BattleScene::unapplyItemForSelf(ItemApplicationTemplate const* appTmpl)
{
	switch (appTmpl->visualId)
	{
	case ITEM_VISUAL_HIGHLIGHT_HIDING_SPOTS:
		m_screenGlowBorder->setVisible(false);
		break;
	case ITEM_VISUAL_ATTACK_STICK_CHARGE_ENABLE:
		m_gamePad->setChargeEnabled(false);
		break;
	default:
		break;
	}
}

void BattleScene::setupViewport()
{
	DataPlayer* myChar = World::getInstance()->getMyself();

	//  Initialize the map
	m_gameMapLayer = GameMapLayer::create();
	m_viewportLayer->addChild(m_gameMapLayer);

	// Initialize the danger zone
	if (m_safeZone)
	{
		Rect viewport;
		viewport.origin = Director::getInstance()->getVisibleOrigin();
		viewport.size = Director::getInstance()->getVisibleSize();
		m_dangerZone = DangerZone::create(myChar->getMapData(), viewport);
		m_dangerZone->setSafeZone(m_safeZone);
		if(m_minimap)
			m_dangerZone->addSafeZoneListener(m_minimap);
		m_dangerZone->addSafeZoneListener(this);
		m_viewportLayer->addChild(m_dangerZone);
	}	

#if NS_DEBUG
	sDebugDrawer->attach(m_viewportLayer, INT_MAX);
#endif // NS_DEBUG

	this->setViewPointCenter(myChar->getPosition());
}

void BattleScene::setViewPointCenter(Point const& position)
{
	Size winSize = Director::getInstance()->getWinSize();
	//CCLOG("setViewPointCenter position:%f %f", position.x, position.y);

	Point centerOfView(winSize.width / 2, winSize.height / 2);
	Point viewPoint = centerOfView - position;

	m_viewportLayer->setPosition(viewPoint);
}

#if USE_DEBUG_OPTION

void BattleScene::onWorldStatus(WorldStatus const& status)
{
	m_worldStatusLabel->setString(StringUtils::format(WORLD_STATUS_FORMAT,
		status.online_players(), status.max_players(),
		status.queued_players(),
		status.theater_count(),
		status.update_diff()
	));
	m_isQueryingWorldStatus = false;
}

void BattleScene::buttonTheaterStatusCallback(Ref* sender)
{
	TheaterStatusWicket* wicket = TheaterStatusWicket::create(this);
	wicket->show();
}

void BattleScene::buttonGMCommandCallback(Ref* sender)
{
	GMCommandWicket* wicket = GMCommandWicket::create(this);
	wicket->show();
}

void BattleScene::updateMyStatus()
{
	DataPlayer* myself = World::getInstance()->getMyself();
	if (!myself)
		return;

	TileCoord tileCoord(myself->getMapData()->getMapSize(), myself->getPosition());
	FootprintPool* fpPool = FootprintPool::getInstance();

	float offsetZ;
	float zOrder = myself->getMapData()->getTileZForPos(myself->getPosition(), &offsetZ) + offsetZ;
	m_myStatusLabel->setString(StringUtils::format(MY_STATUS_FORMAT,
		fpPool->getUnavailableCount(), fpPool->getAvailableCount() + fpPool->getUnavailableCount(),
		(int32)myself->getPosition().x, (int32)myself->getPosition().y, zOrder,
		tileCoord.x, tileCoord.y,
		myself->isGM() ? "On" : "Off"
	));
}

void BattleScene::updateWorldStatus(float delta)
{
	World* world = World::getInstance();
	DataPlayer* myChar = world->getMyself();
	if (!myChar)
		return;

	m_worldStatusUpdateTimer.update(delta);
	if (m_worldStatusUpdateTimer.passed() && !m_isQueryingWorldStatus)
	{
		world->sendQueryWorldStatus();
		m_isQueryingWorldStatus = true;

		m_worldStatusUpdateTimer.reset();
	}
}


void BattleScene::buttonEquipmentTouchCallback(Ref* sender, Widget::TouchEventType eventType)
{
	switch (eventType)
	{
	case Widget::TouchEventType::BEGAN:
		this->scheduleOnce([this](float dt) {
			this->toggleDebugInfo();
		}, 1.f, SCHEDULE_KEY_TOGGLE_DEBUG_INFO);
		break;
	case Widget::TouchEventType::CANCELED:
		this->unschedule(SCHEDULE_KEY_TOGGLE_DEBUG_INFO);
		break;
	default:
		break;
	}
}

void BattleScene::toggleDebugInfo()
{
	m_isShowDebugInfo = !m_isShowDebugInfo;

	if (m_isShowDebugInfo)
	{
		m_myStatusLabel->setVisible(true);
		m_regionalInfoLabel->setVisible(true);
        m_battleTimer->setVisible(true);
	}
	else
	{
		m_myStatusLabel->setVisible(false);
		m_regionalInfoLabel->setVisible(false);
        m_battleTimer->setVisible(false);
	}
	if (!m_regionalInfo.isAppReviewModeEnabled || !m_isTraining)
	{
		m_quitBtn->setVisible(m_isShowDebugInfo);
	}

	World* world = World::getInstance();
	WorldSession* session = world->getSession() ? world->getSession()->asWorldSession() : nullptr;
	if (session && session->hasGMPermission())
	{
		if (m_isShowDebugInfo)
		{
			world->addWorldStatusListener(this);
			m_worldStatusUpdateTimer.setInterval(WORLD_STATUS_UPDATE_INTERVAL);
			m_worldStatusUpdateTimer.setPassed();

			m_worldStatusLabel->setVisible(true);
			m_theaterStatusBtn->setVisible(true);
			m_gmCommandBtn->setVisible(true);
		}
		else
		{
			world->removeWorldStatusListener(this);
			m_worldStatusLabel->setVisible(false);
			m_theaterStatusBtn->setVisible(false);
			m_gmCommandBtn->setVisible(false);
		}
	}
}

#endif // USE_DEBUG_OPTION


void BattleScene::backToHome(RewardedAdConfig const& adConfig, bool isEnableInterstitialAd)
{
	this->cleanupBeforeExit();

	sAssetsLoader->loadFunctional([this, adConfig, isEnableInterstitialAd]() {
		auto scene = FuncScene::create(adConfig, isEnableInterstitialAd)->addToScene();
		Director::getInstance()->replaceScene(scene);
	});
}

void BattleScene::playNext()
{
	this->cleanupBeforeExit();

	auto scene = GameLoadScene::create(m_playerProfile, false)->addToScene();
	Director::getInstance()->replaceScene(scene);
}

void BattleScene::cleanupBeforeExit()
{
#if NS_DEBUG
	sDebugDrawer->detach();
#endif // NS_DEBUG

	TutorialService::destoryInstance();
	sGameCenter->removeListener(this);
	sAssetsLoader->unloadWorldGamble();
}

void BattleScene::stopWorldAndBackToHome()
{
	this->stopWorld([this]() {
		this->backToHome(RewardedAdConfig(), true);
	});
}

void BattleScene::stopWorld(std::function<void()> callback)
{
	sGameCenter->reportScores(LEADERBOARD_ID_TOTAL_KILLS);

	if(m_dangerZone)
		m_dangerZone->stop();
	this->unscheduleUpdate();

	DataPlayer* myChar = World::getInstance()->getMyself();
	NS_ASSERT(myChar);
	sAssetsLoader->unloadMapTilesets(myChar->getMapData());

	m_onWorldStoppedCallback = callback;
	World::getInstance()->stopDelayed();
}

void BattleScene::updateLatency()
{
    MyCharacter* myChar = World::getInstance()->getMyCharacter();
    if (!myChar || !myChar->getSession())
        return;
    
	NSTime latency = myChar->getSession()->getLatency();
	this->updateLabelWithLatency(latency);
}

void BattleScene::updateLabelWithLatency(NSTime latency)
{
	if (latency <= NETWORK_GOOD)
		m_latencyLabel->setTextColor(Color4B(63, 183, 0, 255));
	else if (latency <= NETWORK_NORMAL)
		m_latencyLabel->setTextColor(Color4B(250, 180, 5, 255));
	else if (latency <= NETWORK_BAD)
		m_latencyLabel->setTextColor(Color4B(239, 113, 0, 255));
	else // NETWORK_VERY_BAD
		m_latencyLabel->setTextColor(Color4B(224, 41, 27, 255));

	Utils::enableBoldForLabel(m_latencyLabel);
	m_latencyLabel->setString(StringUtils::format(sLocaleMgr->getString("battle_network_latency").c_str(), latency));
}

#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS

void BattleScene::onAuthStarted(AuthOperation op)
{
    this->showLoadingView();
}

void BattleScene::onAuthFinished(AuthOperation op, AuthResponse const& response)
{
    this->dismissLoadingView();
    
    WorldSession* session = nullptr;
    World* world = World::getInstance();
    if (world->getSession())
    {
        session = world->getSession()->asWorldSession();
    }
    
    if(response.error.code == GAMECENTER_ERROR_ACCOUNT_CHANGED)
    {
        if (session)
            session->closeDelayed();
        
        MessageDialog* dialog = MessageDialog::create();
        dialog->setTitle(sLocaleMgr->getString("gamecenter_dlg_title_account_switched"));
        dialog->setMessage(sLocaleMgr->getString("gamecenter_dlg_msg_account_switched"));
        dialog->addOkButton([this](Ref* sender) {
            Utils::exitApp();
        });
        dialog->show();
    }
    else
    {
        bool isAuthed = false;
        if (session)
        {
            LocalPlayer* localPlayer = sGameCenter->getLocalPlayer();
            isAuthed = world->isLocalPlayerAuthed(localPlayer);
            if (!isAuthed)
                session->closeDelayed();
        }

        if (!isAuthed)
        {
            MessageDialog* dialog = MessageDialog::create();
            dialog->setTitle(sLocaleMgr->getString("battle_dlg_title_not_authenticated"));
            dialog->setMessage(sLocaleMgr->getString("battle_dlg_msg_not_authenticated"));
            dialog->addOkButton([this](Ref* sender) {
                this->stopWorldAndBackToHome();
            });
            dialog->show();
        }
    }
}


#else

void BattleScene::onAuthStarted(AuthOperation op)
{
}

void BattleScene::onAuthFinished(AuthOperation op, AuthResponse const& response)
{
}

#endif // CC_TARGET_PLATFORM

void BattleScene::onReportScoreSuccess()
{
    
}

void BattleScene::onReportScoreFail(GameCenterError const& error)
{
    
}


void BattleScene::smileyBoxSelectCallback(Ref* sender, uint16 code)
{
    // CCLOG("Selected smiley code: %d", code);
	World::getInstance()->sendSmiley(code);
}

void BattleScene::buttonEquipmentCallback(Ref* sender)
{
#if USE_DEBUG_OPTION
	if (!this->isScheduled(SCHEDULE_KEY_TOGGLE_DEBUG_INFO))
		return;
	else
		this->unschedule(SCHEDULE_KEY_TOGGLE_DEBUG_INFO);
#endif // USE_DEBUG_OPTION

	DataPlayer* myChar = World::getInstance()->getMyself();
	if (!myChar)
		return;

	MyHero* myHero = m_gameMapLayer->getGameObject<MyHero>(myChar->getGuid());
	if (!myHero)
		return;

	sSoundMgr->play(SOUND_BUTTON);
	myHero->toggleEquipmentBar();

	auto service = TutorialService::getInstance();
	if (service->isEnabled())
		service->triggerEvent(TUTORIAL_EVENT_EQUIPMENT_BUTTON_TAPPED);
}

bool BattleScene::tryRestoreConnection()
{
	if (!m_hasTryRestoreConn)
	{
		CCLOG("Try to restore connection...(%d/%d)", m_autoRestoreConnCount + 1, MAX_AUTO_RESTORE_CONNECTIONS);
		if (World::getInstance()->tryRestoreConnection())
			m_signalIndicator->show(SignalIndicator::SIGNAL_LEVEL_LOST_CONNECTION);
		else
			this->showNetworkErrorDialog(sLocaleMgr->getString("battle_dlg_title_disconnect"), sLocaleMgr->getString("battle_dlg_msg_disconnect"));

		if (m_autoRestoreConnCount < MAX_AUTO_RESTORE_CONNECTIONS)
			++m_autoRestoreConnCount;
		m_hasTryRestoreConn = true;

		return true;
	}

	return false;
}

void BattleScene::update(float delta)
{
	DataPlayer* myChar = World::getInstance()->getMyself();
	if (myChar)
		this->setViewPointCenter(myChar->getPosition());

#if USE_DEBUG_OPTION
	this->updateWorldStatus(delta);
#endif // USE_DEBUG_OPTION

	m_gameMapLayer->update(delta);
	m_inventoryBar->update(delta);
	if(m_statusBar)
		m_statusBar->update(delta);
	if(m_minimap)
		m_minimap->update(delta);

	m_gameMapLayer->cleanAfterUpdate();
	m_inventoryBar->cleanAfterUpdate();

	this->updateLatency();

#if USE_DEBUG_OPTION
	this->updateMyStatus();
#endif // USE_DEBUG_OPTION
}

void BattleScene::onMoveControlMoving(Ref* sender, float direction)
{
	DataPlayer* myChar = World::getInstance()->getMyself();
	if (myChar)
	{
		MyHero* myHero = m_gameMapLayer->getGameObject<MyHero>(myChar->getGuid());
		if (myHero)
			myHero->moveBy(direction);
	}
}

void BattleScene::onMoveControlStopped(Ref* sender)
{
	DataPlayer* myChar = World::getInstance()->getMyself();
	if (myChar)
	{
		MyHero* myHero = m_gameMapLayer->getGameObject<MyHero>(myChar->getGuid());
		if (myHero)
			myHero->moveStop();
	}
}

void BattleScene::onAttackControlAiming(Ref* sender, float direction)
{
	DataPlayer* myChar = World::getInstance()->getMyself();
	if (myChar)
	{
		MyHero* myHero = m_gameMapLayer->getGameObject<MyHero>(myChar->getGuid());
		if (myHero)
			myHero->setAimingDirection(direction);
	}
}

void BattleScene::onAttackControlFire(Ref* sender, float direction)
{
	DataPlayer* myChar = World::getInstance()->getMyself();
	if (myChar)
	{
		MyHero* myHero = m_gameMapLayer->getGameObject<MyHero>(myChar->getGuid());
		if (myHero)
			myHero->fire(direction);
	}
}

void BattleScene::onAttackControlFire(Ref * sender, Point const& aimPoint)
{
	DataPlayer* myChar = World::getInstance()->getMyself();
	if (myChar)
	{
		MyHero* myHero = m_gameMapLayer->getGameObject<MyHero>(myChar->getGuid());
		if (myHero)
		{
			Point dest = aimPoint;
			if(dest.x >= 0 && dest.y >= 0)
				dest = m_viewportLayer->convertToNodeSpace(aimPoint);
			myHero->fire(dest);
		}
	}
}

void BattleScene::onAttackControlCharging(Ref* sender)
{
	DataPlayer* myChar = World::getInstance()->getMyself();
	if (myChar)
	{
		MyHero* myHero = m_gameMapLayer->getGameObject<MyHero>(myChar->getGuid());
		if (myHero)
			myHero->charge();
	}
}

void BattleScene::onAttackControlCanceling(Ref* sender)
{
	DataPlayer* myChar = World::getInstance()->getMyself();
	if (myChar)
	{
		MyHero* myHero = m_gameMapLayer->getGameObject<MyHero>(myChar->getGuid());
		if (myHero)
			myHero->fireStop(false);
	}
}

void BattleScene::onAttackControlCanceled(Ref* sender)
{
	DataPlayer* myChar = World::getInstance()->getMyself();
	if (myChar)
	{
		MyHero* myHero = m_gameMapLayer->getGameObject<MyHero>(myChar->getGuid());
		if (myHero)
			myHero->fireStop(true);
	}
}

void BattleScene::showQuitDialog()
{
	if (ModalDialog::getDialogByName(DIALOG_NAME_QUIT))
		return;

	MessageDialog* dialog = MessageDialog::create();
	dialog->setName(DIALOG_NAME_QUIT);
	dialog->setTitle(sLocaleMgr->getString("battle_dlg_title_quit"));
	dialog->setMessage(sLocaleMgr->getString("battle_dlg_msg_quit"));
	dialog->addTickButton([this](Ref* sender)
	{
		this->stopWorldAndBackToHome();

	}, MessageDialog::BUTTON_NEGATIVE);
	dialog->addCrossButton(nullptr, MessageDialog::BUTTON_POSITIVE);
	dialog->show();
}

void BattleScene::buttonQuitCallback(Ref* sender)
{
	this->showQuitDialog();
	sSoundMgr->play(SOUND_BUTTON);

	/*To navigate back to native iOS screen(if present) without quitting the application  ,do not use Director::getInstance()->end() and exit(0) as given above,instead trigger a custom event created in RootViewController.mm as below*/

	//EventCustom customEndEvent("game_scene_close_event");
	//_eventDispatcher->dispatchEvent(&customEndEvent);
}

void BattleScene::showLoadingView()
{
	if (!m_loadingView)
	{
		m_loadingView = LoadingView::create(this);
		m_loadingView->show();
	}
}

void BattleScene::dismissLoadingView()
{
	if (m_loadingView)
	{
		m_loadingView->dismiss();
		m_loadingView = nullptr;
	}
}

void BattleScene::showNetworkErrorDialog(std::string const& title, std::string const& message, std::string const& retryButtonTitle /*= ""*/)
{
	MessageDialog* dialog = dynamic_cast<MessageDialog*>(ModalDialog::getDialogByName(DIALOG_NAME_NETWORK_ERROR));
	if (dialog)
	{
		dialog->dismiss();
		dialog = nullptr;
	}

	dialog = MessageDialog::create();
	dialog->setName(DIALOG_NAME_NETWORK_ERROR);
	dialog->setTitle(title);
	dialog->setMessage(message);
	if (!retryButtonTitle.empty())
	{
		dialog->addPositiveButton(retryButtonTitle, [this](Ref* sender) {
			if (World::getInstance()->tryRestoreConnection())
				this->showLoadingView();
			else
				this->stopWorldAndBackToHome();
		});
		dialog->addCancelButton([this](Ref* sender) {
			this->stopWorldAndBackToHome();
		});
	}
	else
	{
		dialog->addOkButton([this](Ref* sender) {
			this->stopWorldAndBackToHome();
		});
	}
	dialog->show();
}


NS_END