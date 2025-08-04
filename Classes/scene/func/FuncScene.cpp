#include "FuncScene.h"

#include "common/utils/StringUtility.h"
#include "common/Machine.h"
#include "game/UserPreferences.h"
#include "game/LocaleMgr.h"
#include "game/ObjectMgr.h"
#include "debugopt/DebugOptionWicket.h"
#include "scene/SoundMgr.h"
#include "scene/AssetsLoader.h"
#include "scene/gui/ModalDialog.h"
#include "scene/GameLoadScene.h"
#include "scene/review/StoreReview.h"
#include "scene/Utils.h"
#include "option/OptionWicket.h"
#include "shop/ShopWicket.h"
#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
#include "game/GoogleServicesUtils.h"
#endif // CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
#include "RemoveAdsDialog.h"
#include "DailyRewardDialog.h"


using namespace cocos2d::ui;

NS_BEGIN


// Scene element frame name
#define FRAMENAME_FUNC_BTN_PLAY				"func_btn_play.png"
#define FRAMENAME_FUNC_FG_LOGO				"func_fg_logo.png"
#define FRAMENAME_FUNC_FG_DOGHOUSE			"func_fg_doghouse.png"
#define FRAMENAME_FUNC_FG_SIGNPOST			"func_fg_signpost.png"
#define FRAMENAME_FUNC_FG_SLEIGH			"func_fg_sleigh.png"
#define FRAMENAME_FUNC_TOTALKILLS_BG		"func_totalkills_bg.png"
#define FRAMENAME_FUNC_BTN_OPTION			"func_btn_option.png"
#define FRAMENAME_FUNC_BTN_SHARE			"func_btn_share.png"
#define FRAMENAME_FUNC_REMOVE_ADS			"func_btn_remove_ads.png"
#define FRAMENAME_FUNC_BTN_RANKING			"func_btn_ranking.png"
#define FRAMENAME_FUNC_GP_CONTROLLER        "func_gp_controller.png"
#define FRAMENAME_FUNC_GP_BTN_LEADERBOARD   "func_gp_btn_leaderboard.png"

// Snow effect configuration. Unit: seconds
#define SNOWEFFECT_DURATION_MIN					60.0f
#define SNOWEFFECT_DURATION_MAX					120.0f
#define SNOWEFFECT_INTERVAL_MIN					60.0f
#define SNOWEFFECT_INTERVAL_MAX					120.0f
#define SNOWEFFECT_DELAY_MIN					10.0f
#define SNOWEFFECT_DELAY_MAX					60.0f

// Bug sleeping animation configuration
#define BUG_SLEEP_FRAMES					10
#define BUG_SLEEP_DEFAULT_FRAME_INDEX		9
#define BUG_SLEEP_FRAMENAME_FORMAT			"func_bug_sleep%.2d.png"


#define WICKET_NAME_SHOP						"SHOP_WICKET"
#define SCHEDULE_KEY_SHOW_OPTION_WICKET			"ShowOptionWicket"

Scene* FuncScene::createScene()
{
	auto scene = FuncScene::create()->addToScene();
	return scene;
}

FuncScene* FuncScene::create(RewardedAdConfig const& adConfig, bool isEnableInterstitialAd)
{
	FuncScene *pRet = new(std::nothrow) FuncScene();
	if (pRet && pRet->init(adConfig, isEnableInterstitialAd))
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

FuncScene* FuncScene::create()
{
	FuncScene *pRet = new(std::nothrow) FuncScene();
	if (pRet && pRet->init(RewardedAdConfig(), false))
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

bool FuncScene::init(RewardedAdConfig const& adConfig, bool isEnableInterstitialAd)
{
	if (!BaseScene::init())
	{
		return false;
	}

    sAnalytics->setCurrentScreen("FuncScene");

	m_rewardedAdConfig = adConfig;
	m_isEnableInterstitialAd = isEnableInterstitialAd;

    sGameCenter->setShowAuthDialog(true);
    sGameCenter->addListener(this);
    sStore->addListener(this);
    sStore->registerTransactionObserver();
    sShare->setListener(this);
	sTimeService->addListener(this);
	sAdManager->addRewardedAdListener(RewardedAdConfig::AD_TYPE_BATTLE_OUTCOME, this);
	sAdManager->addRewardedAdListener(RewardedAdConfig::AD_TYPE_DAILY_REWARD, this);
	sAdManager->addInterstitialAdListener(this);

	// Get current hero information
	uint32 heroId = sGameCenter->getLocalPlayer()->getHeroId();
	if (heroId == HERO_NONE)
		m_currHeroTmpl = sShopMgr->getHeroTemplateByHeroId(DEFAULT_HERO_ID);
	else
		m_currHeroTmpl = sShopMgr->getHeroTemplateByHeroId(heroId);

	auto keyboardListener = EventListenerKeyboard::create();
	keyboardListener->onKeyReleased = CC_CALLBACK_2(FuncScene::onKeyReleased, this);
	this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(keyboardListener, this);

	auto visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();

	// Is the device a narrow screen
	m_isNarrowScreen = Machine::instance()->isNarrowScreen();

	// Set the screen safe area
	Rect safeArea = Machine::instance()->getSafeAreaRect();
	m_safeInsetLeft = Utils::getSafeInsetLeft(safeArea);
	m_safeInsetRight = Utils::getSafeInsetRight(safeArea);

	// Set up the finger tap process
	this->setupFingerTapProcess();

	// Background
	m_background = Sprite::create();
	m_background->setAnchorPoint(Point::ANCHOR_MIDDLE);
	m_background->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y + visibleSize.height / 2));
	if (m_isNarrowScreen)
		m_background->setTexture(FUNC_BG_NARROW_ATLAS);
	else
		m_background->setTexture(FUNC_BG_ATLAS);
	this->addChild(m_background);

	// Background snow effect
	m_snowEffectBg = SnowEffect::create();
	this->addChild(m_snowEffectBg);

	// Name input box
	m_nameInputBox = FuncNameInputBox::create();
	m_nameInputBox->setAnchorPoint(Point::ANCHOR_MIDDLE);
	m_nameInputBox->setEditEventListener(CC_CALLBACK_2(FuncScene::nameInputBoxEditCallback, this));
	if(m_isNarrowScreen)
		m_nameInputBox->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y + 115));
	else
		m_nameInputBox->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y + 99));
	this->addChild(m_nameInputBox);
    this->updateNameInputBox();

	// Hero catwalk
	m_heroCatwalk = HeroCatwalk::create();
	m_heroCatwalk->setAnchorPoint(Point::ANCHOR_MIDDLE_BOTTOM);
	m_heroCatwalk->setSelectEventListener(CC_CALLBACK_2(FuncScene::heroCatwalkSelectCallback, this));
    m_heroCatwalk->setBeginScrollEventListener(CC_CALLBACK_1(FuncScene::heroCatwalkBeginScrollCallback, this));
	m_heroCatwalk->setupHeroes();
	m_heroCatwalk->setSelectedHero(m_currHeroTmpl->id);
	if (m_isNarrowScreen)
		m_heroCatwalk->setPosition(origin.x + visibleSize.width / 2, origin.y + 139);
	else
		m_heroCatwalk->setPosition(origin.x + visibleSize.width / 2, origin.y + 120);
	this->addChild(m_heroCatwalk);

	// Hero statistic layer
	m_heroStatLayer = HeroStatLayer::create(visibleSize);
	m_heroStatLayer->setPosition(origin);
	m_heroStatLayer->setPanelCenteredSpacing(146);
	m_heroStatLayer->setStatUpgradedEventListener(CC_CALLBACK_2(FuncScene::heroStatUpgradedCallback, this));
	m_heroStatLayer->setVisibleStateChangedEventListener(CC_CALLBACK_2(FuncScene::heroStatVisibleStateChangedCallback, this));
	m_heroStatLayer->setUpgradePanelScrollEventListener(CC_CALLBACK_2(FuncScene::heroStatUpgradePanelScrollCallback, this));
	if (m_isNarrowScreen)
		m_heroStatLayer->setPanelBottom(141);
	else
		m_heroStatLayer->setPanelBottom(118);
	this->addChild(m_heroStatLayer);

	// Hero statistic button
	m_heroStatBtn = HeroStatButton::create();
	m_heroStatBtn->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
	m_heroStatBtn->setClickEventListener(CC_CALLBACK_1(FuncScene::buttonHeroStatCallback, this));
	if(!sGameCenter->getLocalPlayer()->isSuggestionAccepted(SUGGESTION_BACK_HOME))
		m_heroStatBtn->setVisible(false);
	if (m_isNarrowScreen)
		m_heroStatBtn->setPosition(origin.x + visibleSize.width / 2 + 17, origin.y + 141);
	else
		m_heroStatBtn->setPosition(origin.x + visibleSize.width / 2 + 17, origin.y + 122);
	this->addChild(m_heroStatBtn);

	// Foreground
	this->initForeground();

	// Foreground snow effect
	m_snowEffectFg = SnowEffect::create();
	this->addChild(m_snowEffectFg);

	// Property bar
	m_propertyBar = PropertyBar::create();
	m_propertyBar->setAnchorPoint(Point::ANCHOR_TOP_LEFT);
	m_propertyBar->setClickEventListener(CC_CALLBACK_1(FuncScene::propertyBarClickCallback, this));
	m_propertyBar->setWatchAdReminderDelay(1.0f);
	if (m_isNarrowScreen)
	{
		m_propertyBar->setScale(0.9f);
		m_propertyBar->setPosition(Vec2(origin.x + 7, origin.y + visibleSize.height - 24));
	}
	else
		m_propertyBar->setPosition(Vec2(origin.x + (m_safeInsetLeft > 0 ? m_safeInsetLeft : 6), origin.y + visibleSize.height - 11));
	this->addChild(m_propertyBar);

	// Function button
	m_funcBtn = FuncButton::create();
	m_funcBtn->setAnchorPoint(Point::ANCHOR_MIDDLE);
	m_funcBtn->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y + 45));
	m_funcBtn->setPlayButtonClickEventListener(CC_CALLBACK_1(FuncScene::buttonPlayCallback, this));
	m_funcBtn->setBuyButtonClickEventListener(CC_CALLBACK_2(FuncScene::buttonBuyCallback, this));
	m_funcBtn->setHeroTemplate(m_currHeroTmpl);
    this->addChild(m_funcBtn);

	// Option button
	m_optionBtn = Button::create(FRAMENAME_FUNC_BTN_OPTION, "", "", Widget::TextureResType::PLIST);
	m_optionBtn->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
	m_optionBtn->addClickEventListener(CC_CALLBACK_1(FuncScene::buttonOptionCallback, this));
	if (m_isNarrowScreen)
		m_optionBtn->setPosition(Vec2(origin.x + (m_safeInsetLeft > 0 ? m_safeInsetLeft : 8), origin.y + 21));
	else
		m_optionBtn->setPosition(Vec2(origin.x + (m_safeInsetLeft > 0 ? m_safeInsetLeft : 18), origin.y + 18));
#if USE_DEBUG_OPTION
	m_optionBtn->addTouchEventListener(CC_CALLBACK_2(FuncScene::buttonOptionTouchCallback, this));
#endif // USE_DEBUG_OPTION
	this->addChild(m_optionBtn);

	// Ranking button
#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
    m_rankingBtn = Button::create(FRAMENAME_FUNC_GP_BTN_LEADERBOARD, "", "", Widget::TextureResType::PLIST);
#else
    m_rankingBtn = Button::create(FRAMENAME_FUNC_BTN_RANKING, "", "", Widget::TextureResType::PLIST);
#endif
	m_rankingBtn->setAnchorPoint(Point::ANCHOR_BOTTOM_RIGHT);
	m_rankingBtn->addClickEventListener(CC_CALLBACK_1(FuncScene::buttonRankingCallback, this));
	if (m_isNarrowScreen)
		m_rankingBtn->setPosition(Vec2(origin.x + visibleSize.width - (m_safeInsetRight > 0 ? m_safeInsetRight : 8), origin.y + 21));
	else
		m_rankingBtn->setPosition(Vec2(origin.x + visibleSize.width - (m_safeInsetRight > 0 ? m_safeInsetRight : 18), origin.y + 18));
	this->addChild(m_rankingBtn);
#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
	m_gpCntlrSp = Sprite::createWithSpriteFrameName(FRAMENAME_FUNC_GP_CONTROLLER);
	m_gpCntlrSp->setAnchorPoint(Point::ANCHOR_MIDDLE_RIGHT);
	if (m_isNarrowScreen)
	{
		m_gpCntlrSp->setScale(0.9f);
		m_gpCntlrSp->setPosition(Vec2(m_rankingBtn->getBoundingBox().getMinX() - 1, m_rankingBtn->getBoundingBox().getMidY()));
	}
	else
		m_gpCntlrSp->setPosition(Vec2(m_rankingBtn->getBoundingBox().getMinX() + 1, m_rankingBtn->getBoundingBox().getMidY()));
	this->addChild(m_gpCntlrSp);
#endif

	// Remove ads button
	m_removeAdsBtn = this->createShortcutButton(sLocaleMgr->getString("func_btn_remove_ads"), FRAMENAME_FUNC_REMOVE_ADS);
	m_removeAdsBtn->setAnchorPoint(Point::ANCHOR_TOP_RIGHT);
	m_removeAdsBtn->addClickEventListener(CC_CALLBACK_1(FuncScene::buttonRemoveAdsCallback, this));
	this->addChild(m_removeAdsBtn);
	if(sGameCenter->getLocalPlayer()->isAdsRemoved())
	    m_removeAdsBtn->setVisible(false);

	// Share button
	m_shareBtn = this->createShortcutButton(sLocaleMgr->getString("func_btn_share"), FRAMENAME_FUNC_BTN_SHARE);
	m_shareBtn->setAnchorPoint(Point::ANCHOR_TOP_RIGHT);
	m_shareBtn->addClickEventListener(CC_CALLBACK_1(FuncScene::buttonShareCallback, this));
	this->addChild(m_shareBtn);

	// Finger tap
	if (m_fingerTapProcess != FINGERTAP_PROCESS_NONE)
	{
		m_fingerTap = FingerTap::create();
		m_fingerTap->setAnchorPoint(Point::ANCHOR_TOP_LEFT);
		this->addChild(m_fingerTap);
	}

	this->layoutShortcutButtons();
	this->updateTotalKills();

	// Play background music
	sSoundMgr->play(SOUND_FUNC_MUSIC, true, 0.0f);
    sSoundMgr->fadeInVolume(SOUND_FUNC_MUSIC, 1.0f);

	// Schedule snow effect
	this->scheduleSnowEffect();

	return true;
}

Scene* FuncScene::addToScene()
{
	auto scene = Scene::create();
	scene->addChild(this);
	return scene;
}

void FuncScene::onEnterTransitionDidFinish()
{
	BaseScene::onEnterTransitionDidFinish();

	// Watch the ad
	bool isWatchAd;
	if (m_rewardedAdConfig.adType != RewardedAdConfig::AD_TYPE_NONE)
		isWatchAd = this->requestRewardedVideo(m_rewardedAdConfig);
	else
		isWatchAd = this->showInterstitialAdIfNeeded();
	if(!isWatchAd)
		this->updateUIAfterAdHidden();
}

void FuncScene::onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event)
{
	switch (keyCode)
	{
	case cocos2d::EventKeyboard::KeyCode::KEY_BACK:
	    if(m_heroStatLayer->isShown())
            m_heroStatLayer->hide();
	    else
		    Utils::exitApp();
		break;
	default:
		break;
	}
}

FuncScene::FuncScene():
	m_currHeroTmpl(nullptr),
    m_isShowingLeaderboard(false),
	m_isEnableInterstitialAd(false),
	m_isNarrowScreen(false),
	m_safeInsetLeft(0),
	m_safeInsetRight(0),
	m_fingerTapProcess(FINGERTAP_PROCESS_NONE),
	m_totalKillsLabel(nullptr),
    m_totalKillsBg(nullptr),
	m_fgContainer(nullptr),
	m_logoSp(nullptr),
	m_background(nullptr),
	m_heroCatwalk(nullptr),
	m_funcBtn(nullptr),
	m_nameInputBox(nullptr),
	m_optionBtn(nullptr),
	m_gpCntlrSp(nullptr),
	m_rankingBtn(nullptr),
	m_shareBtn(nullptr),
	m_removeAdsBtn(nullptr),
	m_propertyBar(nullptr),
    m_loadingView(nullptr),
	m_heroStatBtn(nullptr),
	m_heroStatLayer(nullptr),
	m_fingerTap(nullptr),
	m_snowEffectBg(nullptr),
	m_snowEffectFg(nullptr)
{
}


FuncScene::~FuncScene()
{
    sGameCenter->removeListener(this);
    sStore->removeListener(this);
    sShare->removeListener();
	sTimeService->removeListener(this);
	sAdManager->removeRewardedAdListener(this);
	sAdManager->removeInterstitialAdListener(this);
    
	m_currHeroTmpl = nullptr;
	m_totalKillsLabel = nullptr;
    m_totalKillsBg = nullptr;
	m_fgContainer = nullptr;
	m_logoSp = nullptr;
	m_background = nullptr;
	m_heroCatwalk = nullptr;
	m_funcBtn = nullptr;
    m_nameInputBox = nullptr;
	m_optionBtn = nullptr;
	m_gpCntlrSp = nullptr;
	m_rankingBtn = nullptr;
	m_shareBtn = nullptr;
	m_removeAdsBtn = nullptr;
	m_propertyBar = nullptr;
    m_loadingView = nullptr;
	m_heroStatBtn = nullptr;
	m_heroStatLayer = nullptr;
	m_fingerTap = nullptr;
	m_snowEffectBg = nullptr;
	m_snowEffectFg = nullptr;
}

void FuncScene::initForeground()
{
	// Foreground container
	m_fgContainer = Node::create();
	m_fgContainer->setIgnoreAnchorPointForPosition(false);
	m_fgContainer->setPosition(m_background->getPosition());
	m_fgContainer->setContentSize(m_background->getContentSize());
	m_fgContainer->setAnchorPoint(m_background->getAnchorPoint());
	this->addChild(m_fgContainer);

	// Static elements
	m_logoSp = Sprite::createWithSpriteFrameName(FRAMENAME_FUNC_FG_LOGO);
	m_logoSp->setAnchorPoint(Point::ANCHOR_MIDDLE_TOP);
	if (m_isNarrowScreen)
	{
		m_logoSp->setScale(0.9f);
		m_logoSp->setPosition(m_fgContainer->getContentSize().width / 2, m_fgContainer->getContentSize().height - 20);
	}
	else
		m_logoSp->setPosition(m_fgContainer->getContentSize().width / 2, m_fgContainer->getContentSize().height);
	m_fgContainer->addChild(m_logoSp);
	m_totalKillsBg = Sprite::createWithSpriteFrameName(FRAMENAME_FUNC_TOTALKILLS_BG);
	m_totalKillsBg->setAnchorPoint(Point::ANCHOR_MIDDLE);
	if (m_isNarrowScreen)
	{
		m_totalKillsBg->setScale(0.9f);
		m_totalKillsBg->setPosition(m_logoSp->getBoundingBox().getMaxX() - 26, m_logoSp->getBoundingBox().getMinY() + 8);
	}
	else
		m_totalKillsBg->setPosition(m_logoSp->getBoundingBox().getMaxX() - 33, m_logoSp->getBoundingBox().getMinY() + 8);
	m_fgContainer->addChild(m_totalKillsBg);
	Sprite* doghouse = Sprite::createWithSpriteFrameName(FRAMENAME_FUNC_FG_DOGHOUSE);
	doghouse->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
	if (m_isNarrowScreen)
		doghouse->setPosition(32, 79);
	else
		doghouse->setPosition(99, 66);
	m_fgContainer->addChild(doghouse);
	if (!m_isNarrowScreen)
	{
		Sprite* signpostFg = Sprite::createWithSpriteFrameName(FRAMENAME_FUNC_FG_SIGNPOST);
		signpostFg->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
		signpostFg->setPosition(38, 70);
		m_fgContainer->addChild(signpostFg);
	}
	Sprite*sleighFg = Sprite::createWithSpriteFrameName(FRAMENAME_FUNC_FG_SLEIGH);
	sleighFg->setAnchorPoint(Point::ANCHOR_BOTTOM_RIGHT);
	if (m_isNarrowScreen)
		sleighFg->setPosition(m_fgContainer->getContentSize().width - 38, 79);
	else
		sleighFg->setPosition(m_fgContainer->getContentSize().width - 131, 79);
	m_fgContainer->addChild(sleighFg);

	// Bug sleeping animation
	Vector<SpriteFrame*> frameSeq;
	for (int32 i = 0; i < BUG_SLEEP_FRAMES; ++i)
	{
		std::string framename = StringUtils::format(BUG_SLEEP_FRAMENAME_FORMAT, i);
		SpriteFrame* sf = SpriteFrameCache::getInstance()->getSpriteFrameByName(framename);
		NS_ASSERT(sf != nullptr);
		frameSeq.pushBack(sf);
	}
	Animation* animation = Animation::createWithSpriteFrames(frameSeq, ANIM_NORMAL_FRAME_DELAY);
	Animate* animate = Animate::create(animation);
	Sprite* bugSp = Sprite::createWithSpriteFrame(frameSeq.at(BUG_SLEEP_DEFAULT_FRAME_INDEX));
	bugSp->setAnchorPoint(Point::ANCHOR_MIDDLE_BOTTOM);
	if(m_isNarrowScreen)
		bugSp->setPosition(Vec2(m_fgContainer->getContentSize().width - 221, 10.0f));
	else
		bugSp->setPosition(Vec2(m_fgContainer->getContentSize().width - 314, 10.0f));
	m_fgContainer->addChild(bugSp);
	Sequence* seq = Sequence::create(DelayTime::create(1.0f), animate, nullptr);
	RepeatForever* repeat = RepeatForever::create(seq);
	bugSp->runAction(repeat);

	// Total kills
	m_totalKillsLabel = Label::createWithBMFont(BMFONT_PIXCELSTYLE, "");
	m_totalKillsLabel->setColor(Color3B(218, 86, 64));
	m_totalKillsLabel->setAdditionalKerning(4.0f);
	m_totalKillsLabel->setAnchorPoint(Point::ANCHOR_MIDDLE_BOTTOM);
	m_totalKillsLabel->setPosition(Vec2(93, 13));
	m_totalKillsBg->addChild(m_totalKillsLabel);

	// Gift box
	m_giftBox = GiftBox::create();
	m_giftBox->setAnchorPoint(Point::ANCHOR_MIDDLE_BOTTOM);
	if (m_isNarrowScreen)
		m_giftBox->setPosition(Vec2(m_fgContainer->getContentSize().width - 184, 78.0f));
	else
		m_giftBox->setPosition(Vec2(m_fgContainer->getContentSize().width - 282, 78.0f));
	m_giftBox->setClickEventListener(CC_CALLBACK_1(FuncScene::giftBoxClickCallback, this));
	m_fgContainer->addChild(m_giftBox);
	if (sGameCenter->getLocalPlayer()->isSuggestionAccepted(SUGGESTION_BACK_HOME))
		m_giftBox->setVisible(true);
	else
		m_giftBox->setVisible(false);
}

ui::Button* FuncScene::createShortcutButton(std::string const& title, std::string const& normalImage)
{
	Button* btn = Button::create(normalImage, "", "", Widget::TextureResType::PLIST);
	Label* titleLabel = Label::createWithSystemFont(title, DEFAULT_SYSTEM_FONT, 8);
	btn->setTitleLabel(titleLabel);
	titleLabel->setAnchorPoint(Point::ANCHOR_MIDDLE_BOTTOM);
	titleLabel->setTextColor(Color4B(94, 100, 124, 255));
	Utils::enableBoldForLabel(titleLabel);
	titleLabel->setPosition(btn->getContentSize().width / 2, 5.0f);

	return btn;
}

void FuncScene::layoutShortcutButtons()
{
	auto visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();

	if (m_isNarrowScreen)
	{
		float nextX = origin.x + visibleSize.width - (m_safeInsetRight > 0 ? m_safeInsetRight : 2);
		m_removeAdsBtn->setPosition(Vec2(nextX, origin.y + visibleSize.height - 23));
		m_removeAdsBtn->setScale(0.9f);
		if (m_removeAdsBtn->isVisible())
			nextX = m_removeAdsBtn->getBoundingBox().getMinX();

		m_shareBtn->setPosition(Vec2(nextX, m_removeAdsBtn->getBoundingBox().getMaxY()));
		m_shareBtn->setScale(m_removeAdsBtn->getScale());
	}
	else
	{
		float nextX = origin.x + visibleSize.width - (m_safeInsetRight > 0 ? m_safeInsetRight : 8);
		m_removeAdsBtn->setPosition(Vec2(nextX, origin.y + visibleSize.height - 7));
		if (m_removeAdsBtn->isVisible())
			nextX = m_removeAdsBtn->getBoundingBox().getMinX();

		m_shareBtn->setPosition(Vec2(nextX, m_removeAdsBtn->getBoundingBox().getMaxY()));
	}
}

void FuncScene::setFingerTapProcessCompleted(FingerTapProcess progress)
{
	switch (progress)
	{
	case FINGERTAP_PROCESS_CLAIM_REWARD:
		this->saveSuggestionAccepted(SUGGESTION_CLAIM_REWARD, true);
		break;
	case FINGERTAP_PROCESS_UPGRADE_STATS:
		this->saveSuggestionAccepted(SUGGESTION_UPGRADE_STATS, true);
		break;
	case FINGERTAP_PROCESS_PLAY:
		break;
	default:
		break;
	}

	FingerTapProcess prevProgress = m_fingerTapProcess;
	this->setupFingerTapProcess();
	if(prevProgress != m_fingerTapProcess)
	{
		this->startFingerTap(false);
		if(m_fingerTapProcess == FINGERTAP_PROCESS_NONE)
			sAnalytics->logTutorialComplete();
	}
}

void FuncScene::setupFingerTapProcess()
{
	auto localPlayer = sGameCenter->getLocalPlayer();
	if (!localPlayer->isSuggestionAccepted(SUGGESTION_BACK_HOME))
	{
		m_fingerTapProcess = FINGERTAP_PROCESS_PLAY;
		return;
	}

	if (!localPlayer->isSuggestionAccepted(SUGGESTION_CLAIM_REWARD)
		&& this->getDailyRewardState() == DAILY_REWARD_STATE_READY)
	{
		m_fingerTapProcess = FINGERTAP_PROCESS_CLAIM_REWARD;
	}
	else if (!localPlayer->isSuggestionAccepted(SUGGESTION_UPGRADE_STATS))
		m_fingerTapProcess = FINGERTAP_PROCESS_UPGRADE_STATS;
	else
		m_fingerTapProcess = FINGERTAP_PROCESS_NONE;
}

void FuncScene::startFingerTap(bool isDelayed)
{
	switch (m_fingerTapProcess)
	{
	case FINGERTAP_PROCESS_CLAIM_REWARD:
		this->applyClaimRewardTips(isDelayed);
		break;
	case FINGERTAP_PROCESS_UPGRADE_STATS:
		this->applyUpgradeStatsSuggestion(isDelayed);
		break;
	case FINGERTAP_PROCESS_PLAY:
		this->applyPlayFingerTap(isDelayed);
		break;
	default: // FINGER_TAP_NONE
		if (m_fingerTap && m_fingerTap->isShown())
			m_fingerTap->hide();
		break;
	}
}

void FuncScene::applyUpgradeStatsTips(bool isDelayed)
{
	if (!m_heroStatBtn->isVisible())
		return;

	if (this->recommendUpgradeType() != StatUpgradeType::STAT_UPGRADE_NONE)
	{
		m_heroStatBtn->showTips(HeroStatButton::TIPS_UPGRADE_STATS, isDelayed ? 0.5f : 0.f);
		m_fingerTap->show(Vec2(m_heroStatBtn->getBoundingBox().getMidX(), m_heroStatBtn->getBoundingBox().getMidY()), isDelayed ? 0.8f : 0.f);
	}
}

void FuncScene::applyClaimRewardTips(bool isDelayed)
{
	if (!m_giftBox->isVisible())
		return;

	Point screenPos = m_giftBox->convertToWorldSpace(Vec2(m_giftBox->getContentSize().width / 2, m_giftBox->getContentSize().height / 2));
	m_fingerTap->show(this->convertToNodeSpace(screenPos), isDelayed ? 0.8f : 0.f);
	m_giftBox->showTips(isDelayed ? 0.5f : 0.f);
}

void FuncScene::applyFirstWatchAdTips()
{
	auto localPlayer = sGameCenter->getLocalPlayer();
	if (!localPlayer->isSuggestionAccepted(SUGGESTION_BACK_HOME))
		return;

	if (m_fingerTapProcess != FINGERTAP_PROCESS_NONE)
		return;

	if (!localPlayer->isSuggestionAccepted(SUGGESTION_WATCH_AD_FIRST))
	{
		if(m_propertyBar->getShownTipsType() != PropertyBar::TIPS_WATCH_AD_FIRST)
			m_propertyBar->showFirstWatchAdTips(1.0f);
	}
}

void FuncScene::applyPlayFingerTap(bool isDelayed)
{
	if (m_fingerTapProcess != FINGERTAP_PROCESS_PLAY)
		return;

	if (m_funcBtn->canPlay())
	{
		Point screenPos = m_funcBtn->convertToWorldSpace(Vec2(m_funcBtn->getContentSize().width / 2, m_funcBtn->getContentSize().height / 2));
		m_fingerTap->show(screenPos, isDelayed ? 0.5f : 0.f);
	}
}

void FuncScene::applyUpgradeStatsSuggestion(bool isDelayed)
{
	if (m_fingerTapProcess != FINGERTAP_PROCESS_UPGRADE_STATS)
		return;

	if (m_heroStatLayer->isShown())
	{
		this->updateRecommendedUpgradeTypes();
		if (m_heroStatLayer->isAnyOfUpgradeTypesRecommended())
			m_heroStatLayer->updateUpgradePanel();

		Point pos;
		if (this->getRecommendedUpgradeButtonPosition(pos))
			m_fingerTap->show(pos, isDelayed? 0.5f : 0.f);
	}
	else
		this->applyUpgradeStatsTips(isDelayed);
}

void FuncScene::updateRecommendedUpgradeTypes()
{
	m_heroStatLayer->resetRecommendedUpgradeTypes();
	StatUpgradeType upgradeType = this->recommendUpgradeType();
	if (upgradeType != StatUpgradeType::STAT_UPGRADE_NONE)
		m_heroStatLayer->setRecommendedUpgradeType(upgradeType);
}

StatUpgradeType FuncScene::recommendUpgradeType()
{
	if (m_currHeroTmpl->isFree())
	{
		LocalPlayer* localPlayer = sGameCenter->getLocalPlayer();
		uint8 maxStage = STAT_STAGE_MIN;
		for (int32 type = 0; type < MAX_STAT_TYPES; ++type)
		{
			maxStage = std::max(maxStage, localPlayer->getStatStage(m_currHeroTmpl->id, (StatType)type));
		}
		if (maxStage == STAT_STAGE_MIN)
		{
			int32 property = localPlayer->getProperty();
			int32 price = m_currHeroTmpl->getNextStageUpgradePrice(STAT_STAGE_MIN, STAT_MAX_HEALTH);
			if (price > 0)
			{
				if (property >= price)
				{
					return StatUpgradeType::STAT_UPGRADE_HEALTH;
				}
			}
		}
	}

	return StatUpgradeType::STAT_UPGRADE_NONE;
}

bool FuncScene::getRecommendedUpgradeButtonPosition(Vec2& pos)
{
	StatUpgradeType upgradeType = this->recommendUpgradeType();
	if (upgradeType != StatUpgradeType::STAT_UPGRADE_NONE)
	{
		ui::Button* btn = m_heroStatLayer->getUpgradeButton(upgradeType);
		Point screenPos = btn->convertToWorldSpace(Vec2(btn->getContentSize().width * btn->getAnchorPoint().x, btn->getContentSize().height * btn->getAnchorPoint().y));
		pos = this->convertToNodeSpace(screenPos);
		return true;
	}

	return false;
}

void FuncScene::saveSuggestionAccepted(uint32 suggestion, bool accepted)
{
	auto localPlayer = sGameCenter->getLocalPlayer();
	if (accepted)
	{
		if (!localPlayer->isSuggestionAccepted(suggestion))
		{
			localPlayer->setSuggestionAccepted(suggestion, true);
			localPlayer->saveDataAsync();
		}
	}
	else
	{
		if (localPlayer->isSuggestionAccepted(suggestion))
		{
			localPlayer->setSuggestionAccepted(suggestion, false);
			localPlayer->saveDataAsync();
		}
	}
}

void FuncScene::updateGiftBoxState()
{
	if (!m_giftBox->isVisible())
		return;

	m_giftBox->setColor(Color3B::WHITE);
	switch (this->getDailyRewardState())
	{
	case DAILY_REWARD_STATE_READY:
		if (m_fingerTapProcess == FINGERTAP_PROCESS_NONE)
			m_giftBox->showTips(0.5f);
		m_giftBox->setGiftState(GiftBox::GIFT_STATE_READY);
		break;
	case DAILY_REWARD_STATE_CLAIMED:
		m_giftBox->setGiftState(GiftBox::GIFT_STATE_CLAIMED);
		break;
	default:
		m_giftBox->setColor(Color3B::GRAY);
		m_giftBox->setGiftState(GiftBox::GIFT_STATE_NONE);
		break;
	}
}

void FuncScene::giveDailyReward(DailyRewardType rewardType, int32 value, bool isDouble)
{
	LocalPlayer* localPlayer = sGameCenter->getLocalPlayer();
	switch (rewardType)
	{
	case DAILY_REWARD_TYPE_HERO:
	{
		HeroID heroId = (HeroID)value;
		NS_ASSERT(heroId != HERO_NONE);
		m_currHeroTmpl = sShopMgr->getHeroTemplateByHeroId(heroId);
		this->resetUIForSelectHero();
		m_heroCatwalk->setSelectedHero(m_currHeroTmpl->id, true);
		this->updateUIForSelectedHero();
		this->unlockHero(m_currHeroTmpl->id);
		break;
	}
	case DAILY_REWARD_TYPE_GOLD:
	{
		int32 amount = value;
		if (isDouble)
			amount *= 2;
		localPlayer->addMoney(amount);
		m_propertyBar->updateAmount(true);
		break;
	}
	default:
		NS_ASSERT_LOG(false, "Unsupported daily reward type.");
		break;
	}

	NS_ASSERT(sTimeService->getTimeState() == TimeService::TIME_SYNCED);
	uint32 claimTime = (uint32)(sTimeService->getCurrentTimeMillis() / 1000);
	localPlayer->setDailyRewardClaimTime(claimTime);
	localPlayer->increaseDailyRewardDays();
	localPlayer->saveDataAsync();
}

void FuncScene::giveOutcomeReward(int32 amount)
{
	LocalPlayer* localPlayer = sGameCenter->getLocalPlayer();
	localPlayer->addMoney(amount);
	localPlayer->saveDataAsync();
}

FuncScene::DailyRewardState FuncScene::getDailyRewardState() const
{
	if (sTimeService->getTimeState() == TimeService::TIME_SYNCED)
	{
		LocalPlayer* localPlayer = sGameCenter->getLocalPlayer();
		time_t claimTime = (time_t)localPlayer->getDailyRewardClaimTime();
		time_t nowTime = (time_t)(sTimeService->getCurrentTimeMillis() / 1000);

		using namespace time_util;
		tm nowTm;
		tm claimTm;
		localtime_r(&nowTime, &nowTm);
		localtime_r(&claimTime, &claimTm);

		if (nowTm.tm_year >= claimTm.tm_year)
		{
			if (nowTm.tm_year > claimTm.tm_year || nowTm.tm_yday > claimTm.tm_yday)
				return DAILY_REWARD_STATE_READY;
		}
		return DAILY_REWARD_STATE_CLAIMED;
	}

	return DAILY_REWARD_STATE_UNKNOWN;
}

bool FuncScene::requestRewardedVideo(RewardedAdConfig const& adConfig)
{
	if (sAdManager->isRewardedVideoInProgress())
		return false;

	if (sAdManager->requestShowRewardedVideo(adConfig))
	{
		this->showLoadingView();
		return true;
	}

	return false;
}

bool FuncScene::showInterstitialAdIfNeeded()
{
	if(!m_isEnableInterstitialAd)
		return false;

	if (!sGameCenter->getLocalPlayer()->isAdsRemoved() && sAdManager->isInterstitialAdLoaded())
	{
		if(sAdManager->showInterstitialAd())
			return true;
	}
	return false;
}

void FuncScene::updateUIAfterAdHidden()
{
	m_propertyBar->updateAmount(true, 0.3f);
	this->startFingerTap();
	this->updateGiftBoxState();

	auto localPlayer = sGameCenter->getLocalPlayer();
	if (localPlayer->isSuggestionAccepted(SUGGESTION_WATCH_AD_FIRST)
		&& !localPlayer->isSuggestionAccepted(SUGGESTION_WATCH_AD_REMINDED))
	{
		m_propertyBar->setWatchAdReminderEnabled(true);
	}
	this->applyFirstWatchAdTips();

	sStoreReview->requestReview();
}

void FuncScene::updateTotalKills()
{
    int32 totalKills = sGameCenter->getLocalPlayer()->getScoreValue(LEADERBOARD_ID_TOTAL_KILLS);
    if (totalKills <= 0)
    {
		m_shareBtn->setVisible(false);
        m_totalKillsBg->setVisible(false);
        m_totalKillsLabel->setVisible(false);
    }
    else
    {
		m_shareBtn->setVisible(true);
        m_totalKillsBg->setVisible(true);
        m_totalKillsLabel->setVisible(true);
        
        if (totalKills > 999)
            m_totalKillsLabel->setScale(0.5f);
        else if (totalKills > 99)
            m_totalKillsLabel->setScale(0.6f);
        else if (totalKills > 9)
            m_totalKillsLabel->setScale(0.8f);
        else
            m_totalKillsLabel->setScale(1.0f);
        
        m_totalKillsLabel->setString(StringUtils::format("%d", totalKills));
    }

}

void FuncScene::startGameLoadScene()
{
	PlayerProfile profile = createDefaultPlayerProfile(m_currHeroTmpl->id, m_nameInputBox->getText());
	auto scene = GameLoadScene::create(profile, false)->addToScene();
	Director::getInstance()->replaceScene(scene);
}

void FuncScene::showShopWicket(bool enableWatchAdHintIfNeeded)
{
	ShopWicket* wicket = ShopWicket::create(this);
	wicket->setName(WICKET_NAME_SHOP);
	wicket->setGoldDeliveryPoint(Vec2(m_propertyBar->getBoundingBox().getMidX(), m_propertyBar->getBoundingBox().getMidY()));
	wicket->setShopItemStateChangedEventListener([this](Ref* sender, ShopItemState state) {
		m_propertyBar->updateAmount(true);
		this->applyUpgradeStatsSuggestion();
	});
	wicket->setResultListener([this](Wicket::ResultCode resultCode, ValueMapIntKey const& data){
		auto localPlayer = sGameCenter->getLocalPlayer();
		if (!localPlayer->isSuggestionAccepted(SUGGESTION_WATCH_AD_REMINDED))
			m_propertyBar->setWatchAdReminderEnabled(true);
	});

	if (enableWatchAdHintIfNeeded)
	{
		auto localPlayer = sGameCenter->getLocalPlayer();
		if (!localPlayer->isSuggestionAccepted(SUGGESTION_WATCH_AD_REMINDED))
			wicket->setWatchAdHintEnabled(true);
	}

	wicket->show();

	m_propertyBar->setWatchAdReminderEnabled(false);
	m_propertyBar->hideTips();

	this->saveSuggestionAccepted(SUGGESTION_WATCH_AD_FIRST, true);
}	

void FuncScene::applyNicknameToLocalPlayer()
{
    LocalPlayer* localPlayer = sGameCenter->getLocalPlayer();
    std::string nickname = localPlayer->getNickname();
    std::string displayName = localPlayer->getDisplayName();
    
    // Set and save a nickname for LocalPlayer
    if(nickname.empty() && !displayName.empty())
    {
        InputBox* inputBox = m_nameInputBox->getInputBoxRender();
        int32 contentLength = InputBox::calcContentLength(displayName);
        if(contentLength <= inputBox->getMaxContentLength())
            nickname = displayName;
        else
            nickname = StringUtility::substring(displayName, inputBox->getMaxContentLength());
        localPlayer->setNickname(nickname);
        localPlayer->saveDataAsync();
    }
}

void FuncScene::updateNameInputBox()
{
    this->applyNicknameToLocalPlayer();
    
    LocalPlayer* localPlayer = sGameCenter->getLocalPlayer();
    std::string nickname = localPlayer->getNickname();
    
    if(!nickname.empty())
        m_nameInputBox->setText(nickname);
}

void FuncScene::nameInputBoxEditCallback(Ref* sender, InputBoxEditEvent event)
{
	switch(event)
	{
	case INPUTBOX_EVENT_RETURN:
	{
		std::string name = m_nameInputBox->getText();
		if(name != sGameCenter->getLocalPlayer()->getNickname())
		{
			sGameCenter->getLocalPlayer()->setNickname(name);
			sGameCenter->getLocalPlayer()->saveDataAsync();
		}
		break;
	}
	case INPUTBOX_EVENT_EDITING_DID_BEGIN:
		break;
	default:
		break;
	}

}

void FuncScene::buyHeroWithGameCoin()
{
	LocalPlayer* localPlayer = sGameCenter->getLocalPlayer();

	int32 price = m_currHeroTmpl->price;
	int32 property = localPlayer->getProperty();
	if (property >= price)
	{
		localPlayer->removeMoney(price);
		this->unlockHero(m_currHeroTmpl->id);
		m_propertyBar->updateAmount(true);
		if (!sStoreReview->isProcessCompleted(REVIEW_PROCESS_UPGRADE_STATS_OR_UNLOCK_HERO))
			sStoreReview->setProcessCompleted(REVIEW_PROCESS_UPGRADE_STATS_OR_UNLOCK_HERO);
	}
	else
	{
		MessageDialog* dialog = MessageDialog::create();
		dialog->setTitle(sLocaleMgr->getString("func_dlg_title_not_enough_gold"));
		dialog->addPositiveButton(sLocaleMgr->getString("func_dlg_btn_buy"), [this](Ref* sender) {
			this->showShopWicket(false);
		});
        dialog->addCancelButton(nullptr);
		dialog->setMessage(sLocaleMgr->getString("func_dlg_msg_not_enough_gold"));
		dialog->show();
	}
}

void FuncScene::buyHeroWithCurrency()
{
	this->requestPayment(m_currHeroTmpl->productId);
}

void FuncScene::unlockHero(HeroID heroId, bool playSoundForCurrentHero)
{
	NS_ASSERT(heroId != HERO_NONE);

	LocalPlayer* localPlayer = sGameCenter->getLocalPlayer();
	if (!localPlayer->hasUnlockedHero(heroId))
	{
		localPlayer->unlockHero(heroId);
		localPlayer->saveDataAsync();
	}

	if (heroId == m_currHeroTmpl->id)
	{
		m_funcBtn->updateButton();
		if (m_heroStatLayer->isShown())
			m_heroStatLayer->updateUpgradePanel();
		if(playSoundForCurrentHero)
			sSoundMgr->play(SOUND_UNLOCK_HERO);
	}

	auto hero = m_heroCatwalk->getHeroUnitByHeroId(heroId);
	if (hero)
		hero->updateLockState();
}

void FuncScene::giveGameCoin(int32 amount)
{
	sGameCenter->getLocalPlayer()->addMoney(amount);
	sGameCenter->getLocalPlayer()->saveDataAsync();
	if (!Wicket::getWicketByName(WICKET_NAME_SHOP))
	{
		MessageDialog* dialog = MessageDialog::create();
		dialog->setTitle(sLocaleMgr->getString("func_dlg_title_golds_delivered"));
		dialog->setMessage(StringUtils::format(sLocaleMgr->getString("func_dlg_msg_golds_delivered").c_str(), Utils::separateThousands(amount).c_str()));
		dialog->addOkButton([this](Ref* sender) {
			m_propertyBar->updateAmount(true);
			this->applyUpgradeStatsSuggestion();
		});
		dialog->show();
	}
}

void FuncScene::removeAds(bool isRestored)
{
    if(sGameCenter->getLocalPlayer()->isAdsRemoved())
        return;

    sGameCenter->getLocalPlayer()->setAdsRemoved(true);
    sGameCenter->getLocalPlayer()->saveDataAsync();
    if(!isRestored)
    {
		MessageDialog* dialog = MessageDialog::create();
		dialog->setTitle(sLocaleMgr->getString("func_dlg_title_ads_removed"));
		dialog->setMessage(sLocaleMgr->getString("func_dlg_msg_ads_removed"));
		dialog->addOkButton([this](Ref* sender){
			m_removeAdsBtn->setVisible(false);
			this->layoutShortcutButtons();
		});
		dialog->show();
    }
    else
    {
        m_removeAdsBtn->setVisible(false);
        this->layoutShortcutButtons();
    }
}

void FuncScene::deliverGoods(bool isRestored, std::string const& productId)
{
	IAPItem const* item = sShopMgr->getIAPItem(productId);
    if(item->isNull())
    {
        CCLOG("There are no IAP item with product ID %s.", productId.c_str());
        return;
    }
    
	// Cannot restore purchased consumable
    if(isRestored && item->productType == PRODUCT_TYPE_CONSUMABLE)
    {
        CCLOG("Product ID %s is consumable and cannot be restored.", productId.c_str());
        return;
    }
    
	switch (item->templateType)
	{
	case TEMPLATE_TYPE_HERO:
	{
		HeroTemplate const* tmpl = sShopMgr->getHeroTemplateByProductId(productId);
		this->unlockHero(tmpl->id, !isRestored);
		break;
	}
	case TEMPLATE_TYPE_GAME_COIN:
	{
        GameCoinTemplate const* tmpl = sShopMgr->getGameCoinTemplateByProductId(productId);
        this->giveGameCoin(tmpl->amount);
		break;
	}
	case TEMPLATE_TYPE_REMOVE_ADS:
    {
        this->removeAds(isRestored);
        break;
    }
	default: // TEMPLATE_TYPE_NONE
		break;
	}
}

void FuncScene::buttonPlayCallback(Ref* sender)
{
	this->saveUserConfigs();

	sSoundMgr->play(SOUND_PLAY);
	this->enableUserInteraction(false);

	sAnalytics->logSelectContent(kContentTypeButton, "play");

    this->startGameLoadScene();
}

void FuncScene::buttonBuyCallback(Ref* sender, MoneyType type)
{
	switch (type)
	{
	case MONEY_TYPE_CURRENCY:
        this->buyHeroWithCurrency();
		break;
	default:
        this->buyHeroWithGameCoin();
		break;
	}

    sSoundMgr->play(SOUND_BUTTON);
}

void FuncScene::buttonOptionCallback(Ref* sender)
{
#if USE_DEBUG_OPTION
	if (!this->isScheduled(SCHEDULE_KEY_SHOW_OPTION_WICKET))
		return;
	else
		this->unschedule(SCHEDULE_KEY_SHOW_OPTION_WICKET);
#endif // USE_DEBUG_OPTION

    OptionWicket* wicket = OptionWicket::create(this);
    wicket->show();
    sSoundMgr->play(SOUND_BUTTON);
}

void FuncScene::buttonRankingCallback(Ref* sender)
{
    LocalPlayer const* localPlayer = sGameCenter->getLocalPlayer();
    if(localPlayer)
    {
        if(!localPlayer->isAuthenticated())
        {
#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
            int32 errorCode = GoogleServicesUtils::checkGooglePlayServices();
            switch (errorCode)
            {
            case ServicesConnectionResult::SERVICE_SUCCESS:
                if (sGameCenter->startAuthorizationUI())
                    m_isShowingLeaderboard = true;
                break;
            case ServicesConnectionResult::SERVICE_MISSING:
            case ServicesConnectionResult::SERVICE_INVALID:
			{
				MessageDialog* dialog = MessageDialog::create();
				dialog->setTitle(sLocaleMgr->getString("gamecenter_dlg_title_gps_invalid"));
				dialog->setMessage(sLocaleMgr->getString("gamecenter_dlg_msg_gps_invalid"));
				dialog->addOkButton(nullptr);
				dialog->show();
				break;
			}
            default:
                GoogleServicesUtils::showErrorDialog(errorCode);
                break;
            }
#else
            MessageDialog* dialog = MessageDialog::create();
			dialog->setTitle(sLocaleMgr->getString("gamecenter_dlg_title_no_authenticated"));
            dialog->setMessage(sLocaleMgr->getString("gamecenter_dlg_msg_no_authenticated"));
            dialog->addOkButton(nullptr);
            dialog->show();
#endif // CC_TARGET_PLATFORM
        }
        else
            sGameCenter->showLeaderboard(LEADERBOARD_ID_TOTAL_KILLS);
    }
    
    sSoundMgr->play(SOUND_BUTTON);

    sAnalytics->logSelectContent(kContentTypeButton,"ranking");
}

void FuncScene::buttonShareCallback(Ref* sender)
{
    this->showLoadingView();
    sSoundMgr->play(SOUND_BUTTON);
	this->scheduleOnce([this](float dt)
	{
		int32 totalKills = sGameCenter->getLocalPlayer()->getScoreValue(LEADERBOARD_ID_TOTAL_KILLS);
		ShareInfo info;
		info.title = sLocaleMgr->getString("share_info_title");
		info.text = StringUtils::format(sLocaleMgr->getString("share_info_text").c_str(), totalKills);
		info.link = STORE_APP_URL;
        Vec2 popoverAnchor(m_shareBtn->getBoundingBox().getMidX(),
                           m_shareBtn->getBoundingBox().getMinY());
		sShare->share(info, popoverAnchor);
	}, 0.06f, "ShareDelayed");

    sAnalytics->logSelectContent(kContentTypeButton,"share");

}

void FuncScene::buttonRemoveAdsCallback(Ref* sender)
{
	sSoundMgr->play(SOUND_BUTTON);

	RemoveAdsDialog* dialog = RemoveAdsDialog::create();
	dialog->setBuyCallback([this](Ref* sender, ProductTemplate const* tmpl) {
	    this->requestPayment(tmpl->productId, 0.15f);
	});

	dialog->show();

    sAnalytics->logSelectContent(kContentTypeButton,"remove_ads");
}

void FuncScene::buttonHeroStatCallback(Ref* sender)
{
	if (m_heroStatLayer->isShown())
		m_heroStatLayer->hide();
	else
	{
		this->saveSuggestionAccepted(SUGGESTION_UPGRADE_STATS, true);
		if(m_fingerTapProcess == FINGERTAP_PROCESS_UPGRADE_STATS)
			this->updateRecommendedUpgradeTypes();

		m_heroStatLayer->show(m_currHeroTmpl->id);

		sAnalytics->logSelectContent(kContentTypeButton,"hero_stat");
	}

	sSoundMgr->play(SOUND_BUTTON);

	if (m_heroStatBtn->getShownTipsType() != HeroStatButton::TIPS_NONE)
		m_heroStatBtn->hideTips();
}

void FuncScene::propertyBarClickCallback(Ref* sender)
{
	this->showShopWicket(true);

    sAnalytics->logSelectContent(kContentTypeButton,"property_bar");
}

void FuncScene::giftBoxClickCallback(Ref* sender)
{
	if (this->getDailyRewardState() == DAILY_REWARD_STATE_READY)
	{
		m_giftBox->hideTips();
		m_giftBox->setGiftState(GiftBox::GIFT_STATE_NONE);

		DailyRewardDialog* dialog = DailyRewardDialog::create();
		if (m_fingerTapProcess == FINGERTAP_PROCESS_CLAIM_REWARD)
		{
			m_fingerTap->hide();
			dialog->setShowFingerTap(true);
		}
		dialog->setWatchAdButtonCallback([this](Ref* sender, DailyRewardType rewardType, int32 value) {
			RewardedAdConfig adConfig;
			adConfig.adType = RewardedAdConfig::AD_TYPE_DAILY_REWARD;
			adConfig.value1 = rewardType;
			adConfig.value2 = value;
			this->requestRewardedVideo(adConfig);
		});
		dialog->setClaimButtonCallback([this](Ref* sender, DailyRewardType rewardType, int32 value) {
			this->giveDailyReward(rewardType, value, false);

			this->updateGiftBoxState();
			this->setFingerTapProcessCompleted(FINGERTAP_PROCESS_CLAIM_REWARD);
			this->applyFirstWatchAdTips();

		});
		dialog->show();

		sAnalytics->logSelectContent(kContentTypeButton,"gift_box");
	}
}

void FuncScene::updateSelectedHero()
{
	uint32 heroId = sGameCenter->getLocalPlayer()->getHeroId();
	if (heroId == HERO_NONE)
		m_currHeroTmpl = sShopMgr->getHeroTemplateByHeroId(DEFAULT_HERO_ID);
	else
		m_currHeroTmpl = sShopMgr->getHeroTemplateByHeroId(heroId);

	this->resetUIForSelectHero();
	m_heroCatwalk->setSelectedHero(m_currHeroTmpl->id, false);
	this->updateUIForSelectedHero();
}

void FuncScene::resetUIForSelectHero()
{
	if (m_heroStatBtn->getShownTipsType() != HeroStatButton::TIPS_NONE)
		m_heroStatBtn->hideTips();

	switch (m_fingerTapProcess)
	{
	case FINGERTAP_PROCESS_UPGRADE_STATS:
		if (!m_heroStatLayer->isShown())
			m_fingerTap->hide();
		break;
	case FINGERTAP_PROCESS_PLAY:
		m_fingerTap->hide();
		break;
	default:
		break;
	}
}

void FuncScene::updateUIForSelectedHero()
{
	if (m_heroStatLayer->isShown())
	{
		if (m_fingerTapProcess == FINGERTAP_PROCESS_UPGRADE_STATS)
			this->updateRecommendedUpgradeTypes();

		m_heroStatLayer->show(m_currHeroTmpl->id);
	}
	else
	{
		if (m_fingerTapProcess == FINGERTAP_PROCESS_UPGRADE_STATS)
			this->applyUpgradeStatsTips();
	}

	m_funcBtn->setHeroTemplate(m_currHeroTmpl);
	this->applyPlayFingerTap();
}

void FuncScene::heroCatwalkSelectCallback(Ref* sender, HeroTemplate const* tmpl)
{
	m_currHeroTmpl = tmpl;
	this->updateUIForSelectedHero();
}

void FuncScene::heroCatwalkBeginScrollCallback(Ref* sender)
{
	this->resetUIForSelectHero();
    if(!sStore->isProductIdsValidated() && !sStore->isValidatingProductIds())
        this->validateProductIds();
}

void FuncScene::heroStatUpgradedCallback(Ref* sender, HeroStatLayer::StatUpgradeAction action)
{
	switch (action)
	{
	case HeroStatLayer::STAT_UPGRADE_ACTION_UPGRADED:
		m_propertyBar->updateAmount(true);
		if (m_fingerTapProcess == FINGERTAP_PROCESS_UPGRADE_STATS)
		{
			if (m_heroStatLayer->isAnyOfUpgradeTypesRecommended())
			{
				m_heroStatLayer->resetRecommendedUpgradeTypes();
				m_heroStatLayer->updateUpgradePanel();
			}
		}
		this->setFingerTapProcessCompleted(FINGERTAP_PROCESS_UPGRADE_STATS);
		this->applyFirstWatchAdTips();
		break;
	case HeroStatLayer::STAT_UPGRADE_ACTION_BUY_OK:
		this->showShopWicket(false);
		break;
	case HeroStatLayer::STAT_UPGRADE_ACTION_BUY_CANCEL:
		break;
	default:
		break;
	}
}

void FuncScene::heroStatVisibleStateChangedCallback(Ref* sender, HeroStatLayer::VisibleState state)
{
	if (m_fingerTapProcess != FINGERTAP_PROCESS_UPGRADE_STATS)
		return;

	switch (state)
	{
	case HeroStatLayer::VISIBLE_STATE_SHOWN:
	{
		Point pos;
		if (this->getRecommendedUpgradeButtonPosition(pos))
			m_fingerTap->show(pos, 0.5f);
		break;
	}
	case HeroStatLayer::VISIBLE_STATE_UPDATED:
	{
		m_fingerTap->hide();
		Point pos;
		if (this->getRecommendedUpgradeButtonPosition(pos))
			m_fingerTap->show(pos, 0.5f);
		break;
	}
	case HeroStatLayer::VISIBLE_STATE_WILL_HIDE:
		m_fingerTap->hide();
		break;
	default:
		break;
	}
}

void FuncScene::heroStatUpgradePanelScrollCallback(Ref* sender, StatUpgradePanel::ListScrollEventType eventType)
{
	if (m_fingerTapProcess != FINGERTAP_PROCESS_UPGRADE_STATS)
		return;
	
	switch (eventType)
	{
	case StatUpgradePanel::LIST_SCROLLING_BEGAN:
		m_fingerTap->hide();
		break;
	case StatUpgradePanel::LIST_SCROLLING_ENDED:
		if (m_heroStatLayer->isShown())
		{
			Point pos;
			if (this->getRecommendedUpgradeButtonPosition(pos))
				m_fingerTap->show(pos);
		}
		break;
	default:
		break;
}
}

void FuncScene::enableUserInteraction(bool enabled)
{
	m_nameInputBox->setEnabled(enabled);
	m_funcBtn->setEnabled(enabled);
}

void FuncScene::saveUserConfigs()
{
	auto localPlayer = sGameCenter->getLocalPlayer();
	localPlayer->setHeroId(m_currHeroTmpl->id);
	localPlayer->saveDataAsync();
}

void FuncScene::scheduleSnowEffect()
{
	float delay = SNOWEFFECT_DELAY_MIN + CCRANDOM_0_1() * (SNOWEFFECT_DELAY_MAX - SNOWEFFECT_DELAY_MIN);
	CCLOG("SnowEffect delay: %f", delay);
	this->scheduleOnce([this](float dt) {
		this->startSnowEffect();
	}, delay, "SnowEffect");
}

void FuncScene::startSnowEffect()
{
	float dur = SNOWEFFECT_DURATION_MIN + CCRANDOM_0_1() * (SNOWEFFECT_DURATION_MAX - SNOWEFFECT_DURATION_MIN);
	float inr = SNOWEFFECT_INTERVAL_MIN + CCRANDOM_0_1() * (SNOWEFFECT_INTERVAL_MAX - SNOWEFFECT_INTERVAL_MIN);
	auto director = Director::getInstance();
	auto visSize = director->getVisibleSize();
	float scale = scale = visSize.width / designResolutionSize.width;

	Vec2 grivity(CCRANDOM_MINUS1_1() * 10, -10);
	int32 flakeIncr = (int32)(CCRANDOM_0_1() * 20);
	int32 bgFlakes = (int32)((25 + flakeIncr) * scale);
	int32 fgFlakes = (int32)((8 + flakeIncr) * scale);
	CCLOG("SnowEffect dur: %f delay: %f grivity:[%f,%f] bg flakes:%d fg flakes:%d", dur, inr, grivity.x, grivity.y, bgFlakes, fgFlakes);
 
	m_snowEffectBg->start(dur, bgFlakes, grivity, 15, m_isNarrowScreen ? 5.40f : 4.75f, 0.f, 10, 2);
	m_snowEffectFg->start(dur, fgFlakes, grivity, 50, m_isNarrowScreen ? 4.40f : 3.75f, 0.f, 20, 4);
	this->scheduleOnce([this](float dt) {
		this->startSnowEffect();
	}, dur + inr, "SnowEffect");
}

void FuncScene::requestPayment(std::string productId, float delay)
{
	if (!sStore->canMakePayments())
	{
		MessageDialog* dialog = MessageDialog::create();
		dialog->addOkButton(nullptr);
		dialog->setTitle(sLocaleMgr->getString("store_dlg_title_buy_fail"));
		dialog->setMessage(sLocaleMgr->getString("store_dlg_msg_not_allowed_make_payments"));
		dialog->show();
	}
	else
	{
		if (!sStore->hasDeferredTransaction(productId))
        {
		    if(delay > 0)
            {
                this->scheduleOnce([this, productId](float dt){
                    sStore->requestPayment(productId);
                }, delay, "RequestPayment");
            }
		    else
		        sStore->requestPayment(productId);
        }
		else
		{
#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
			MessageDialog* dialog = MessageDialog::create();
			dialog->setTitle(sLocaleMgr->getString("store_dlg_title_buy_deferred"));
			dialog->addOkButton(nullptr);
			dialog->setMessage(sLocaleMgr->getString("store_dlg_msg_pending_purchase"));
			dialog->show();
#elif CC_TARGET_PLATFORM == CC_PLATFORM_IOS
			MessageDialog* dialog = MessageDialog::create();
			dialog->setTitle(sLocaleMgr->getString("store_dlg_title_buy_deferred"));
			dialog->addOkButton(nullptr);
			dialog->setMessage(sLocaleMgr->getString("store_dlg_msg_pending_approval"));
			dialog->show();
#endif
		}
	}
}

void FuncScene::validateProductIds()
{
    std::vector<std::string> productIds = sShopMgr->getProductIdList();
    sStore->validateProductIds(productIds);
}

void FuncScene::showLoadingView()
{
    if (!m_loadingView)
    {
        m_loadingView = LoadingView::create(this);
        m_loadingView->show();
    }
}

void FuncScene::dismissLoadingView()
{
    if (m_loadingView)
    {
        m_loadingView->dismiss();
        m_loadingView = nullptr;
    }
}

void FuncScene::onAuthStarted(AuthOperation op)
{
    this->showLoadingView();
}

#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID

void FuncScene::onAuthFinished(AuthOperation op, AuthResponse const& response)
{
	this->dismissLoadingView();

    if(op == AuthOperation::SIGN_IN)
    {
        switch (response.state)
        {
        case AUTH_STATE_SUCCESS:
            this->updateNameInputBox();
            this->updateTotalKills();
            this->updateSelectedHero();
            m_propertyBar->updateAmount(false);

            sGameCenter->reportScores(LEADERBOARD_ID_TOTAL_KILLS);
            if (m_isShowingLeaderboard)
                sGameCenter->showLeaderboard(LEADERBOARD_ID_TOTAL_KILLS);
            break;
        case AUTH_STATE_FAIL:
            this->handleAuthError(response.error);
            break;
        default:
            break;
        }
    }

	m_isShowingLeaderboard = false;
}


void FuncScene::handleAuthError(GameCenterError error)
{
    if(error.code == GAMECENTER_ERROR_CANCELED)
        return;

    if(error.code == GAMECENTER_ERROR_LOAD_DATA_FAILED)
    {
        MessageDialog* dialog = MessageDialog::create();
        dialog->setTitle(sLocaleMgr->getString("gamecenter_dlg_title_playerdata_error"));
        dialog->setMessage(sLocaleMgr->getString("gamecenter_dlg_msg_playerdata_error"));
        dialog->addOkButton([this](Ref* sender) {
            Utils::exitApp();
        });
        dialog->show();
    }
    else if(error.code == GAMECENTER_ERROR_ACCOUNT_CHANGED)
    {
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
        MessageDialog* dialog = MessageDialog::create();
        dialog->setTitle(sLocaleMgr->getString("gamecenter_dlg_title_auth_failed"));
        dialog->addOkButton(nullptr);
        switch (error.code)
        {
        case GAMECENTER_ERROR_COMMUNICATIONS_FAILURE:
            dialog->setMessage(sLocaleMgr->getString("gamecenter_dlg_msg_network_error"));
            break;
        case GAMECENTER_ERROR_TIMEOUT:
            dialog->setMessage(sLocaleMgr->getString("gamecenter_dlg_msg_timeout"));
            break;
        case GAMECENTER_ERROR_GPS_VERSION_UPDATE_REQUIRED:
            dialog->setMessage(sLocaleMgr->getString("gamecenter_dlg_msg_gps_outofdate"));
            break;
        default:
            dialog->setMessage(StringUtils::format(sLocaleMgr->getString("gamecenter_dlg_msg_auth_failed").c_str(), (int32)error.code));
            break;
        }
        dialog->show();
    }
}
#else

void FuncScene::onAuthFinished(AuthOperation op, AuthResponse const& response)
{
    this->dismissLoadingView();

    switch (response.state) {
        case AUTH_STATE_SUCCESS:
            this->updateNameInputBox();
            this->updateTotalKills();
            this->updateSelectedHero();
            m_propertyBar->updateAmount(false);

            sGameCenter->reportScores(LEADERBOARD_ID_TOTAL_KILLS);
            break;
        case AUTH_STATE_FAIL:
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
            else if(response.error.code == GAMECENTER_ERROR_ACCOUNT_CHANGED)
            {
                MessageDialog* dialog = MessageDialog::create();
                dialog->setTitle(sLocaleMgr->getString("gamecenter_dlg_title_account_switched"));
                dialog->setMessage(sLocaleMgr->getString("gamecenter_dlg_msg_account_switched"));
                dialog->addOkButton([this](Ref* sender) {
                    Utils::exitApp();
                });
                dialog->show();
            }
            break;
        default:
            break;
    }
}

#endif // CC_TARGET_PLATFORM

void FuncScene::onReportScoreSuccess()
{
    
}

void FuncScene::onReportScoreFail(GameCenterError const& error)
{
    
}

void FuncScene::onProductsRequestFinished()
{
    if(m_funcBtn->getHeroTemplate()->productType != PRODUCT_TYPE_PRIZE)
		m_funcBtn->updateButton();
}

void FuncScene::onProductsRequestFailed(StoreError const& error)
{
	if(m_funcBtn->getHeroTemplate()->productType != PRODUCT_TYPE_PRIZE)
		m_funcBtn->updateButton();
}

#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID

void FuncScene::onPaymentTransactionUpdated(PaymentTransaction const& transaction)
{
	GoogleBillingTransaction const& gbTrans = static_cast<GoogleBillingTransaction const&>(transaction);
	switch(gbTrans.state)
	{
		case PAYMENT_STATE_PURCHASED:
		case PAYMENT_STATE_RESTORED:
		{
			IAPItem const* item = sShopMgr->getIAPItem(gbTrans.productId);
//			NS_ASSERT(!item->isNull());
			switch(item->productType)
			{
				case PRODUCT_TYPE_NON_CONSUMABLE:
					if(!gbTrans.isAcknowledged)
						sStore->acknowledgeTransaction(gbTrans);
					else
						this->deliverGoods(gbTrans.state == PAYMENT_STATE_RESTORED, gbTrans.productId);
					break;
				case PRODUCT_TYPE_CONSUMABLE:
					sStore->consume(gbTrans);
					break;
				default:
					break;
			}
			break;
		}
		case PAYMENT_STATE_DEFERRED:
		{
			MessageDialog* dialog = MessageDialog::create();
			dialog->setTitle(sLocaleMgr->getString("store_dlg_title_buy_deferred"));
			dialog->addOkButton(nullptr);
            dialog->setMessage(sLocaleMgr->getString("store_dlg_msg_pending_purchase"));
			dialog->show();
			break;
		}
		default: // PAYMENT_STATE_UNSPECIFIED | PAYMENT_STATE_FAILED
			this->handleTransactionError(gbTrans.error);
			break;
	}
}

void FuncScene::handleTransactionError(StoreError const& error)
{
    switch (error.code) {
        case STORE_ERROR_CANCELLED:
            break;
        default:
        {
            MessageDialog* dialog = MessageDialog::create();
			dialog->setTitle(sLocaleMgr->getString("store_dlg_title_buy_fail"));
            dialog->addOkButton(nullptr);

            switch(error.code)
            {
                case STORE_ERROR_SERVICE_DISCONNECTED:
                case STORE_ERROR_SERVICE_UNAVAILABLE:
                case STORE_ERROR_FEATURE_NOT_SUPPORTED:
					dialog->setMessage(sLocaleMgr->getString("store_dlg_msg_google_play_services_error"));
                    break;
            	case STORE_ERROR_ITEM_ALREADY_OWNED:
					dialog->setMessage(sLocaleMgr->getString("store_dlg_msg_item_already_owned"));
            		break;
                default:
                    dialog->setMessage(sLocaleMgr->getString("store_dlg_msg_buy_fail"));
                    break;
            }
            dialog->show();
            break;
        }
    }


}

void FuncScene::onPurchasesFailed(StoreError const& error)
{
	this->handleTransactionError(error);
}

void FuncScene::onConsumeFinished(PaymentTransaction const& transaction)
{
	this->deliverGoods(false, transaction.productId);
}

void FuncScene::onConsumeFailed(StoreError const& error)
{
	this->handleTransactionError(error);
}

void FuncScene::onAcknowledgePurchaseFinished(PaymentTransaction const& transaction)
{
    NS_ASSERT(transaction.state == PAYMENT_STATE_RESTORED || transaction.state == PAYMENT_STATE_PURCHASED);
	this->deliverGoods(transaction.state == PAYMENT_STATE_RESTORED, transaction.productId);
}

void FuncScene::onAcknowledgePurchaseFailed(StoreError const& error)
{
	this->handleTransactionError(error);
}

#elif CC_TARGET_PLATFORM == CC_PLATFORM_IOS

void FuncScene::onPaymentTransactionUpdated(PaymentTransaction const& transaction)
{
    switch(transaction.state)
    {
        case PAYMENT_STATE_PURCHASED:
            this->dismissLoadingView();
            this->deliverGoods(false, transaction.productId);
            sStore->finishTransaction(transaction.transactionId);
            break;
        case PAYMENT_STATE_RESTORED:
			this->deliverGoods(true, transaction.productId);
            break;
        case PAYMENT_STATE_PURCHASING:
            this->showLoadingView();
            break;
        case PAYMENT_STATE_DEFERRED:
        {
			// Need to wait for parental approval before completing the purchase
            this->dismissLoadingView();
            MessageDialog* dialog = MessageDialog::create();
			dialog->setTitle(sLocaleMgr->getString("store_dlg_title_buy_deferred"));
            dialog->addOkButton(nullptr);
            dialog->setMessage(sLocaleMgr->getString("store_dlg_msg_pending_approval"));
            dialog->show();
            break;
        }
        default: // PAYMENT_STATE_FAILED
        {
            this->dismissLoadingView();
            this->handleTransactionError(transaction.error);
            sStore->finishTransaction(transaction.transactionId);
            break;
        }
    }
}

void FuncScene::handleTransactionError(StoreError const& error)
{
    switch (error.code) {
        case STORE_ERROR_CANCELLED:
            break;
        default:
        {
            MessageDialog* dialog = MessageDialog::create();
			dialog->setTitle(sLocaleMgr->getString("store_dlg_title_buy_fail"));
            dialog->addOkButton(nullptr);

            switch(error.code)
            {
                case STORE_ERROR_URL_CANNOT_CONNECT_TO_HOST:
                case STORE_ERROR_URL_TIMED_OUT:
                case STORE_ERROR_URL_UNKNOWN:
                    dialog->setMessage(sLocaleMgr->getString("store_dlg_msg_unable_connect_appstore"));
                    break;
                default:
                    dialog->setMessage(sLocaleMgr->getString("store_dlg_msg_buy_fail"));
                    break;
            }
            dialog->show();
            break;
        }
    }
}

#else

void FuncScene::onPaymentTransactionUpdated(PaymentTransaction const& transaction) { }
void FuncScene::handleTransactionError(StoreError const& error) { }

#endif

void FuncScene::onShareState(ShareResponse const& response)
{
    this->dismissLoadingView();
    switch(response.state)
    {
        case SHARE_STATE_FAIL:
        {
            MessageDialog* dialog = MessageDialog::create();
            dialog->addOkButton(nullptr);
            if(response.errorCode == ShareErrorCode::SHARE_ERROR_NO_APP)
            	dialog->setMessage(sLocaleMgr->getString("share_error_noapp"));
            else
				dialog->setMessage(sLocaleMgr->getString("share_error_unable_to_share"));

            dialog->show();
            break;
        }
        default:
            break;
    }
}

void FuncScene::onTimeSyncSuccess(int64 millis)
{
	this->updateGiftBoxState();
}

void FuncScene::onTimeSyncFail()
{
}


void FuncScene::onRewardedAdClosed(RewardedAdConfig const& adConfig, bool isEarnedReward)
{
	this->dismissLoadingView();

	switch (adConfig.adType)
	{
	case RewardedAdConfig::AD_TYPE_DAILY_REWARD:
		this->updateGiftBoxState();
		this->setFingerTapProcessCompleted(FINGERTAP_PROCESS_CLAIM_REWARD);
		this->applyFirstWatchAdTips();
		break;
	case RewardedAdConfig::AD_TYPE_BATTLE_OUTCOME:
    {
        bool isWatchAd = false;
        if (!isEarnedReward)
            isWatchAd = this->showInterstitialAdIfNeeded();
        if (!isWatchAd)
        	this->updateUIAfterAdHidden();
        break;
    }
	default:
		break;
	}
}

void FuncScene::onUserEarnedReward(RewardedAdConfig const& adConfig)
{
	switch (adConfig.adType)
	{
	case RewardedAdConfig::AD_TYPE_DAILY_REWARD:
	{
		DailyRewardType rewardType = (DailyRewardType)adConfig.value1.asInt();
		int32 value = adConfig.value2.asInt();
		this->giveDailyReward(rewardType, value, true);
		break;
	}
	case RewardedAdConfig::AD_TYPE_BATTLE_OUTCOME:
	{
		int32 amount = adConfig.value1.asInt();
		this->giveOutcomeReward(amount);
		break;
	}
	default:
		break;
	}
}

void FuncScene::onRewardedAdRequestFailed(RewardedAdConfig const& adConfig, AdError const& error)
{
	this->dismissLoadingView();

	switch (adConfig.adType)
	{
	case RewardedAdConfig::AD_TYPE_DAILY_REWARD:
		this->updateGiftBoxState();
		break;
	case RewardedAdConfig::AD_TYPE_BATTLE_OUTCOME:
		this->updateUIAfterAdHidden();
	    break;
	default:
		break;
	}

	if(error.errorCode != AD_ERROR_NONE)
	{
		MessageDialog* dialog = MessageDialog::create();
		dialog->setTitle(sLocaleMgr->getString("loadad_dlg_title_fail"));
        switch (error.errorCode) 
        {
            case AD_ERROR_OS_VERSION_TOO_LOW:
                dialog->setMessage(sLocaleMgr->getString("loadad_dlg_msg_os_version_too_low"));
                break;
            default:
                dialog->setMessage(sLocaleMgr->getString("loadad_dlg_msg_fail"));
                break;
        }
		dialog->show();
	}
}

void FuncScene::onInterstitialAdClosed()
{
	this->updateUIAfterAdHidden();
}

void FuncScene::onInterstitialAdLoaded()
{

}

void FuncScene::onInterstitialAdRequestFailed(AdError const& error)
{

}

#if USE_DEBUG_OPTION

void FuncScene::buttonOptionTouchCallback(Ref* sender, ui::Widget::TouchEventType eventType)
{
	switch (eventType)
	{
	case Widget::TouchEventType::BEGAN:
		this->scheduleOnce([this](float dt) {
			DebugOptionWicket* wicket = DebugOptionWicket::create(this);
			wicket->setResultListener(CC_CALLBACK_2(FuncScene::onDebugOptionWicketResult, this));
			wicket->show();
		}, 1.f, SCHEDULE_KEY_SHOW_OPTION_WICKET);
		break;
	case Widget::TouchEventType::CANCELED:
		this->unschedule(SCHEDULE_KEY_SHOW_OPTION_WICKET);
		break;
	default:
		break;
	}
}

void FuncScene::onDebugOptionWicketResult(Wicket::ResultCode resultCode, ValueMapIntKey const& data)
{
	if (data.find(DebugOptionWicket::DATA_KEY_HEROS_UNLOCKED) != data.end())
	{
		auto heroTmplList = sShopMgr->getHeroTemplateList();
		for (auto const& tmpl : *heroTmplList)
		{
			this->unlockHero(tmpl.id, false);
		}
	}

	if (data.find(DebugOptionWicket::DATA_KEY_REWARD_TIME_RESET) != data.end())
		this->updateGiftBoxState();
}

#endif

NS_END
