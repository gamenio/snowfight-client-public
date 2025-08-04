#include "DebugOptionWicket.h"

#include "game/GameConfig.h"

#if USE_DEBUG_OPTION

using namespace cocos2d::ui;

#include "common/utils/TimeUtil.h"
#include "game/ShopMgr.h"
#include "game/UserPreferences.h"
#include "game/gamecenter/GameCenter.h"
#include "game/store/Store.h"
#include "game/nts/TimeService.h"
#include "scene/gui/ModalDialog.h"
#include "scene/Utils.h"
#include "SelectCountryWicket.h"

NS_BEGIN

#define FRAME_SIZE						Size(300, 250)
#define OPTION_MARGIN_TOP				0
#define OPTION_MARGIN_LEFTRIGHT			16

#define COUNTRYOPT_BUTTON_SIZE			Size(120, 26)

#define TIME_UNKNOWN    "0000/00/00 00:00:00.000"
#define TIME_FORMAT     "Local time: %s\nRemote time: %s"

using SystemClock = std::chrono::system_clock;

DebugOptionWicket* DebugOptionWicket::create(Node* owner)
{
	DebugOptionWicket *ret = new (std::nothrow) DebugOptionWicket();
	if (ret && ret->init(owner))
	{
		ret->autorelease();
		return ret;
	}
	else
	{
		CC_SAFE_DELETE(ret);
		return nullptr;
	}
}

bool DebugOptionWicket::init(Node* owner)
{
	if (!Wicket::init(owner, FRAME_SIZE, "DEBUG OPTIONS", true, false))
		return false;

	m_main = Node::create();
	this->setContent(m_main);

	Size frameSize = m_main->getBoundingBox().size;

	// GM switch
	m_gameMasterClb = this->createCheckLabelBox("Login as GameMaster");
	m_gameMasterClb->setAnchorPoint(Point::ANCHOR_TOP_LEFT);
	m_gameMasterClb->setPosition(Vec2(0.0f, frameSize.height - OPTION_MARGIN_TOP));
	m_gameMasterClb->setSelected(sUserPreferences->isDebugOptionEnabled(DEBUG_OPTION_GAME_MASTER));
	m_gameMasterClb->addEventListener(CC_CALLBACK_2(DebugOptionWicket::checkboxGameMasterCallback, this));
	m_main->addChild(m_gameMasterClb);

	// Zoom in window
	m_zoomInWindowClb = this->createCheckLabelBox("Zoom In (2x)");
	m_zoomInWindowClb->setAnchorPoint(Point::ANCHOR_TOP_LEFT);
	m_zoomInWindowClb->setPosition(Vec2(m_gameMasterClb->getBoundingBox().getMaxX() + 12, m_gameMasterClb->getBoundingBox().getMaxY()));
	m_zoomInWindowClb->setSelected(sUserPreferences->isDebugOptionEnabled(DEBUG_OPTION_ZOOM_IN_WINDOW));
	m_zoomInWindowClb->addEventListener(CC_CALLBACK_2(DebugOptionWicket::checkboxZoomInWindowCallback, this));
	m_main->addChild(m_zoomInWindowClb);

	// Disable session timeout
	m_disableSessionTimeoutClb = this->createCheckLabelBox("Disable Session Timeout");
	m_disableSessionTimeoutClb->setAnchorPoint(Point::ANCHOR_TOP_LEFT);
	m_disableSessionTimeoutClb->setPosition(Vec2(0.0f, m_zoomInWindowClb->getBoundingBox().getMinY()));
	m_disableSessionTimeoutClb->setSelected(sUserPreferences->isDebugOptionEnabled(DEBUG_OPTION_SESSION_TIMEOUT_DISABLED));
	m_disableSessionTimeoutClb->addEventListener(CC_CALLBACK_2(DebugOptionWicket::checkboxDisableSessionTimeoutCallback, this));
	m_main->addChild(m_disableSessionTimeoutClb);

	// Keyboard and mouse control
	m_keyboardMouseClb = this->createCheckLabelBox("Keyboard Mouse");
	m_keyboardMouseClb->setAnchorPoint(Point::ANCHOR_TOP_LEFT);
	m_keyboardMouseClb->setPosition(Vec2(m_gameMasterClb->getBoundingBox().getMaxX() + 12, m_disableSessionTimeoutClb->getBoundingBox().getMaxY()));
	m_keyboardMouseClb->setSelected(sUserPreferences->getControllerType() == CONTROLLER_TYPE_KEYBOARD_MOUSE);
	m_keyboardMouseClb->addEventListener(CC_CALLBACK_2(DebugOptionWicket::checkboxKeyboardMouseCallback, this));
	m_main->addChild(m_keyboardMouseClb);

	m_countryLabel = Label::createWithSystemFont("RealmServer by Region: ", DEFAULT_SYSTEM_FONT, 13);
	m_countryLabel->setTextColor(Color4B(150, 129, 56, 255));
	Utils::enableBoldForLabel(m_countryLabel);
	m_countryLabel->setAnchorPoint(Point::ANCHOR_TOP_LEFT);
	m_countryLabel->setPosition(Vec2(4, m_disableSessionTimeoutClb->getBoundingBox().getMinY() - (COUNTRYOPT_BUTTON_SIZE.height - m_countryLabel->getContentSize().height) / 2 + 1));
	m_main->addChild(m_countryLabel);

	std::string countryName = "<None>";
	std::string countryCode = sUserPreferences->getCountry();
	if(!countryCode.empty())
		countryName = sLocaleMgr->getCountryName(countryCode);
	m_countryOptBtn = OptionButton::create(COUNTRYOPT_BUTTON_SIZE, countryName, "");
	m_countryOptBtn->setAnchorPoint(Point::ANCHOR_TOP_LEFT);
	m_countryOptBtn->setPosition(Vec2(m_countryLabel->getBoundingBox().getMaxX(), m_disableSessionTimeoutClb->getBoundingBox().getMinY()));
	m_countryOptBtn->addClickEventListener(CC_CALLBACK_1(DebugOptionWicket::optionButtonCountryCallback, this));
	m_main->addChild(m_countryOptBtn);

	// Unlock heroes
	m_unlockHeroesBtn = this->createButton("Unlock Heroes");
	m_unlockHeroesBtn->setPosition(Vec2(0.0f, m_countryOptBtn->getBoundingBox().getMinY()));
	m_unlockHeroesBtn->setAnchorPoint(Point::ANCHOR_TOP_LEFT);
	m_unlockHeroesBtn->addClickEventListener(CC_CALLBACK_1(DebugOptionWicket::buttonUnlockHeroesClickCallback, this));
	m_main->addChild(m_unlockHeroesBtn);

	// Clear user data
	m_clearUserDataBtn = this->createButton("Clear User Data");
	m_clearUserDataBtn->setPosition(Vec2(0.0f, m_unlockHeroesBtn->getBoundingBox().getMinY()));
	m_clearUserDataBtn->setAnchorPoint(Point::ANCHOR_TOP_LEFT);
	m_clearUserDataBtn->addClickEventListener(CC_CALLBACK_1(DebugOptionWicket::buttonClearUserDataClickCallback, this));
	m_main->addChild(m_clearUserDataBtn);
    
    // Reset reward time
    m_resetRewardTimeBtn = this->createButton("Reset Reward Time");
    m_resetRewardTimeBtn->setPosition(Vec2(0.0f, m_clearUserDataBtn->getBoundingBox().getMinY()));
    m_resetRewardTimeBtn->setAnchorPoint(Point::ANCHOR_TOP_LEFT);
    m_resetRewardTimeBtn->addClickEventListener(CC_CALLBACK_1(DebugOptionWicket::buttonResetRewardTimeClickCallback, this));
    m_main->addChild(m_resetRewardTimeBtn);

	std::stringstream ss;
	ss << "Cocos2d Debug: ";
#if COCOS2D_DEBUG
	ss << "Enabled";
#else
	ss << "Disabled";
#endif
	m_statusLabel = Label::createWithSystemFont(ss.str(), DEFAULT_SYSTEM_FONT, 9);
	m_statusLabel->setTextColor(Color4B(150, 129, 56, 255));
	m_statusLabel->setAnchorPoint(Point::ANCHOR_MIDDLE_BOTTOM);
	m_statusLabel->setHorizontalAlignment(TextHAlignment::CENTER);
	Utils::enableBoldForLabel(m_statusLabel);
	m_statusLabel->setPosition(Vec2(frameSize.width / 2, 2));
	m_main->addChild(m_statusLabel);
    
    m_timeLabel = Label::createWithSystemFont(StringUtils::format(TIME_FORMAT, TIME_UNKNOWN, TIME_UNKNOWN), DEFAULT_SYSTEM_FONT, 9);
    m_timeLabel->setTextColor(Color4B(150, 129, 56, 255));
    m_timeLabel->setAnchorPoint(Point::ANCHOR_MIDDLE_BOTTOM);
    m_timeLabel->setHorizontalAlignment(TextHAlignment::CENTER);
    Utils::enableBoldForLabel(m_timeLabel);
    m_timeLabel->setPosition(Vec2(frameSize.width / 2, m_statusLabel->getBoundingBox().getMaxY() + 3));
    m_main->addChild(m_timeLabel);
    
    this->schedule(CC_SCHEDULE_SELECTOR(DebugOptionWicket::updateTime), 1.0f, CC_REPEAT_FOREVER, 0.f);
    
	return true;
}

DebugOptionWicket::DebugOptionWicket() :
	m_gameMasterClb(nullptr),
	m_zoomInWindowClb(nullptr),
	m_countryOptBtn(nullptr),
	m_countryLabel(nullptr),
	m_disableSessionTimeoutClb(nullptr),
	m_keyboardMouseClb(nullptr),
	m_unlockHeroesBtn(nullptr),
	m_clearUserDataBtn(nullptr),
    m_resetRewardTimeBtn(nullptr),
	m_statusLabel(nullptr),
    m_timeLabel(nullptr)
{

}

DebugOptionWicket::~DebugOptionWicket()
{
	m_gameMasterClb = nullptr;
	m_zoomInWindowClb = nullptr;
	m_countryOptBtn = nullptr;
	m_countryLabel = nullptr;
	m_disableSessionTimeoutClb = nullptr;
	m_keyboardMouseClb = nullptr;
	m_unlockHeroesBtn = nullptr;
	m_clearUserDataBtn = nullptr;
    m_resetRewardTimeBtn = nullptr;
	m_statusLabel = nullptr;
    m_timeLabel = nullptr;
}

cocos2d::ui::Button* DebugOptionWicket::createButton(std::string const& title)
{
	Button* button = Button::create("btn_bg_positive.png", "", "", Widget::TextureResType::PLIST);
	button->setScale9Enabled(true);
	button->setTitleText(title);
	button->setTitleFontSize(12);
	button->setTitleColor(Color3B(243, 244, 237));
	Utils::enableBoldForLabel(button->getTitleRenderer());
	button->setContentSize(button->getTitleRenderer()->getContentSize() + Size(30, 14));

	return button;
}

CheckLabelBox* DebugOptionWicket::createCheckLabelBox(std::string const& title)
{
	CheckLabelBox* checkLabelBox = CheckLabelBox::create(title);
	Label* label = checkLabelBox->getLabel();
	label->setSystemFontSize(12);
	checkLabelBox->layoutElements();
	return checkLabelBox;
}

void DebugOptionWicket::onClosed()
{
	this->setResult(Wicket::RESULT_CODE_OK, m_data);
}

void DebugOptionWicket::checkboxGameMasterCallback(Ref* render, CheckLabelBox::EventType eventType)
{
	sUserPreferences->setDebugOptionEnabled(DEBUG_OPTION_GAME_MASTER, eventType == CheckLabelBox::EventType::SELECTED);
}

void DebugOptionWicket::checkboxZoomInWindowCallback(Ref* render, CheckLabelBox::EventType eventType)
{
	if (eventType == CheckLabelBox::EventType::SELECTED)
	{
		sUserPreferences->setDebugOptionEnabled(DEBUG_OPTION_ZOOM_IN_WINDOW, true);
		Director::getInstance()->getOpenGLView()->setFrameZoomFactor(2.0f);
	}
	else
	{
		sUserPreferences->setDebugOptionEnabled(DEBUG_OPTION_ZOOM_IN_WINDOW, false);
		Director::getInstance()->getOpenGLView()->setFrameZoomFactor(1.0f);
	}
}

void DebugOptionWicket::checkboxDisableSessionTimeoutCallback(Ref* render, CheckLabelBox::EventType eventType)
{
	sUserPreferences->setDebugOptionEnabled(DEBUG_OPTION_SESSION_TIMEOUT_DISABLED, eventType == CheckLabelBox::EventType::SELECTED);
}

void DebugOptionWicket::checkboxKeyboardMouseCallback(Ref* render, CheckLabelBox::EventType eventType)
{
	sUserPreferences->setControllerType(eventType == CheckLabelBox::EventType::SELECTED ? CONTROLLER_TYPE_KEYBOARD_MOUSE : CONTROLLER_TYPE_DUAL_STICKS);
}

void DebugOptionWicket::optionButtonCountryCallback(Ref* sender)
{
	SelectCountryWicket* selCountryWicket = SelectCountryWicket::create(this);
	selCountryWicket->setSelectEventListener(CC_CALLBACK_2(DebugOptionWicket::selectCountryCallback, this));
	selCountryWicket->setSelectedCountry(sUserPreferences->getCountry());
	selCountryWicket->show();
}

void DebugOptionWicket::selectCountryCallback(Ref* sender, CountryInfo const& data)
{
	m_countryOptBtn->setTitle(data.name);
	sUserPreferences->setCountry(data.code);
	sUserPreferences->setDebugOptionEnabled(DEBUG_OPTION_REALM_BY_REGION, !data.code.empty());
}

void DebugOptionWicket::buttonUnlockHeroesClickCallback(Ref* sender)
{
	LocalPlayer* localPlayer = sGameCenter->getLocalPlayer();
	auto tmplList = sShopMgr->getHeroTemplateList();
	bool unlocked = false;
	for (auto const& tmpl : *tmplList)
	{
		if (!localPlayer->hasUnlockedHero(tmpl.id))
		{
			localPlayer->unlockHero(tmpl.id);
			unlocked = true;
		}
	}
	if (unlocked)
	{
		m_unlockHeroesBtn->setEnabled(false);
		m_data[DATA_KEY_HEROS_UNLOCKED] = true;
		localPlayer->saveDataAsync([this](bool ret) {
			m_unlockHeroesBtn->setEnabled(true);
			MessageDialog* dialog = MessageDialog::create();
			dialog->setMessage("All heroes have been unlocked.");
			dialog->show();
		});
	}

}

void DebugOptionWicket::buttonClearUserDataClickCallback(Ref* render)
{
	sUserPreferences->clear();
    sStore->clearCachedData();
	m_clearUserDataBtn->setEnabled(false);
	sGameCenter->getLocalPlayer()->deleteDataAsync([this]() {
		m_clearUserDataBtn->setEnabled(true);
		MessageDialog* dialog = MessageDialog::create();
		dialog->setMessage("User data has been cleared.");
		dialog->addOkButton([this](Ref* sender) {
			Utils::exitApp();
		});
		dialog->show();
	});
}

void DebugOptionWicket::buttonResetRewardTimeClickCallback(Ref* sender)
{
    m_resetRewardTimeBtn->setEnabled(false);
    LocalPlayer* localPlayer = sGameCenter->getLocalPlayer();
    localPlayer->setRewardTime(0);
	localPlayer->setDailyRewardClaimTime(0);
	m_data[DATA_KEY_REWARD_TIME_RESET] = true;
    localPlayer->saveDataAsync([this](bool ret) {
        m_resetRewardTimeBtn->setEnabled(true);
        MessageDialog* dialog = MessageDialog::create();
        dialog->setMessage("Reward time has been reset.");
        dialog->show();
    });
}

void DebugOptionWicket::updateTime(float dt)
{
    if(sTimeService->getTimeState() == TimeService::TIME_SYNCED)
    {
        int64 remoteTime = sTimeService->getCurrentTimeMillis();
        auto localNow = SystemClock::now();
        SystemClock::time_point remoteNow(std::chrono::duration_cast<SystemClock::duration>(std::chrono::milliseconds(remoteTime)));
        m_timeLabel->setString(StringUtils::format(TIME_FORMAT, time_util::getDateTimeStr(localNow).c_str(), time_util::getDateTimeStr(remoteNow).c_str()));
    }
}

NS_END

#endif // USE_DEBUG_OPTION
