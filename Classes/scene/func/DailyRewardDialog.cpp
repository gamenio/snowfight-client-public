#include "DailyRewardDialog.h"

#include "scene/SoundMgr.h"
#include "scene/Utils.h"
#include "game/GameConfig.h"
#include "game/gamecenter/GameCenter.h"

using namespace cocos2d::ui;

NS_BEGIN

#define FRAME_SIZE							Size(290, 195)

#define BUTTON_PANEL_HEIGHT					36
#define BUTTON_PANEL_MARGIN_LEFTRIGHT       13
#define BUTTON_PANEL_MARGIN_BOTTOM			32

#define BUTTON_WIDTH						105

#define CONTENT_MARGIN_TOP					31
#define CONTENT_MARGIN_BOTTOM				24
#define CONTENT_MARGIN_LEFTRIGHT			13

#define GROUP_MARGIN_LEFTRIGHT				17
#define GROUP_HEIGHT						59

#define MESSAGE_MARGIN_LEFTRIGHT			20
#define CAPTION_LABEL_MARGIN_RIGHT			5

#define FRAMENAME_FRAME_BG							"form_bg.png"
#define FRAMENAME_DIALOG_GROUP_DIVIDER				"dlg_group_divider.png"
#define FRAMENAME_DIALOG_GROUP_BG					"dlg_group_bg.png"
#define FRAMENAME_DIALOG_TITLE_BG					"dailyreward_dlg_title_bg.png"
#define FRAMENAME_DIALOG_BTN_CLAIM_NEUTRAL			"btn_bg_neutral.png"
#define FRAMENAME_DIALOG_BTN_CLAIM_POSITIVE			"btn_bg_positive.png"
#define FRAMENAME_DIALOG_BTN_WATCH_AD				"btn_bg_positive.png"
#define FRAMENAME_BTN_ICON_WATCH_AD					"btn_icon_watch_ad.png"
#define FRAMENAME_BTN_ICON_GOLD						"btn_icon_gold.png"

#define DIALOG_PIC_GOLD_FRAMENAME					"dailyreward_dlg_pic_gold.png"
#define DIALOG_PIC_HERO_FRAMENAME_FORMAT			"dailyreward_dlg_pic_hero%d.png"


DailyRewardDialog* DailyRewardDialog::create()
{
	auto ret = new (std::nothrow) DailyRewardDialog();
	if (ret && ret->init())
	{
		ret->autorelease();
	}
	else
	{
		CC_SAFE_DELETE(ret);
	}

	return ret;
}

bool DailyRewardDialog::init()
{
	if (!ModalDialog::init(FRAME_SIZE))
		return false;

	SpriteFrame* titleBgFrame = SpriteFrameCache::getInstance()->getSpriteFrameByName(FRAMENAME_DIALOG_TITLE_BG);
	Node* titleBar = Node::create();
	titleBar->setAnchorPoint(Point::ANCHOR_MIDDLE_BOTTOM);
	titleBar->setPosition(FRAME_SIZE.width / 2, FRAME_SIZE.height - 34);
	titleBar->setContentSize(titleBgFrame->getOriginalSize());
	this->setTitleBar(titleBar);

	m_titleBg = Scale9Sprite::createWithSpriteFrame(titleBgFrame);
	m_titleBg->setPosition(titleBar->getContentSize().width / 2, titleBar->getContentSize().height / 2);
	m_titleBg->setAnchorPoint(Point::ANCHOR_MIDDLE);
	titleBar->addChild(m_titleBg);

	m_titleLabel = Label::createWithSystemFont(sLocaleMgr->getString("func_dailyreward_dlg_title"), DEFAULT_SYSTEM_FONT, 15);
	m_titleLabel->setPosition(titleBar->getContentSize().width / 2, 44);
	m_titleLabel->setAnchorPoint(Point::ANCHOR_MIDDLE);
	m_titleLabel->setTextColor(Color4B(237, 232, 239, 255));
	Utils::enableBoldForLabel(m_titleLabel);
	titleBar->addChild(m_titleLabel);

	m_contentPanel = Node::create();
	m_contentPanel->setPosition(CONTENT_MARGIN_LEFTRIGHT, FRAME_SIZE.height - CONTENT_MARGIN_TOP);
	m_contentPanel->setAnchorPoint(Point::ANCHOR_TOP_LEFT);
	m_contentPanel->setContentSize(Size(FRAME_SIZE.width - CONTENT_MARGIN_LEFTRIGHT * 2, FRAME_SIZE.height - CONTENT_MARGIN_TOP - CONTENT_MARGIN_BOTTOM));
	this->setContent(m_contentPanel);

	Node* buttonPanel = Node::create();
	buttonPanel->setPosition(FRAME_SIZE.width / 2, BUTTON_PANEL_MARGIN_BOTTOM);
	buttonPanel->setAnchorPoint(Point::ANCHOR_MIDDLE);
	buttonPanel->setContentSize(Size(FRAME_SIZE.width - BUTTON_PANEL_MARGIN_LEFTRIGHT * 2, BUTTON_PANEL_HEIGHT));
	this->setButtonPanel(buttonPanel);

	Scale9Sprite* background = Scale9Sprite::createWithSpriteFrameName(FRAMENAME_FRAME_BG);
	this->setBackground(background);

	this->initButtons();
	this->initContents();

	return true;
}

DailyRewardDialog::DailyRewardDialog() :
	m_todayRewardType(DAILY_REWARD_TYPE_NONE),
	m_todayValue(0),
	m_titleBg(nullptr),
	m_titleLabel(nullptr),
	m_titleNode(nullptr),
	m_messageLabel(nullptr),
	m_contentPanel(nullptr),
	m_groupContainer(nullptr),
	m_dividerSp(nullptr),
	m_todayContainer(nullptr),
	m_todayTitleLabel(nullptr),
	m_todayCaptionLabel(nullptr),
	m_daysContainer(nullptr),
	m_daysTitleLabel(nullptr),
	m_daysPicSp(nullptr),
	m_daysCaptionLabel(nullptr),
	m_watchAdBtn(nullptr),
	m_claimBtn(nullptr),
	m_fingerTap(nullptr)
{
}

DailyRewardDialog::~DailyRewardDialog()
{
	m_titleBg = nullptr;
	m_titleLabel = nullptr;
	m_titleNode = nullptr;
	m_messageLabel = nullptr;
	m_contentPanel = nullptr;
	m_groupContainer = nullptr;
	m_dividerSp = nullptr;
	m_todayContainer = nullptr;
	m_todayTitleLabel = nullptr;
	m_todayCaptionLabel = nullptr;
	m_daysContainer = nullptr;
	m_daysTitleLabel = nullptr;
	m_daysPicSp = nullptr;
	m_daysCaptionLabel = nullptr;
	m_fingerTap = nullptr;

	CC_SAFE_RELEASE_NULL(m_watchAdBtn);
	CC_SAFE_RELEASE_NULL(m_claimBtn);
}

void DailyRewardDialog::setShowFingerTap(bool isShown)
{
	if (isShown)
	{
		if (!m_fingerTap)
		{
			m_fingerTap = FingerTap::create();
			m_fingerTap->setAnchorPoint(Point::ANCHOR_TOP_LEFT);
			this->addChild(m_fingerTap);
		}
	}
	else
	{
		if (m_fingerTap)
		{
			m_fingerTap->removeFromParent();
			m_fingerTap = nullptr;
		}
	}

}

void DailyRewardDialog::onEnter()
{
	ModalDialog::onEnter();

	this->loadData();
}

void DailyRewardDialog::onAppeared()
{
	if (m_fingerTap)
	{
		Point screenPos = m_claimBtn->convertToWorldSpace(Vec2(m_claimBtn->getContentSize().width - 25, m_claimBtn->getContentSize().height / 2));
		m_fingerTap->show(this->convertToNodeSpace(screenPos));
	}
}

void DailyRewardDialog::onWillDisappear()
{
	if (m_fingerTap)
		m_fingerTap->hide();
}

void DailyRewardDialog::initButtons()
{
	m_watchAdBtn = IconButton::create(FRAMENAME_DIALOG_BTN_WATCH_AD, sLocaleMgr->getString("func_dailyreward_dlg_btn_watch_ad"), FRAMENAME_BTN_ICON_GOLD);
	m_watchAdBtn->setContentSize(Size(BUTTON_WIDTH, BUTTON_PANEL_HEIGHT));
	m_watchAdBtn->addClickEventListener([this](Ref* sender) {
		if (m_watchAdBtnCallback)
			m_watchAdBtnCallback(this, m_todayRewardType, m_todayValue);
		this->dismiss();
		sSoundMgr->play(SOUND_BUTTON);
	});
	m_watchAdBtn->setTitleFontSize(12);
	m_watchAdBtn->setTitleColor(Color4B(243, 244, 237, 255));
	m_watchAdBtn->setBackgroundIconImage(FRAMENAME_BTN_ICON_WATCH_AD);
	CC_SAFE_RETAIN(m_watchAdBtn);

	m_claimBtn = Button::create(FRAMENAME_DIALOG_BTN_CLAIM_NEUTRAL, "", "", Widget::TextureResType::PLIST);
	m_claimBtn->addClickEventListener([this](Ref* sender) {
		if (m_claimBtnCallback)
			m_claimBtnCallback(this, m_todayRewardType, m_todayValue);
		this->dismiss();
		sSoundMgr->play(SOUND_BUTTON);
	});
	m_claimBtn->setScale9Enabled(true);
	m_claimBtn->setTitleText(sLocaleMgr->getString("func_dailyreward_dlg_btn_claim"));
	m_claimBtn->setTitleFontSize(12);
	m_claimBtn->setTitleColor(Color3B(239, 243, 244));
	m_claimBtn->setContentSize(Size(BUTTON_WIDTH, BUTTON_PANEL_HEIGHT));
	Utils::enableBoldForLabel(m_claimBtn->getTitleRenderer());
	CC_SAFE_RETAIN(m_claimBtn);
}


void DailyRewardDialog::initContents()
{
	m_groupContainer = Node::create();
	m_groupContainer->setContentSize(Size(m_contentPanel->getContentSize().width - GROUP_MARGIN_LEFTRIGHT * 2, GROUP_HEIGHT));
	m_groupContainer->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
	m_groupContainer->setPosition(GROUP_MARGIN_LEFTRIGHT, 36);
	m_contentPanel->addChild(m_groupContainer);

	Scale9Sprite* groupBg = Scale9Sprite::createWithSpriteFrameName(FRAMENAME_DIALOG_GROUP_BG);
	groupBg->setContentSize(m_groupContainer->getContentSize());
	groupBg->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
	groupBg->setPosition(Point::ZERO);
	m_groupContainer->addChild(groupBg);

	m_daysContainer = Node::create();
	m_daysContainer->setIgnoreAnchorPointForPosition(false);
	m_daysContainer->setAnchorPoint(Point::ANCHOR_MIDDLE_LEFT);
	m_daysContainer->setContentSize(Size(m_groupContainer->getContentSize().width / 2, m_groupContainer->getContentSize().height));
	m_groupContainer->addChild(m_daysContainer);

	m_daysTitleLabel = Label::createWithSystemFont("Day 7", DEFAULT_SYSTEM_FONT, 13);
	m_daysTitleLabel->setAnchorPoint(Point::ANCHOR_MIDDLE_TOP);
	m_daysTitleLabel->setTextColor(Color4B(128, 109, 66, 255));
	Utils::enableBoldForLabel(m_daysTitleLabel);
	m_daysContainer->addChild(m_daysTitleLabel);

	m_daysPicSp = Sprite::createWithSpriteFrameName(StringUtils::format(DIALOG_PIC_HERO_FRAMENAME_FORMAT, HeroID::HERO_SANTA));
	m_daysPicSp->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
	m_daysContainer->addChild(m_daysPicSp);

	m_daysCaptionLabel = Label::createWithSystemFont("Stana", DEFAULT_SYSTEM_FONT, 20);
	m_daysCaptionLabel->setAnchorPoint(Point::ANCHOR_MIDDLE_LEFT);
	m_daysCaptionLabel->setTextColor(Color4B(128, 109, 66, 255));
	Utils::enableBoldForLabel(m_daysCaptionLabel);
	m_daysContainer->addChild(m_daysCaptionLabel);

	m_dividerSp = Sprite::createWithSpriteFrameName(FRAMENAME_DIALOG_GROUP_DIVIDER);
	m_dividerSp->setContentSize(Size(1, 36));
	m_dividerSp->setAnchorPoint(Point::ANCHOR_MIDDLE);
	m_dividerSp->setPosition(m_groupContainer->getContentSize().width / 2, m_groupContainer->getContentSize().height / 2);
	m_groupContainer->addChild(m_dividerSp);

	m_todayContainer = Node::create();
	m_todayContainer->setAnchorPoint(Point::ANCHOR_MIDDLE);
	m_todayContainer->setIgnoreAnchorPointForPosition(false);
	m_todayContainer->setContentSize(Size(m_groupContainer->getContentSize().width / 2, m_groupContainer->getContentSize().height));
	m_groupContainer->addChild(m_todayContainer);

	m_todayTitleLabel = Label::createWithSystemFont(sLocaleMgr->getString("func_dailyreward_dlg_today_title"), DEFAULT_SYSTEM_FONT, 13);
	m_todayTitleLabel->setAnchorPoint(Point::ANCHOR_MIDDLE_TOP);
	m_todayTitleLabel->setTextColor(Color4B(128, 109, 66, 255));
	Utils::enableBoldForLabel(m_todayTitleLabel);
	m_todayContainer->addChild(m_todayTitleLabel);

	m_todayPicSp = Sprite::createWithSpriteFrameName(DIALOG_PIC_GOLD_FRAMENAME);
	m_todayPicSp->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
	m_todayContainer->addChild(m_todayPicSp);

	m_todayCaptionLabel = Label::createWithSystemFont("100", DEFAULT_SYSTEM_FONT, 20);
	m_todayCaptionLabel->setAnchorPoint(Point::ANCHOR_MIDDLE_LEFT);
	m_todayCaptionLabel->setTextColor(Color4B(128, 109, 66, 255));
	Utils::enableBoldForLabel(m_todayCaptionLabel);
	m_todayContainer->addChild(m_todayCaptionLabel);

	m_messageLabel = Label::createWithSystemFont("MessageLabel", DEFAULT_SYSTEM_FONT, 14);
	m_messageLabel->setTextColor(Color4B(150, 129, 56, 255));
	m_messageLabel->setAlignment(TextHAlignment::CENTER, TextVAlignment::CENTER);
	Utils::enableBoldForLabel(m_messageLabel);
	m_messageLabel->setPosition(m_contentPanel->getContentSize().width / 2, m_contentPanel->getContentSize().height);
	m_messageLabel->setAnchorPoint(Point::ANCHOR_MIDDLE_TOP);
	m_messageLabel->setDimensions(m_contentPanel->getContentSize().width - MESSAGE_MARGIN_LEFTRIGHT * 2, m_contentPanel->getContentSize().height - m_groupContainer->getBoundingBox().getMaxY() - 8);
	m_contentPanel->addChild(m_messageLabel);
}

void DailyRewardDialog::layoutGroupElements()
{
	m_daysContainer->setPosition(m_groupContainer->getContentSize().width / 2, m_groupContainer->getContentSize().height / 2);
	m_daysTitleLabel->setPosition(m_daysContainer->getContentSize().width / 2, m_daysContainer->getContentSize().height - 7);
	m_daysPicSp->setPosition(0, 0);
	m_daysCaptionLabel->setPosition(m_daysPicSp->getBoundingBox().getMaxX(), 20);

	float daysCaptionMaxWidth = m_daysContainer->getContentSize().width - m_daysCaptionLabel->getPosition().x - CAPTION_LABEL_MARGIN_RIGHT;
	if (m_daysCaptionLabel->getContentSize().width > daysCaptionMaxWidth)
	{
		float scale = daysCaptionMaxWidth / m_daysCaptionLabel->getContentSize().width;
		m_daysCaptionLabel->setScale(scale);
	}

	m_todayPicSp->setPosition(0, 0);
	m_todayCaptionLabel->setPosition(m_todayPicSp->getBoundingBox().getMaxX(), 20);
	if (m_daysContainer->isVisible())
		m_todayContainer->setPosition((m_groupContainer->getContentSize().width - m_todayContainer->getContentSize().width) / 2, m_groupContainer->getContentSize().height / 2);
	else
		m_todayContainer->setPosition(m_groupContainer->getContentSize().width / 2, m_groupContainer->getContentSize().height / 2);
	m_todayTitleLabel->setPosition(m_todayContainer->getContentSize().width / 2, m_todayContainer->getContentSize().height - 7);

	float todayCaptionMaxWidth = m_todayContainer->getContentSize().width - m_todayCaptionLabel->getPosition().x - CAPTION_LABEL_MARGIN_RIGHT;
	if (m_todayCaptionLabel->getContentSize().width > todayCaptionMaxWidth)
	{
		float scale = todayCaptionMaxWidth / m_todayCaptionLabel->getContentSize().width;
		m_todayCaptionLabel->setScale(scale);
	}

}

void DailyRewardDialog::loadData()
{
	LocalPlayer* localPlayer = sGameCenter->getLocalPlayer();
	auto tmplList = sShopMgr->getDailyRewardTemplateList();
	int32 currDays = localPlayer->getDailyRewardDays();
	int32 numOfTmpl = (int32)tmplList->size();

	int32 tmplIndex = std::min(currDays, numOfTmpl - 1);
	auto const& todayTmpl = (*tmplList)[tmplIndex];
	if (todayTmpl.rewardType1 == DAILY_REWARD_TYPE_HERO && localPlayer->hasUnlockedHero(todayTmpl.value1))
	{
		m_todayRewardType = todayTmpl.rewardType2;
		m_todayValue = todayTmpl.value2;
	}
	else
	{
		m_todayRewardType = todayTmpl.rewardType1;
		m_todayValue = todayTmpl.value1;
	}
	this->setTodayReward(m_todayRewardType, m_todayValue);

	int32 milestoneTmplIndex = 0;
	for (int32 i = tmplIndex + 1; i < numOfTmpl; ++i)
	{
		auto const& tmpl = (*tmplList)[i];
		if (tmpl.isMilestone)
		{
			milestoneTmplIndex = i;
			break;
		}
	}
	if (milestoneTmplIndex > 0)
	{
		DailyRewardTemplate const& daysTmpl = (*tmplList)[milestoneTmplIndex];
		if (daysTmpl.rewardType1 == DAILY_REWARD_TYPE_HERO && localPlayer->hasUnlockedHero(daysTmpl.value1))
			this->setDaysReward(daysTmpl.day, daysTmpl.rewardType2, daysTmpl.value2);
		else
			this->setDaysReward(daysTmpl.day, daysTmpl.rewardType1, daysTmpl.value1);
	}
	else
		this->setDaysReward(0, DAILY_REWARD_TYPE_NONE, 0);

	if (currDays == 0)
		this->setMessage(sLocaleMgr->getString("func_dailyreward_dlg_msg_first_day"));
	else
		this->setMessage(StringUtils::format(sLocaleMgr->getString("func_dailyreward_dlg_msg_day_n").c_str(), currDays + 1));

	Vector<ui::Widget*> buttons;
	if (m_todayRewardType == DAILY_REWARD_TYPE_GOLD)
		buttons.pushBack(m_watchAdBtn);
	else
		m_claimBtn->loadTextureNormal(FRAMENAME_DIALOG_BTN_CLAIM_POSITIVE, Widget::TextureResType::PLIST);
	buttons.pushBack(m_claimBtn);
	this->setButtons(buttons);

	this->layoutGroupElements();
}

void DailyRewardDialog::setMessage(std::string const& message)
{
	m_messageLabel->setString(message);
}

void DailyRewardDialog::setTodayReward(DailyRewardType rewardType, int32 value)
{
	switch (rewardType)
	{
	case DAILY_REWARD_TYPE_HERO:
	{
		m_todayPicSp->setSpriteFrame(StringUtils::format(DIALOG_PIC_HERO_FRAMENAME_FORMAT, value));
		HeroTemplate const* tmpl = sShopMgr->getHeroTemplateByHeroId(value);
		m_todayCaptionLabel->setString(tmpl->name);
		break;
	}
	default:
		m_todayPicSp->setSpriteFrame(DIALOG_PIC_GOLD_FRAMENAME);
		m_todayCaptionLabel->setString(std::to_string(value));
		break;
	}

}

void DailyRewardDialog::setDaysReward(int32 days, DailyRewardType rewardType, int32 value)
{
	m_daysContainer->setVisible(true);
	m_daysTitleLabel->setVisible(true);
	m_dividerSp->setVisible(true);
	m_daysTitleLabel->setString(StringUtils::format(sLocaleMgr->getString("func_dailyreward_dlg_days_title").c_str(), days));
	switch (rewardType)
	{
	case DAILY_REWARD_TYPE_HERO:
	{
		m_daysPicSp->setSpriteFrame(StringUtils::format(DIALOG_PIC_HERO_FRAMENAME_FORMAT, value));
		HeroTemplate const* tmpl = sShopMgr->getHeroTemplateByHeroId(value);
		m_daysCaptionLabel->setString(tmpl->name);
		break;
	}
	case DAILY_REWARD_TYPE_GOLD:
		m_daysPicSp->setSpriteFrame(DIALOG_PIC_GOLD_FRAMENAME);
		m_daysCaptionLabel->setString(std::to_string(value));
		break;
	default:
		m_daysContainer->setVisible(false);
		m_daysTitleLabel->setVisible(false);
		m_dividerSp->setVisible(false);
		break;
	}
		
}

NS_END