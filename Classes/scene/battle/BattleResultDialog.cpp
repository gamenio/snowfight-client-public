#include "BattleResultDialog.h"

#include "../SoundMgr.h"
#include "scene/Utils.h"

using namespace cocos2d::ui;

NS_BEGIN

#define FRAME_SIZE							Size(275, 180)

#define BUTTON_PANEL_HEIGHT					36
#define BUTTON_PANEL_MARGIN_LEFTRIGHT       13
#define BUTTON_PANEL_MARGIN_BOTTOM			32

#define CONTENT_MARGIN_TOP					31
#define CONTENT_MARGIN_BOTTOM				24
#define CONTENT_MARGIN_LEFTRIGHT			13

#define GROUP_MARGIN_LEFTRIGHT				14
#define GROUP_HEIGHT						43

#define BUTTON_WIDTH_WATCH_AD				105
#define BUTTON_WIDTH_HOME_POS				105
#define BUTTON_WIDTH_HOME_NEG				84

#define FRAMENAME_FRAME_BG							"form_bg.png"
#define FRAMENAME_DIALOG_TITLE_BG_VICTORY			"battleret_dlg_title_bg_victory.png"
#define FRAMENAME_DIALOG_TITLE_BG_DEFEAT			"battleret_dlg_title_bg_defeat.png"
#define FRAMENAME_DIALOG_BTN_HOME_ICON				"battleret_dlg_btn_home_icon.png"
#define FRAMENAME_DIALOG_BTN_HOME_NEG				"btn_bg_negative.png"
#define FRAMENAME_DIALOG_BTN_HOME_POS				"btn_bg_positive.png"
#define FRAMENAME_DIALOG_BTN_WATCH_AD				"btn_bg_positive.png"
#define FRAMENAME_BTN_ICON_WATCH_AD					"btn_icon_watch_ad.png"
#define FRAMENAME_BTN_ICON_GOLD						"btn_icon_gold.png"
#define FRAMENAME_DIVIDER							"divider.png"
#define FRAMENAME_DIALOG_ICON_MONEY					"battleret_dlg_icon_money.png"
#define FRAMENAME_DIALOG_ICON_XP					"battleret_dlg_icon_xp.png"
#define FRAMENAME_DIALOG_GROUP_BG					"dlg_group_bg.png"

BattleResultDialog* BattleResultDialog::create()
{
	auto ret = new (std::nothrow) BattleResultDialog();
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

bool BattleResultDialog::init()
{
	if (!ModalDialog::init(FRAME_SIZE))
		return false;

	SpriteFrame* titleBgFrame = SpriteFrameCache::getInstance()->getSpriteFrameByName(FRAMENAME_DIALOG_TITLE_BG_VICTORY);
	Node* titleBar = Node::create();
	titleBar->setAnchorPoint(Point::ANCHOR_MIDDLE_BOTTOM);
	titleBar->setPosition(FRAME_SIZE.width / 2, FRAME_SIZE.height - 34);
	titleBar->setContentSize(titleBgFrame->getOriginalSize());
	this->setTitleBar(titleBar);

	m_titleBg = Scale9Sprite::createWithSpriteFrame(titleBgFrame);
	m_titleBg->setPosition(titleBar->getContentSize().width / 2, titleBar->getContentSize().height / 2);
	m_titleBg->setAnchorPoint(Point::ANCHOR_MIDDLE);
	titleBar->addChild(m_titleBg);

	m_titleLabel = Label::createWithSystemFont("title", DEFAULT_SYSTEM_FONT, 15);
	m_titleLabel->setPosition(titleBar->getContentSize().width / 2, 38);
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

BattleResultDialog::BattleResultDialog() :
	m_battleOutcome(BATTLE_VICTORY),
	m_isWatchAdButtonVisible(true),
	m_titleBg(nullptr),
	m_titleLabel(nullptr),
	m_titleNode(nullptr),
	m_contentPanel(nullptr),
	m_killCountLabel(nullptr),
	m_rankNoLabel(nullptr),
	m_groupContainer(nullptr),
	m_moneyIconSp(nullptr),
	m_moneyAmountLabel(nullptr),
	m_extraAmountLabel(nullptr),
	m_xpIconSp(nullptr),
	m_extraXPLabel(nullptr),
	m_fingerTap(nullptr),
	m_homeBtn(nullptr),
	m_watchAdBtn(nullptr)
{
}

BattleResultDialog::~BattleResultDialog()
{
	m_titleBg = nullptr;
	m_titleLabel = nullptr;
	m_titleNode = nullptr;
	m_contentPanel = nullptr;
	m_killCountLabel = nullptr;
	m_rankNoLabel = nullptr;
	m_groupContainer = nullptr;
	m_moneyIconSp = nullptr;
	m_moneyAmountLabel = nullptr;
	m_extraAmountLabel = nullptr;
	m_xpIconSp = nullptr;
	m_extraXPLabel = nullptr;
	m_fingerTap = nullptr;

	CC_SAFE_RELEASE_NULL(m_homeBtn);
	CC_SAFE_RELEASE_NULL(m_watchAdBtn);
}

void BattleResultDialog::setRankNo(int32 no)
{
	m_rankNoLabel->setString(std::to_string(no));
}

void BattleResultDialog::setKillCount(int32 count)
{
	m_killCountLabel->setString(std::to_string(count));
}

void BattleResultDialog::setBattleOutcome(BattleOutcome outcome)
{
	m_battleOutcome = outcome;
	switch (outcome)
	{
	case BATTLE_VICTORY:
		m_titleLabel->setString(sLocaleMgr->getString("battle_ret_dlg_title_victory"));
		m_titleBg->setSpriteFrame(FRAMENAME_DIALOG_TITLE_BG_VICTORY);
		m_titleLabel->setTextColor(Color4B(237, 232, 239, 255));
		break;
	case BATTLE_DEFEAT:
		m_titleLabel->setString(sLocaleMgr->getString("battle_ret_dlg_title_defeat"));
		m_titleBg->setSpriteFrame(FRAMENAME_DIALOG_TITLE_BG_DEFEAT);
		m_titleLabel->setTextColor(Color4B(237, 234, 232, 255));
		break;
	default:
		break;
	}
	Utils::enableBoldForLabel(m_titleLabel);
}

void BattleResultDialog::setMoney(int32 amount, int32 extraAmount)
{
	if (extraAmount > 0)
	{
		m_extraAmountLabel->setString(Utils::separateThousands(extraAmount, true));
		m_extraAmountLabel->setVisible(true);
	}
	else
		m_extraAmountLabel->setVisible(false);

	if (amount > 0 || extraAmount <= 0)
	{
		m_moneyAmountLabel->setVisible(true);
		m_moneyAmountLabel->setString(Utils::separateThousands(amount, false));
	}
	else
		m_moneyAmountLabel->setVisible(false);
}

void BattleResultDialog::setExtraXP(int32 xp)
{
	if (xp > 0)
	{
		m_extraXPLabel->setString(Utils::separateThousands(xp, true));
		m_extraXPLabel->setVisible(true);
		m_xpIconSp->setVisible(true);
	}
	else
	{
		m_xpIconSp->setVisible(false);
		m_extraXPLabel->setVisible(false);
	}
}

void BattleResultDialog::setShowFingerTap(bool isShown)
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

void BattleResultDialog::setWatchAdButtonVisible(bool isVisbile)
{
	m_isWatchAdButtonVisible = isVisbile;
}

void BattleResultDialog::onEnter()
{
	ModalDialog::onEnter();

	Vector<ui::Widget*> buttons;
	if (m_isWatchAdButtonVisible)
		buttons.pushBack(m_watchAdBtn);
	else
	{
		m_homeBtn->setBackgroundImage(FRAMENAME_DIALOG_BTN_HOME_POS);
		m_homeBtn->setContentSize(Size(BUTTON_WIDTH_HOME_POS, BUTTON_PANEL_HEIGHT));
	}
		
	buttons.pushBack(m_homeBtn);

	this->setButtons(buttons);
	this->layoutGroupElements();

	switch (m_battleOutcome)
	{
	case BATTLE_VICTORY:
		sSoundMgr->play(SOUND_BATTLE_VICTORY);
		break;
	case BATTLE_DEFEAT:
		sSoundMgr->play(SOUND_BATTLE_DEFEAT);
		break;
	default:
		break;
	}
}

void BattleResultDialog::onAppeared()
{
	if (m_fingerTap)
	{
		Point screenPos = m_homeBtn->convertToWorldSpace(Vec2(m_homeBtn->getContentSize().width - 25, m_homeBtn->getContentSize().height / 2));
		m_fingerTap->show(this->convertToNodeSpace(screenPos));
	}
}

void BattleResultDialog::onWillDisappear()
{
	if (m_fingerTap)
		m_fingerTap->hide();
}

void BattleResultDialog::initButtons()
{
	m_homeBtn = IconButton::create(FRAMENAME_DIALOG_BTN_HOME_NEG, "", FRAMENAME_DIALOG_BTN_HOME_ICON);
	m_homeBtn->setContentSize(Size(BUTTON_WIDTH_HOME_NEG, BUTTON_PANEL_HEIGHT));
	m_homeBtn->addClickEventListener([this](Ref* sender) {
		if (m_homeBtnCallback)
			m_homeBtnCallback(this);
		this->dismiss();
		sSoundMgr->play(SOUND_BUTTON);
	});
	CC_SAFE_RETAIN(m_homeBtn);

	m_watchAdBtn = IconButton::create(FRAMENAME_DIALOG_BTN_WATCH_AD, sLocaleMgr->getString("func_dailyreward_dlg_btn_watch_ad"), FRAMENAME_BTN_ICON_GOLD);
	m_watchAdBtn->setContentSize(Size(BUTTON_WIDTH_WATCH_AD, BUTTON_PANEL_HEIGHT));
	m_watchAdBtn->addClickEventListener([this](Ref* sender) {
		if (m_watchAdBtnCallback)
			m_watchAdBtnCallback(this);
		this->dismiss();
		sSoundMgr->play(SOUND_BUTTON);
	});
	m_watchAdBtn->setTitleFontSize(12);
	m_watchAdBtn->setTitleColor(Color4B(243, 244, 237, 255));
	m_watchAdBtn->setBackgroundIconImage(FRAMENAME_BTN_ICON_WATCH_AD);
	CC_SAFE_RETAIN(m_watchAdBtn);
}


void BattleResultDialog::initContents()
{
	Label* rankTitle = Label::createWithSystemFont(sLocaleMgr->getString("battle_ret_dlg_content_rank"), DEFAULT_SYSTEM_FONT, 13);
	rankTitle->setTextColor(Color4B(150, 129, 56, 255));
	Utils::enableBoldForLabel(rankTitle);
	rankTitle->setPosition(m_contentPanel->getContentSize().width / 4, m_contentPanel->getContentSize().height - 4);
	rankTitle->setAnchorPoint(Point::ANCHOR_MIDDLE_TOP);
	m_contentPanel->addChild(rankTitle);

	m_rankNoLabel = Label::createWithSystemFont("0", DEFAULT_SYSTEM_FONT, 20);
	m_rankNoLabel->setTextColor(Color4B(150, 129, 56, 255));
	Utils::enableBoldForLabel(m_rankNoLabel);
	m_rankNoLabel->setPosition(rankTitle->getBoundingBox().getMidX(), rankTitle->getBoundingBox().getMinY() - 2);
	m_rankNoLabel->setAnchorPoint(Point::ANCHOR_MIDDLE_TOP);
	m_contentPanel->addChild(m_rankNoLabel);

	Label* killCountTitle = Label::createWithSystemFont(sLocaleMgr->getString("battle_ret_dlg_content_killcount"), DEFAULT_SYSTEM_FONT, 13);
	killCountTitle->setTextColor(Color4B(150, 129, 56, 255));
	Utils::enableBoldForLabel(killCountTitle);
	killCountTitle->setPosition(m_contentPanel->getContentSize().width - m_contentPanel->getContentSize().width / 4, m_contentPanel->getContentSize().height - 4);
	killCountTitle->setAnchorPoint(Point::ANCHOR_MIDDLE_TOP);
	m_contentPanel->addChild(killCountTitle);

	m_killCountLabel = Label::createWithSystemFont("0", DEFAULT_SYSTEM_FONT, 20);
	m_killCountLabel->setTextColor(Color4B(150, 129, 56, 255));
	Utils::enableBoldForLabel(m_killCountLabel);
	m_killCountLabel->setPosition(killCountTitle->getBoundingBox().getMidX(), killCountTitle->getBoundingBox().getMinY() - 2);
	m_killCountLabel->setAnchorPoint(Point::ANCHOR_MIDDLE_TOP);
	m_contentPanel->addChild(m_killCountLabel);

	Sprite* dividerSp = Sprite::createWithSpriteFrameName(FRAMENAME_DIVIDER);
	dividerSp->setContentSize(Size(1, 27));
	dividerSp->setAnchorPoint(Point::ANCHOR_MIDDLE_TOP);
	dividerSp->setPosition(m_contentPanel->getContentSize().width / 2, m_contentPanel->getContentSize().height - 10);
	m_contentPanel->addChild(dividerSp);

	m_groupContainer = Node::create();
	m_groupContainer->setContentSize(Size(m_contentPanel->getContentSize().width - GROUP_MARGIN_LEFTRIGHT * 2, GROUP_HEIGHT));
	m_groupContainer->setAnchorPoint(Point::ANCHOR_TOP_LEFT);
	m_groupContainer->setPosition(GROUP_MARGIN_LEFTRIGHT, dividerSp->getBoundingBox().getMinY() - 14);
	m_contentPanel->addChild(m_groupContainer);

	Scale9Sprite* groupBg = Scale9Sprite::createWithSpriteFrameName(FRAMENAME_DIALOG_GROUP_BG);
	groupBg->setContentSize(m_groupContainer->getContentSize());
	groupBg->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
	groupBg->setPosition(Point::ZERO);
	m_groupContainer->addChild(groupBg);

	m_moneyIconSp = Sprite::createWithSpriteFrameName(FRAMENAME_DIALOG_ICON_MONEY);
	m_moneyIconSp->setAnchorPoint(Point::ANCHOR_TOP_LEFT);
	m_groupContainer->addChild(m_moneyIconSp);

	m_moneyAmountLabel = Label::createWithSystemFont("0", DEFAULT_SYSTEM_FONT, 13);
	m_moneyAmountLabel->setTextColor(Color4B(128, 109, 66, 255));
	Utils::enableBoldForLabel(m_moneyAmountLabel);
	m_moneyAmountLabel->setAnchorPoint(Point::ANCHOR_MIDDLE_LEFT);
	m_groupContainer->addChild(m_moneyAmountLabel);

	m_extraAmountLabel = Label::createWithSystemFont("0", DEFAULT_SYSTEM_FONT, 13);
	m_extraAmountLabel->setTextColor(Color4B(181, 86, 63, 255));
	Utils::enableBoldForLabel(m_extraAmountLabel);
	m_extraAmountLabel->setAnchorPoint(Point::ANCHOR_MIDDLE_LEFT);
	m_extraAmountLabel->setVisible(false);
	m_groupContainer->addChild(m_extraAmountLabel);

	m_extraXPLabel = Label::createWithSystemFont("0", DEFAULT_SYSTEM_FONT, 13);
	m_extraXPLabel->setTextColor(Color4B(181, 86, 63, 255));
	Utils::enableBoldForLabel(m_extraXPLabel);
	m_extraXPLabel->setAnchorPoint(Point::ANCHOR_MIDDLE_RIGHT);
	m_extraXPLabel->setVisible(false);
	m_groupContainer->addChild(m_extraXPLabel);

	m_xpIconSp = Sprite::createWithSpriteFrameName(FRAMENAME_DIALOG_ICON_XP);
	m_xpIconSp->setAnchorPoint(Point::ANCHOR_TOP_RIGHT);
	m_xpIconSp->setVisible(false);
	m_groupContainer->addChild(m_xpIconSp);
}


void BattleResultDialog::layoutGroupElements()
{
	const float MONEY_AMOUNT_LABEL_SPACING = 3;
	const float MARGIN_LEFTRIGHT_MIN = 12;

	float maxFieldWidth;
	if (m_extraXPLabel->isVisible())
		maxFieldWidth = m_groupContainer->getContentSize().width / 2;
	else
		maxFieldWidth = m_groupContainer->getContentSize().width;

	float moneyFieldWidth = m_moneyIconSp->getContentSize().width;
	if(m_moneyAmountLabel->isVisible())
		moneyFieldWidth += m_moneyAmountLabel->getContentSize().width;
	if(m_extraAmountLabel->isVisible())
		moneyFieldWidth += MONEY_AMOUNT_LABEL_SPACING + m_extraAmountLabel->getContentSize().width;
	float moneyFieldLeft = std::max(MARGIN_LEFTRIGHT_MIN, (maxFieldWidth - moneyFieldWidth) / 2);

	float top = m_groupContainer->getContentSize().height - 8;
	float nextX = 0;
	m_moneyIconSp->setPosition(moneyFieldLeft, top);
	nextX = m_moneyIconSp->getBoundingBox().getMaxX();
	if (m_moneyAmountLabel->isVisible())
	{
		m_moneyAmountLabel->setPosition(nextX, m_moneyIconSp->getBoundingBox().getMidY() + 1);
		nextX = m_moneyAmountLabel->getBoundingBox().getMaxX() + MONEY_AMOUNT_LABEL_SPACING;
	}
	m_extraAmountLabel->setPosition(nextX, m_moneyIconSp->getBoundingBox().getMidY() + 1);

	if (m_extraXPLabel->isVisible())
	{
		float xpFieldWidth = m_xpIconSp->getContentSize().width + m_extraXPLabel->getContentSize().width;
		float xpFieldRight = m_groupContainer->getContentSize().width - std::max(MARGIN_LEFTRIGHT_MIN, (maxFieldWidth - xpFieldWidth) / 2);

		m_extraXPLabel->setPosition(xpFieldRight, m_moneyIconSp->getBoundingBox().getMidY() + 1);
		m_xpIconSp->setPosition(m_extraXPLabel->getBoundingBox().getMinX(), top);
	}
}


NS_END