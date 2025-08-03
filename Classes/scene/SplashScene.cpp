#include "SplashScene.h"

#include <iomanip>

#include "common/Machine.h"
#include "game/UserPreferences.h"
#include "game/firservice/FirebaseService.h"
#include "game/nts/TimeService.h"
#include "game/ObjectMgr.h"
#include "game/ClientConfig.h"
#include "func/FuncScene.h"
#include "scene/SoundMgr.h"
#include "gui/ModalDialog.h"
#include "review/StoreReview.h"
#include "AssetsLoader.h"
#include "Utils.h"
#include "GameLoadScene.h"

NS_BEGIN

#define IMG_SPLASH				"launch_image.png"
#define IMG_SPLASH_NARROW		"launch_image_narrow.png"

#define SPLASH_DURATION				2.f

#define LOADING_INDICATOR_SIZE		Size(50, 9)

SplashScene::SplashScene() :
	m_isLoadPlayerDataFailed(false),
	m_background(nullptr),
	m_main(nullptr),
	m_loadingIndicator(nullptr),
	m_queuedTasksLabel(nullptr)
{
}

SplashScene::~SplashScene()
{
	m_background = nullptr;
	m_main = nullptr;
	m_loadingIndicator = nullptr;
	m_queuedTasksLabel = nullptr;
}

Scene* SplashScene::createScene()
{
	auto scene = Scene::create();
	auto layer = SplashScene::create();
	scene->addChild(layer);
	return scene;
}

bool SplashScene::init()
{
	if (!BaseScene::init())
		return false;

	auto visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();

	m_background = LayerColor::create(Color4B::WHITE);
	this->addChild(m_background);

	m_main = Sprite::create();
	if (Machine::instance()->isNarrowScreen())
		m_main->setTexture(this->autoUncacheImage(IMG_SPLASH_NARROW));
	else
		m_main->setTexture(this->autoUncacheImage(IMG_SPLASH));
	float scale = visibleSize.height / m_main->getContentSize().height;
	m_main->setContentSize(m_main->getContentSize() * scale);
	m_main->setPosition(origin.x + visibleSize.width / 2, origin.y + visibleSize.height / 2);
	this->addChild(m_main);


    m_loadingIndicator = LoadingIndicator::create(LOADING_INDICATOR_SIZE, Color3B(186, 186, 186));
    m_loadingIndicator->setPosition(origin.x + visibleSize.width / 2, origin.y + 76);
    this->addChild(m_loadingIndicator);

#if NS_DEBUG
	m_queuedTasksLabel = Label::createWithSystemFont("", DEFAULT_SYSTEM_FONT, 10);
	m_queuedTasksLabel->setAnchorPoint(Point::ANCHOR_TOP_LEFT);
	m_queuedTasksLabel->setPosition(Vec2(origin.x + 3, origin.y + visibleSize.height));
	m_queuedTasksLabel->setTextColor(Color4B::YELLOW);
	m_queuedTasksLabel->enableShadow(Color4B::BLACK, Size(1, -1));
	this->addChild(m_queuedTasksLabel);
#endif // NS_DEBUG

	this->startTasks();

	return true;
}

void SplashScene::update(float delta)
{
    this->performTaskInQueue();
    
    m_delayTimer.update(delta);
    if (m_delayTimer.passed())
        this->removeTask(TASK_WAITING_SPLASH);
    
    this->debugTaskQueue();

    // 所有任务执行完毕
    if(m_pendingTaskQueue.empty())
    {
        this->unscheduleUpdate();
		this->onAllTasksCompleted();
    }
}

void SplashScene::performTaskInQueue()
{
    auto it = m_pendingTaskQueue.begin();
    while(it != m_pendingTaskQueue.end())
    {
        switch (*it)
        {
            case TASK_INIT_GAME_CONFIGS:
                this->initGameConfigs();
                this->removeTask(*it);
				return;
            case TASK_LOAD_ASSETS:
                this->loadAssets();
                this->removeTask(*it);
				return;
            case TASK_VALIDATE_RECEIPT:
                this->validateReceipt();
                this->removeTask(*it);
				return;
            default:
                ++it;
                break;
        }
    }
}

void SplashScene::debugTaskQueue()
{
#if NS_DEBUG
	std::stringstream ss;
	ss << "Task Queue:\n";
	for (auto it = m_pendingTaskQueue.begin(); it != m_pendingTaskQueue.end(); ++it)
	{
		switch (*it)
		{
		case TASK_PENDING:
			ss << "TASK_PENDING";
			break;
		case TASK_INIT_GAME_CONFIGS:
			ss << "TASK_INIT_GAME_CONFIGS";
			break;
		case TASK_LOAD_ASSETS:
            ss << "TASK_LOAD_ASSETS";
            break;
        case TASK_ASSETS_LOADING:
            ss << "TASK_ASSETS_LOADING";
            break;
		case TASK_WAITING_SPLASH:
            ss << std::fixed << std::setprecision(2);
            ss << "TASK_WAITING_SPLASH (";
            ss << m_delayTimer.getRemainder();
            ss << "/";
            ss << m_delayTimer.getDuration();
            ss << "s)";
			break;
		case TASK_VALIDATE_RECEIPT:
			ss << "TASK_VALIDATE_RECEIPT";
			break;
        case TASK_AUTHENTICATING_LOCAL_PLAYER:
            ss << "TASK_AUTHENTICATING_LOCAL_PLAYER";
            break;
		default:
			ss << "TASK_UNKNOWN (" << (*it) << ")";
			break;
		}
		ss << "\n";
	}
	m_queuedTasksLabel->setString(ss.str());

#endif // NS_DEBUG
}

void SplashScene::showGameDataErrorDialog()
{
	MessageDialog* dialog = MessageDialog::create();
	dialog->setTitle(sLocaleMgr->getString("message_dialog_title_error"));
	dialog->setMessage(sLocaleMgr->getString("dlg_msg_gamedata_error"));
	dialog->addOkButton([this](Ref* sender) {
		Utils::exitApp();
	});
	dialog->show();
}

void SplashScene::initGameConfigs()
{
	// 初始化随机数种子
	std::srand((unsigned int)time(0));

	// 设置声音开关状态
	SoundMgr::instance()->setEnableBackgroundMusic(UserPreferences::instance()->isMusicEnabled());
	SoundMgr::instance()->setEnableSoundEffect(UserPreferences::instance()->isSoundEffectEnabled());

    // 初始化Firebase服务
	sFirebaseService->init();
    
    // 记录当前场景
    sAnalytics->setCurrentScreen("SplashScene");

	// 初始化时间服务
	sTimeService->init();
}

void SplashScene::onAuthFinished(AuthOperation op, AuthResponse const& response)
{
    if(response.error.code == GAMECENTER_ERROR_LOAD_DATA_FAILED)
    {
		MessageDialog* dialog = MessageDialog::create();
		dialog->setTitle(sLocaleMgr->getString("gamecenter_dlg_title_playerdata_error"));
		dialog->setMessage(sLocaleMgr->getString("gamecenter_dlg_msg_playerdata_error"));
		dialog->addOkButton([this](Ref* sender) {
			Utils::exitApp();
		});
		dialog->show();
    }
    else
    {
        if(response.state == AUTH_STATE_SUCCESS)
            sGameCenter->reportScores(LEADERBOARD_ID_TOTAL_KILLS);

        this->removeTask(TASK_AUTHENTICATING_LOCAL_PLAYER);
    }
}


void SplashScene::onReportScoreSuccess()
{
    
}

void SplashScene::onReportScoreFail(GameCenterError const& error)
{
    
}

void SplashScene::loadAssets()
{
	this->addTask(TASK_ASSETS_LOADING);
    sAssetsLoader->loadGeneral([this]() {
		this->onGeneralAssetsLoaded();
    });
}

void SplashScene::startFuncScene()
{
	auto scene = FuncScene::createScene();
	Director::getInstance()->replaceScene(TransitionCrossFade::create(0.5f, scene));
}

void SplashScene::startGameLoadScene()
{
	PlayerProfile profile = createDefaultPlayerProfile(DEFAULT_HERO_ID, "");
	auto scene = GameLoadScene::create(profile, false)->addToScene();
	Director::getInstance()->replaceScene(scene);
}

void SplashScene::validateProductIds()
{
    std::vector<std::string> productIds = sShopMgr->getProductIdList();
    sStore->validateProductIds(sShopMgr->getProductIdList());
}

void SplashScene::validateReceipt()
{
    sStore->validateReceipt();
}

void SplashScene::initPlayerData()
{
    LocalPlayer* localPlayer = sGameCenter->getLocalPlayer();
	bool needToSave = false;

    // 将免费英雄解锁
    auto tmplList = sShopMgr->getHeroTemplateList();
    for (auto const& tmpl: *tmplList)
    {
        if (tmpl.isFree() && !localPlayer->hasUnlockedHero(tmpl.id))
        {
            localPlayer->unlockHero(tmpl.id);
			needToSave = true;
        }
    }

    if(needToSave)
        localPlayer->saveDataAsync();
}

void SplashScene::onAllTasksCompleted()
{    
	sGameCenter->removeListener(this);

    // 初始化玩家数据
	this->initPlayerData();

	// 评分步骤：启动应用
	if (!sStoreReview->isProcessCompleted(REVIEW_PROCESS_LAUNCH_APP))
	{
		int32 count = sUserPreferences->getAppLaunchCount() + 1;
		if (count >= REVIEW_APP_LAUNCH_TIMES)
			sStoreReview->setProcessCompleted(REVIEW_PROCESS_LAUNCH_APP);
		sUserPreferences->setAppLaunchCount(count);
	}
    
    // 请求IDFA
    if (sAdManager->isNeedTrackingAuthorization())
        sAdManager->requestIDFA(nullptr);
    
	if (sGameCenter->getLocalPlayer()->isTrainee())
		this->startGameLoadScene();
	else
		this->startFuncScene();
}

void SplashScene::onGeneralAssetsLoaded()
{
	sGameCenter->addListener(this);
	sGameCenter->authLocalPlayer();
	this->addTask(TASK_AUTHENTICATING_LOCAL_PLAYER);

	sShopMgr->loadAsync([this](bool ret) {
		if (ret)
			this->onShopDataLoaded();
		else
			this->showGameDataErrorDialog();
	});
}

void SplashScene::onShopDataLoaded()
{
	this->validateProductIds();

	sObjectMgr->loadAsync([this](bool ret) {
		if (ret)
			this->onObjectDataLoaded();
		else
			this->showGameDataErrorDialog();
	});
}

void SplashScene::onObjectDataLoaded()
{
	sAssetsLoader->loadFunctional([this]() {
		this->removeTask(TASK_ASSETS_LOADING);
	});
}

void SplashScene::startTasks()
{
	m_delayTimer.setDuration(SPLASH_DURATION);
	this->addTask(TASK_WAITING_SPLASH);

	this->addTask(TASK_INIT_GAME_CONFIGS);
	this->addTask(TASK_LOAD_ASSETS);
#if NS_DEBUG
	this->addTask(TASK_VALIDATE_RECEIPT);
#endif

	this->scheduleUpdate();
}

void SplashScene::addTask(TaskFlag task)
{
    NS_ASSERT_LOG(std::find(m_pendingTaskQueue.begin(), m_pendingTaskQueue.end(), task) == std::end(m_pendingTaskQueue), "Task already exists in queue.");
    m_pendingTaskQueue.push_back(task);
    this->debugTaskQueue();
}

void SplashScene::removeTask(TaskFlag task)
{
    this->debugTaskQueue();
    m_pendingTaskQueue.erase(std::remove(m_pendingTaskQueue.begin(), m_pendingTaskQueue.end(), task), m_pendingTaskQueue.end());
}


NS_END

