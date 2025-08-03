#include "PropBubble.h"

#include <regex>

#include "game/ObjectMgr.h"
#include "game/LocaleMgr.h"
#include "game/utils/ItemDescriptionUtils.h"
#include "scene/Utils.h"
#include "scene/gamble/CarriedProp.h"


NS_BEGIN

#define FRAMENAME_BUBBLE_ARROW_DOWN				"battle_bubble_arrow_down.png"
#define FRAMENAME_BUBBLE_ICON_SHADOW			"battle_bubble_icon_shadow.png"
#define FRAMENAME_BUBBLE_BG						"battle_bubble_bg.png"

#define ACTION_TAG_BUBBLE				1
#define ACTION_TAG_HIDE_DELAYED			2

#define BUBBLE_MIN_WIDTH						69
#define BUBBLE_MIN_HEIGHT						60

#define TEXT_MARGIN_LEFT						40
#define TEXT_MARGIN_TOP							18
#define TEXT_MARGIN_RIGHT						23
#define TEXT_MARGIN_BOTTOM						23

#define INTRO_MARGIN_LEFT						1
#define INTRO_MARGIN_TOP						2
#define INTRO_MAX_LINE_WIDTH					208

#define ICON_MARGIN_LEFT						15
#define ICON_MARGIN_TOP							15
#define ICON_CONTENT_SIZE						Size(25, 25)

#define SILHOUETTE_EFFECT_OPACITY				255

PropBubble* PropBubble::create()
{
    auto ret = new (std::nothrow) PropBubble();
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

bool PropBubble::init()
{
    if (!Node::init())
        return false;

	this->setIgnoreAnchorPointForPosition(false);

	m_bubbleSp = SILScale9Sprite::createWithSpriteFrameName(FRAMENAME_BUBBLE_BG);
	m_bubbleSp->setVisible(false);
	m_bubbleSp->setEffectOpacity(SILHOUETTE_EFFECT_OPACITY);
	m_bubbleSp->setScale(0.f);
	this->addChild(m_bubbleSp);

	m_arrowSp = SILSprite::createWithSpriteFrameName(FRAMENAME_BUBBLE_ARROW_DOWN);
	m_arrowSp->setAnchorPoint(Point::ANCHOR_MIDDLE_BOTTOM);
	m_arrowSp->setEffectOpacity(SILHOUETTE_EFFECT_OPACITY);
	m_bubbleSp->addChild(m_arrowSp);

	m_iconShadowSp = SILSprite::createWithSpriteFrameName(FRAMENAME_BUBBLE_ICON_SHADOW);
	m_iconShadowSp->setAnchorPoint(Point::ANCHOR_MIDDLE);
	m_iconShadowSp->setEffectOpacity(SILHOUETTE_EFFECT_OPACITY);
	m_bubbleSp->addChild(m_iconShadowSp);

	m_iconSp = SILSprite::create();
	m_iconSp->setAnchorPoint(Point::ANCHOR_TOP_LEFT);
	m_iconSp->setEffectOpacity(SILHOUETTE_EFFECT_OPACITY);
	m_bubbleSp->addChild(m_iconSp);

	m_titleLabel = SILLabel::createWithSystemFont("title", DEFAULT_SYSTEM_FONT, 11);
	m_titleLabel->setAnchorPoint(Point::ANCHOR_TOP_LEFT);
	m_titleLabel->setEffectOpacity(SILHOUETTE_EFFECT_OPACITY);
	m_bubbleSp->addChild(m_titleLabel);

	m_introLabel = SILLabel::createWithSystemFont("introduction", DEFAULT_SYSTEM_FONT, 9);
	m_introLabel->setAnchorPoint(Point::ANCHOR_TOP_LEFT);
	m_introLabel->enableWrap(true);
	m_introLabel->setEffectOpacity(SILHOUETTE_EFFECT_OPACITY);
	m_bubbleSp->addChild(m_introLabel);

	this->layoutElements();

    return true;
}

PropBubble::PropBubble() :
	m_isShown(false),
	m_introLabel(nullptr),
	m_bubbleSp(nullptr),
	m_arrowSp(nullptr),
	m_iconSp(nullptr),
	m_iconShadowSp(nullptr)
{
}

PropBubble::~PropBubble()
{
	this->stopAnimation();

	m_introLabel = nullptr;
	m_bubbleSp = nullptr;
	m_arrowSp = nullptr;
	m_iconSp = nullptr;
	m_iconShadowSp = nullptr;
}

void PropBubble::show(ItemTemplate const* itemTemplate, PickupStatus status, float duration)
{
	if(m_isShown)
		this->hide();

	ItemLocale const* locale = sLocaleMgr->getItemLocale(itemTemplate->id);
	std::string name;
	if (locale)
		name = locale->name;
	else
		name = "NULL";

	if (itemTemplate->itemClass == ITEM_CLASS_EQUIPMENT)
	{
		if (itemTemplate->level == 0)
		{
			m_titleLabel->setTextColor(Color4B(10, 204, 0, 255));
			m_titleLabel->setString(name);
		}
		else
		{
			if (itemTemplate->level == 1)
				m_titleLabel->setTextColor(Color4B(250, 108, 0, 255));
			else if (itemTemplate->level == 2)
				m_titleLabel->setTextColor(Color4B(167, 179, 183, 255));
			else if (itemTemplate->level == 3)
				m_titleLabel->setTextColor(Color4B(247, 206, 0, 255));
			std::string levelStr = StringUtils::format(sLocaleMgr->getString("prop_title_level").c_str(), itemTemplate->level);
			m_titleLabel->setString(StringUtils::format("%s%s", name.c_str(), levelStr.c_str()));
		}
	}
	else
	{
		m_titleLabel->setTextColor(Color4B(225, 223, 232, 255));
		m_titleLabel->setString(name);
	}
	Utils::enableBoldForLabel(m_titleLabel);

	if (status == PICKUP_STATUS_OK)
	{
        std::string description;
        if (locale)
            description = ItemDescriptionUtils::format(itemTemplate, locale);
        else
            description = "NULL";
		if (itemTemplate->itemClass == ITEM_CLASS_CONSUMABLE)
			m_introLabel->setTextColor(Color4B(32, 189, 237, 255));
		else
			m_introLabel->setTextColor(Color4B(225, 223, 232, 255));
		m_introLabel->setString(description);
	}
	else
	{
		m_introLabel->setTextColor(Color4B(224, 41, 27, 255));
		m_introLabel->setString(this->getItemPickupErrorMessage(status));
	}
	Utils::enableBoldForLabel(m_introLabel);

	SpriteFrame* frame = SpriteFrameCache::getInstance()->getSpriteFrameByName(StringUtils::format(CARRIED_PROP_FRAME_FORMAT, itemTemplate->displayId));
	if (frame)
		m_iconSp->setSpriteFrame(frame);
	else
		m_iconSp->setTexture(nullptr);
	m_iconSp->setContentSize(ICON_CONTENT_SIZE);

	this->layoutElements();
	this->startPopAnimation(duration);

	m_isShown = true;
}

void PropBubble::hide(bool animated)
{
	if (!m_isShown)
		return;

	if (animated)
		this->startPushAnimation();
	else
	{
		this->stopAnimation();
		m_bubbleSp->setScale(0.f);
		m_bubbleSp->setVisible(false);
	}

	m_isShown = false;
}

void PropBubble::setGlobalZOrder(float globalZOrder)
{
	Node::setGlobalZOrder(globalZOrder);

	m_titleLabel->setGlobalZOrder(globalZOrder);
	m_introLabel->setGlobalZOrder(globalZOrder);
	m_bubbleSp->setGlobalZOrder(globalZOrder);
	m_arrowSp->setGlobalZOrder(globalZOrder);
	m_iconSp->setGlobalZOrder(globalZOrder);
	m_iconShadowSp->setGlobalZOrder(globalZOrder);
}

void PropBubble::startPopAnimation(float duration)
{
	this->stopAnimation();

	Vector<FiniteTimeAction*> actions;
	float openingTime = 0.f;

	actions.pushBack(Show::create());
	actions.pushBack(EaseBackOut::create(ScaleTo::create(0.3f, 1.0f)));
	for (FiniteTimeAction* action : actions)
		openingTime += action->getDuration();

	actions.pushBack(DelayTime::create(1.0f));
	auto move = MoveBy::create(0.9f, Vec2(0, 4));
	auto repeatMove = Repeat::create(Sequence::createWithTwoActions(move, move->reverse()), std::pow(2, 30));
	actions.pushBack(repeatMove);
	auto seq = Sequence::create(actions);
	seq->setTag(ACTION_TAG_BUBBLE);
	m_bubbleSp->runAction(seq);

	if (duration > 0)
	{
		actions.clear();
		actions.pushBack(DelayTime::create(openingTime + duration));
		CallFunc* finishCallFunc = CallFunc::create([this]() {
			this->hide();
		});
		actions.pushBack(finishCallFunc);
		auto seq = Sequence::create(actions);
		seq->setTag(ACTION_TAG_HIDE_DELAYED);
		m_bubbleSp->runAction(seq);
	}
}

void PropBubble::startPushAnimation()
{
	this->stopAnimation();

	Vector<FiniteTimeAction*> actions;
	actions.pushBack(EaseCubicActionIn::create(ScaleTo::create(0.3f, 0.0f)));
	actions.pushBack(Hide::create());

	auto seq = Sequence::create(actions);
	seq->setTag(ACTION_TAG_BUBBLE);
	m_bubbleSp->runAction(seq);
}

void PropBubble::stopAnimation()
{
	if (Action* action = m_bubbleSp->getActionByTag(ACTION_TAG_BUBBLE))
		m_bubbleSp->stopAction(action);

	if (Action* action = m_bubbleSp->getActionByTag(ACTION_TAG_HIDE_DELAYED))
		m_bubbleSp->stopAction(action);
}

void PropBubble::layoutElements()
{
	m_introLabel->setDimensions(0, 0);

	float titleWidth = m_titleLabel->getContentSize().width;
	float titleHeight = m_titleLabel->getContentSize().height;
	float introWidth = m_introLabel->getContentSize().width;
	float introHeight = m_introLabel->getContentSize().height;

	float introMaxWidth = std::max(titleWidth, (float)INTRO_MAX_LINE_WIDTH);
	if (introWidth > introMaxWidth)
	{
		m_introLabel->setWidth(introMaxWidth);
		introWidth = m_introLabel->getContentSize().width;
		introHeight = m_introLabel->getContentSize().height;
	}

	float bubbleWidth = TEXT_MARGIN_LEFT + std::max(titleWidth, introWidth) + TEXT_MARGIN_RIGHT;
	float bubbleHeight = TEXT_MARGIN_TOP + titleHeight + INTRO_MARGIN_TOP + introHeight + TEXT_MARGIN_BOTTOM;
	bubbleWidth = MAX(bubbleWidth, BUBBLE_MIN_WIDTH);
	bubbleHeight = MAX(bubbleHeight, BUBBLE_MIN_HEIGHT);
	float arrowPos = bubbleWidth / 2;

	m_bubbleSp->setContentSize(Size(bubbleWidth, bubbleHeight));
	m_bubbleSp->setAnchorPoint(Vec2(arrowPos / bubbleWidth, 0));
	m_bubbleSp->setPosition(Vec2(arrowPos, 0));
	m_arrowSp->setPosition(Vec2(arrowPos, 0));

	m_iconSp->setPosition(Vec2(ICON_MARGIN_LEFT, bubbleHeight - ICON_MARGIN_TOP));
	m_iconShadowSp->setPosition(m_iconSp->getBoundingBox().getMidX(), m_iconSp->getBoundingBox().getMinY() - 2);
	m_titleLabel->setPosition(m_iconSp->getBoundingBox().getMaxX() + INTRO_MARGIN_LEFT, m_bubbleSp->getContentSize().height - TEXT_MARGIN_TOP);
	m_introLabel->setPosition(m_titleLabel->getBoundingBox().getMinX(), m_titleLabel->getBoundingBox().getMinY() - INTRO_MARGIN_TOP);

	this->setContentSize(m_bubbleSp->getContentSize());
}

std::string PropBubble::getItemPickupErrorMessage(uint32 errorCode)
{
	std::string msg;
	switch (errorCode)
	{
	case PICKUP_STATUS_ITEM_CANT_STACK:
		msg = sLocaleMgr->getString("pickup_status_item_cant_stack");
		break;
	case PICKUP_STATUS_ITEM_STACK_LIMIT_EXCEEDED:
		msg = sLocaleMgr->getString("pickup_status_item_stack_limit_exceeded");
		break;
	case PICKUP_STATUS_INVENTORY_CUSTOM_SLOTS_FULL:
		msg = sLocaleMgr->getString("pickup_status_inventory_custom_slots_full");
		break;
	case PICKUP_STATUS_ITEM_IS_EQUIPPED:
		msg = sLocaleMgr->getString("pickup_status_item_is_equipped");
		break;
	case PICKUP_STATUS_LEVEL_LOWER_THAN_EXISTING_EQUIP:
		msg = sLocaleMgr->getString("pickup_status_level_lower_than_existing_equip");
		break;
	case PICKUP_STATUS_FORBIDDEN:
		msg = sLocaleMgr->getString("pickup_status_forbidden");
		break;
	default:
		break;
	}
	return msg;
}

NS_END
