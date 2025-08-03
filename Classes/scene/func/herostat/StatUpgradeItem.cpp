#include "StatUpgradeItem.h"

#include "scene/Utils.h"
#include "game/gamecenter/GameCenter.h"
#include "game/LocaleMgr.h"

NS_BEGIN

USING_NS_CC;
using namespace cocos2d::ui;

#define ITEM_MARGIN_LEFTRIGHT		0

#define FRAMENAME_ITEM_BTN_UPGRADE				"statupgrade_item_btn_upgrade.png"
#define FRAMENAME_ITEM_LOCK						"statupgrade_item_lock.png"
#define FRAMENAME_ITEM_COIN						"statupgrade_item_coin.png"
#define FRAMENAME_ITEM_DIVIDER					"statupgrade_item_divider.png"

#define STAT_ICON_FRAME_FORMAT			"statupgrade_item_icon%d.png"

#define ACTION_TAG_RECOMMEND			0

StatUpgradeItem::StatUpgradeItem() :
	m_iconSp(nullptr),
	m_name1Label(nullptr),
	m_value1Label(nullptr),
	m_name2Label(nullptr),
	m_value2Label(nullptr),
	m_introTextContainer(nullptr),
	m_stageLabel(nullptr),
	m_dividerSp(nullptr),
	m_upgradeGroup(nullptr),
	m_coinSp(nullptr),
	m_priceLabel(nullptr),
	m_upgradeBtn(nullptr),
	m_lockSp(nullptr)
{
}

StatUpgradeItem::~StatUpgradeItem()
{
	m_iconSp = nullptr;
	m_name1Label = nullptr;
	m_value1Label = nullptr;
	m_name2Label = nullptr;
	m_value2Label = nullptr;
	m_introTextContainer = nullptr;
	m_stageLabel = nullptr;
	m_dividerSp = nullptr;
	m_upgradeGroup = nullptr;
	m_coinSp = nullptr;
	m_priceLabel = nullptr;
	m_upgradeBtn = nullptr;
	m_lockSp = nullptr;
}

StatUpgradeItem* StatUpgradeItem::create(Size const& size)
{
	StatUpgradeItem* ret = new (std::nothrow) StatUpgradeItem();
	if (ret && ret->init(size))
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

bool StatUpgradeItem::init(Size const& size)
{
	if (!Widget::init())
		return false;

	this->setContentSize(size);

	m_dividerSp = Sprite::createWithSpriteFrameName(FRAMENAME_ITEM_DIVIDER);
	m_dividerSp->setPosition(Vec2(size.width / 2, 0));
	m_dividerSp->setAnchorPoint(Point::ANCHOR_MIDDLE_BOTTOM);
	m_dividerSp->setContentSize(Size(size.width - ITEM_MARGIN_LEFTRIGHT * 2, 1));
	this->addChild(m_dividerSp);

	m_iconSp = Sprite::createWithSpriteFrameName(StringUtils::format(STAT_ICON_FRAME_FORMAT, StatUpgradeType::STAT_UPGRADE_HEALTH));
	m_iconSp->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
	m_iconSp->setPosition(Vec2(ITEM_MARGIN_LEFTRIGHT, 4));
	this->addChild(m_iconSp);

	m_name1Label = Label::createWithSystemFont("Name1", DEFAULT_SYSTEM_FONT, 8);
	m_name1Label->setTextColor(Color4B(86, 111, 132, 255));
	m_name1Label->setPosition(Vec2(m_iconSp->getBoundingBox().getMaxX() + 4, m_iconSp->getBoundingBox().getMaxY() - 1));
	m_name1Label->setAnchorPoint(Vec2::ANCHOR_TOP_LEFT);
	Utils::enableBoldForLabel(m_name1Label);
	this->addChild(m_name1Label);

	m_stageLabel = Label::createWithSystemFont("0/0", DEFAULT_SYSTEM_FONT, 8);
	m_stageLabel->setTextColor(Color4B(86, 111, 132, 255));
	m_stageLabel->setPosition(Vec2(size.width - 58, m_name1Label->getBoundingBox().getMaxY()));
	m_stageLabel->setAnchorPoint(Vec2::ANCHOR_TOP_RIGHT);
	Utils::enableBoldForLabel(m_stageLabel);
	this->addChild(m_stageLabel);

	m_introTextContainer = LayerColor::create(Color4B(0, 0, 0, 0));
	m_introTextContainer->setIgnoreAnchorPointForPosition(false);
	m_introTextContainer->setPosition(Vec2(m_name1Label->getBoundingBox().getMinX(), m_name1Label->getBoundingBox().getMinY() - 4));
	m_introTextContainer->setAnchorPoint(Vec2::ANCHOR_TOP_LEFT);
	this->addChild(m_introTextContainer);

	m_value1Label = this->addIntroText("+9999", Color4B(10, 145, 0, 255), 8);
	m_name2Label = this->addIntroText(", name2 ", Color4B(86, 111, 132, 255), 7);
	m_value2Label = this->addIntroText("+100%", Color4B(219, 86, 64, 255), 7);

	m_upgradeGroup = Widget::create();
	m_upgradeGroup->setTouchEnabled(true);
	m_upgradeGroup->addTouchEventListener(CC_CALLBACK_2(StatUpgradeItem::onTouchUpgradeGroup, this));
	m_upgradeGroup->setAnchorPoint(Point::ANCHOR_MIDDLE_RIGHT);
	m_upgradeGroup->setPosition(Vec2(size.width - ITEM_MARGIN_LEFTRIGHT, size.height / 2));
	m_upgradeGroup->setContentSize(Size(47, size.height));
	this->addChild(m_upgradeGroup);

	m_coinSp = Sprite::createWithSpriteFrameName(FRAMENAME_ITEM_COIN);
	m_coinSp->setPosition(Vec2(3, m_upgradeGroup->getContentSize().height - 4));
	m_coinSp->setAnchorPoint(Point::ANCHOR_TOP_LEFT);
	m_upgradeGroup->addChild(m_coinSp);

	m_priceLabel = Label::createWithSystemFont("999", DEFAULT_SYSTEM_FONT, 8);
	m_priceLabel->setTextColor(Color4B(86, 111, 132, 255));
	m_priceLabel->setPosition(Vec2(m_coinSp->getBoundingBox().getMaxX() + 1, m_coinSp->getBoundingBox().getMidY()));
	m_priceLabel->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
	Utils::enableBoldForLabel(m_priceLabel);
	m_upgradeGroup->addChild(m_priceLabel);

	m_upgradeBtn = Button::create(FRAMENAME_ITEM_BTN_UPGRADE, "", "", Widget::TextureResType::PLIST);
	m_upgradeBtn->setContentSize(Size(m_upgradeGroup->getContentSize().width, 12));
	m_upgradeBtn->setScale9Enabled(true);
	m_upgradeBtn->setTitleText(sLocaleMgr->getString("statupgrade_item_btn_upgrade"));
	m_upgradeBtn->setPosition(Vec2(m_upgradeGroup->getContentSize().width / 2, m_upgradeBtn->getContentSize().height / 2 + 4));
	m_upgradeBtn->setTitleFontSize(7);
	m_upgradeBtn->setTitleColor(Color3B(223, 225, 229));
	Utils::enableBoldForLabel(m_upgradeBtn->getTitleRenderer());
	m_upgradeBtn->setAnchorPoint(Point::ANCHOR_MIDDLE);
	m_upgradeGroup->addChild(m_upgradeBtn);

	m_lockSp = Sprite::createWithSpriteFrameName(FRAMENAME_ITEM_LOCK);
	m_lockSp->setPosition(Vec2(m_upgradeGroup->getBoundingBox().getMidX(), m_upgradeGroup->getBoundingBox().getMidY()));
	m_lockSp->setAnchorPoint(Point::ANCHOR_MIDDLE);
	m_lockSp->setVisible(false);
	this->addChild(m_lockSp);

	return true;
}

void StatUpgradeItem::setEnabled(bool enabled)
{
	Widget::setEnabled(enabled);
	m_upgradeBtn->setEnabled(enabled);
}

void StatUpgradeItem::setLastItem(bool isLastItem)
{
	m_dividerSp->setVisible(!isLastItem);
}

void StatUpgradeItem::playRecommendAnimation()
{
	if (m_upgradeBtn->getActionByTag(ACTION_TAG_RECOMMEND))
		return;

	auto zoomInOut = Sequence::create(EaseSineOut::create(ScaleTo::create(0.2f, 1.2f)), ScaleTo::create(0.05f, 1.0f), nullptr);
	auto repeat = Repeat::create(zoomInOut, 3);
	auto repeatForever = Repeat::create(Sequence::create(repeat, DelayTime::create(2.f), nullptr), pow(2, 30));
	auto action = Sequence::create(DelayTime::create(1.0f), repeatForever, nullptr);
	action->setTag(ACTION_TAG_RECOMMEND);
	m_upgradeBtn->runAction(action);
}

void StatUpgradeItem::initRecommendAnimation()
{
	if (Action* action = m_upgradeBtn->getActionByTag(ACTION_TAG_RECOMMEND))
		m_upgradeBtn->stopAction(action);

	m_upgradeBtn->setScale(1.0f);
}

void StatUpgradeItem::setData(StatUpgradeData const& data)
{
	m_data = data;
	this->reloadData();
}


void StatUpgradeItem::stopAnimation()
{
	if (Action* action = m_upgradeBtn->getActionByTag(ACTION_TAG_RECOMMEND))
		m_upgradeBtn->stopAction(action);
}

void StatUpgradeItem::reset()
{
	this->stopAnimation();
	this->setLastItem(false);
}

void StatUpgradeItem::onEnter()
{
	Widget::onEnter();
}

Label* StatUpgradeItem::addIntroText(std::string const& text, Color4B const& color, float fontSize, int32 tag)
{
	Label* label = Label::createWithSystemFont(text, DEFAULT_SYSTEM_FONT, fontSize);
	label->setTextColor(color);
	label->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
	Utils::enableBoldForLabel(label);
	if (tag != Node::INVALID_TAG)
		label->setTag(tag);
	m_introTextContainer->addChild(label);

	return label;
}

void StatUpgradeItem::arrangeIntroTextElements()
{
	float maxHeight = 0;
	float nextX = 0;
	auto const& children = m_introTextContainer->getChildren();
	for (auto it = children.begin(); it != children.end(); ++it)
	{
		Node* label = *it;
		if (!label->isVisible())
			continue;
		maxHeight = MAX(maxHeight, label->getBoundingBox().size.height);
		label->setPosition(nextX, 0);
		nextX = label->getBoundingBox().getMaxX();
	}

	m_introTextContainer->setContentSize(Size(nextX, maxHeight));
}

void StatUpgradeItem::reloadData()
{
	m_iconSp->setSpriteFrame(StringUtils::format(STAT_ICON_FRAME_FORMAT, m_data.type));
	m_name1Label->setString(m_data.field1.name);

	m_value1Label->setVisible(false);
	m_name2Label->setVisible(false);
	m_value2Label->setVisible(false);
	m_upgradeGroup->setVisible(false);
	m_lockSp->setVisible(false);

	this->initRecommendAnimation();

	if (m_data.stage < m_data.maxStage)
	{
		m_value1Label->setVisible(true);
		if (m_data.field1.format == StatField::PERCENTAGE)
			m_value1Label->setString(Utils::convertToPercentage(m_data.field1.value.asFloat()));
		else
			m_value1Label->setString(StringUtils::format("%+d", m_data.field1.value.asInt()));

		if (!m_data.field2.value.isNull())
		{
			m_name2Label->setVisible(true);
			m_value2Label->setVisible(true);
			m_name2Label->setString(StringUtils::format(", %s ", m_data.field2.name.c_str()));
			if (m_data.field2.format == StatField::PERCENTAGE)
				m_value2Label->setString(Utils::convertToPercentage(m_data.field2.value.asFloat()));
			else
				m_value2Label->setString(StringUtils::format("%+d", m_data.field2.value.asInt()));
		}

		if (m_data.isLocked)
		{
			m_upgradeGroup->setVisible(false);
			m_lockSp->setVisible(true);
		}
		else
		{
			m_upgradeGroup->setVisible(true);
			m_lockSp->setVisible(false);

			if (m_data.isRecommended)
				this->playRecommendAnimation();
				
		}
	}
	else
	{
		m_value1Label->setString("");
		m_value2Label->setString("");

		m_name2Label->setVisible(true);
		m_name2Label->setString(sLocaleMgr->getString("statupgrade_item_fully_upgraded"));
	}

	m_stageLabel->setString(StringUtils::format("%d/%d", m_data.stage, m_data.maxStage));
	m_priceLabel->setString(Utils::separateThousands(m_data.price));

	this->arrangeIntroTextElements();
}

void StatUpgradeItem::onTouchUpgradeGroup(Ref* sender, Widget::TouchEventType eventType)
{
	m_upgradeBtn->setHighlighted(m_upgradeGroup->isHighlighted());
}


NS_END