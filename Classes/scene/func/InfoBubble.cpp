//
//  InfoBubble.cpp
//  snowfight
//
//  Created by Luthier on 2020/8/27.
//

#include "InfoBubble.h"

#include "scene/Utils.h"

using namespace cocos2d::ui;

NS_BEGIN

#define FRAMENAME_INFO_BUBBLE_ARROW_UP					"info_bubble_arrow_up.png"
#define FRAMENAME_INFO_BUBBLE_ARROW_DOWN				"info_bubble_arrow_down.png"
#define FRAMENAME_INFO_BUBBLE_ICON						"info_bubble_icon.png"
#define FRAMENAME_INFO_BUBBLE_BG						"info_bubble_bg.png"

#define ACTION_TAG_BUBBLE				1

#define BUBBLE_MIN_WIDTH						67
#define BUBBLE_MIN_HEIGHT						40

#define INFO_MARGIN_LEFT						30
#define INFO_MARGIN_RIGHT						12
#define INFO_SINGLE_LINE_MARGIN_TOP				13
#define INFO_MULTI_LINE_MARGIN_TOP				12
#define INFO_MARGIN_BOTTOM						14
#define INFO_MAX_LINE_WIDTH						106

InfoBubble* InfoBubble::create(ArrowDirection direction)
{
    auto ret = new (std::nothrow) InfoBubble();
    if (ret && ret->init(direction))
    {
        ret->autorelease();
    }
    else
    {
        CC_SAFE_DELETE(ret);
    }
    
    return ret;
}

bool InfoBubble::init(ArrowDirection direction)
{
    if (!Node::init())
        return false;

	m_direction = direction;

	switch (m_direction)
	{
	case ARROW_UP:
		m_arrowSp = Sprite::createWithSpriteFrameName(FRAMENAME_INFO_BUBBLE_ARROW_UP);
		m_arrowSp->setAnchorPoint(Point::ANCHOR_MIDDLE_TOP);
		break;
	case ARROW_DOWN:
		m_arrowSp = Sprite::createWithSpriteFrameName(FRAMENAME_INFO_BUBBLE_ARROW_DOWN);
		m_arrowSp->setAnchorPoint(Point::ANCHOR_MIDDLE_BOTTOM);
		break;
	default:
		break;
	}

	m_bubbleSp = Scale9Sprite::createWithSpriteFrameName(FRAMENAME_INFO_BUBBLE_BG);
	m_bubbleSp->setVisible(false);
	m_bubbleSp->setScale(0.f);
	this->addChild(m_bubbleSp);

	m_bubbleSp->addChild(m_arrowSp);

	m_iconSp = Sprite::createWithSpriteFrameName(FRAMENAME_INFO_BUBBLE_ICON);
	m_iconSp->setAnchorPoint(Point::ANCHOR_TOP_LEFT);
	m_bubbleSp->addChild(m_iconSp);

	m_infoLabel = Label::createWithSystemFont("info", DEFAULT_SYSTEM_FONT, 11);
	m_infoLabel->setAnchorPoint(Point::ANCHOR_TOP_LEFT);
	m_infoLabel->setTextColor(Color4B(86, 111, 132, 255));
	m_infoLabel->enableWrap(true);
	Utils::enableBoldForLabel(m_infoLabel);
	m_bubbleSp->addChild(m_infoLabel);

	this->layoutElements();

    return true;
}

InfoBubble::InfoBubble() :
	m_isShown(false),
	m_direction(ARROW_UP),
	m_arrowPosition(0),
	m_infoLabel(nullptr),
	m_bubbleSp(nullptr),
	m_arrowSp(nullptr),
	m_iconSp(nullptr)
{
}

InfoBubble::~InfoBubble()
{
	this->stopAnimation();

	m_infoLabel = nullptr;
	m_bubbleSp = nullptr;
	m_arrowSp = nullptr;
	m_iconSp = nullptr;
}

void InfoBubble::show(std::string const& text)
{
	if(m_isShown)
		this->hide();

	m_infoLabel->setString(text);
	this->layoutElements();
	this->startPopAnimation();

	m_isShown = true;
}

void InfoBubble::hide()
{
	if (!m_isShown)
		return;

	this->startPushAnimation();

	m_isShown = false;
}

void InfoBubble::setArrowPosition(float position)
{
	m_arrowPosition = position;
}

void InfoBubble::startPopAnimation()
{
	this->stopAnimation();

	Vector<FiniteTimeAction*> actions;

	actions.pushBack(Show::create());
	actions.pushBack(EaseBackOut::create(ScaleTo::create(0.3f, 1.0f)));
	actions.pushBack(DelayTime::create(1.0f));
	auto move = MoveBy::create(0.9f, Vec2(0, 4));
	auto repeatMove = Repeat::create(Sequence::createWithTwoActions(move, move->reverse()), std::pow(2, 30));
	actions.pushBack(repeatMove);
	auto seq = Sequence::create(actions);
	seq->setTag(ACTION_TAG_BUBBLE);
	m_bubbleSp->runAction(seq);
}

void InfoBubble::startPushAnimation()
{
	this->stopAnimation();

	auto seq = Sequence::create(EaseCubicActionIn::create(ScaleTo::create(0.3f, 0.0f)), Hide::create(), nullptr);
	seq->setTag(ACTION_TAG_BUBBLE);
	m_bubbleSp->runAction(seq);
}

void InfoBubble::stopAnimation()
{
	if (Action* action = m_bubbleSp->getActionByTag(ACTION_TAG_BUBBLE))
		m_bubbleSp->stopAction(action);

}

void InfoBubble::layoutElements()
{
	m_infoLabel->setWidth(0);
	float infoWidth = m_infoLabel->getContentSize().width;
	float infoHeight = m_infoLabel->getContentSize().height;

	float infoMarginTop = INFO_SINGLE_LINE_MARGIN_TOP;
	if (infoWidth > INFO_MAX_LINE_WIDTH)
	{
		m_infoLabel->setWidth(INFO_MAX_LINE_WIDTH);
		infoWidth = m_infoLabel->getContentSize().width;
		infoHeight = m_infoLabel->getContentSize().height;
		infoMarginTop = INFO_MULTI_LINE_MARGIN_TOP;
	}

	float bubbleWidth = INFO_MARGIN_LEFT + infoWidth + INFO_MARGIN_RIGHT;
	float bubbleHeight = infoMarginTop + infoHeight + INFO_MARGIN_BOTTOM;
	bubbleWidth = MAX(bubbleWidth, BUBBLE_MIN_WIDTH);
	bubbleHeight = MAX(bubbleHeight, BUBBLE_MIN_HEIGHT);
	float arrowPos = m_arrowPosition;
	if (arrowPos <= 0)
		arrowPos = bubbleWidth / 2;

	m_bubbleSp->setContentSize(Size(bubbleWidth, bubbleHeight));
	switch (m_direction)
	{
	case ARROW_UP:
		m_bubbleSp->setAnchorPoint(Vec2(arrowPos / bubbleWidth, 1.0));
		m_bubbleSp->setPosition(Vec2(arrowPos, bubbleHeight));
		m_arrowSp->setPosition(Vec2(arrowPos, bubbleHeight));
		break;
	case ARROW_DOWN:
		m_bubbleSp->setAnchorPoint(Vec2(arrowPos / bubbleWidth, 0));
		m_bubbleSp->setPosition(Vec2(arrowPos, 0));
		m_arrowSp->setPosition(Vec2(arrowPos, 0));
		break;
	}

	m_iconSp->setPosition(Vec2(0, bubbleHeight));
	m_infoLabel->setPosition(INFO_MARGIN_LEFT, m_bubbleSp->getContentSize().height - infoMarginTop);

	this->setContentSize(m_bubbleSp->getContentSize());
}

NS_END
