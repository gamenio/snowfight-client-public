#include "MoveControlGesture.h"

#include "common/utils/MathTools.h"

NS_BEGIN

#define FRAMENAME_MOVE_HANDLE_PRESS			    "move_handle_press.png"
#define FRAMENAME_TUTORIAL_THUMB			    "tutorial_thumb.png"

#define HANDLE_MOVE_DIRECTION					135.f	
#define THUMB_RELEASED_POSITION					Vec2(-9.f, -5.f)
#define THUMB_PRESSED_POSITION					Vec2(6.f, 6.f)
#define ACTION_TAG_DRAGGING						1
#define GESTURE_ANIMATION_INTERVAL				0.5f

MoveControlGesture* MoveControlGesture::create(float maxMoveRadius)
{
	MoveControlGesture* ret = new (std::nothrow) MoveControlGesture();
	if (ret && ret->init(maxMoveRadius))
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

bool MoveControlGesture::init(float maxMoveRadius)
{
	if (!Node::init())
	{
		return false;
	}

	m_maxMoveRadius = maxMoveRadius;

	m_handleSp = Sprite::createWithSpriteFrameName(FRAMENAME_MOVE_HANDLE_PRESS);
	m_handleSp->setVisible(false);
	this->addChild(m_handleSp);

	m_thumbSp = Sprite::createWithSpriteFrameName(FRAMENAME_TUTORIAL_THUMB);
	m_thumbSp->setAnchorPoint(Point::ANCHOR_TOP_RIGHT);
	m_thumbSp->setFlippedX(true);
	m_thumbSp->setPosition(THUMB_RELEASED_POSITION);
	this->addChild(m_thumbSp);

	return true;

}

MoveControlGesture::MoveControlGesture() :
	m_maxMoveRadius(0.f),
	m_handleSp(nullptr),
	m_thumbSp(nullptr)
{

}

MoveControlGesture::~MoveControlGesture()
{
	m_handleSp = nullptr;
	m_thumbSp = nullptr;
}

void MoveControlGesture::startGestureAnimation(float delay)
{
	this->stopGestureAnimation();

	Point dest;
	float rad = MathTools::degrees2Radians(HANDLE_MOVE_DIRECTION);
	dest.x = std::cos(rad) * m_maxMoveRadius;
	dest.y = std::sin(rad) * m_maxMoveRadius;

	Action* action = nullptr;
	Repeat* repeat = nullptr;

	// Thumb
	Vector<FiniteTimeAction*> arrayOfActions;
	arrayOfActions.pushBack(ScaleTo::create(0.f, 1.0f));
	arrayOfActions.pushBack(Place::create(THUMB_RELEASED_POSITION));
	Spawn* spawn = Spawn::createWithTwoActions(MoveTo::create(0.2f, THUMB_PRESSED_POSITION), ScaleTo::create(0.2f, 0.85f));
	arrayOfActions.pushBack(spawn);
	arrayOfActions.pushBack(MoveTo::create(0.3f, dest + THUMB_PRESSED_POSITION));
	arrayOfActions.pushBack(DelayTime::create(GESTURE_ANIMATION_INTERVAL));
	repeat = Repeat::create(Sequence::create(arrayOfActions), std::pow(2, 30));

	arrayOfActions.clear();
	if (delay > 0)
		arrayOfActions.pushBack(DelayTime::create(delay));
	arrayOfActions.pushBack(Show::create());
	arrayOfActions.pushBack(repeat);
	action = Sequence::create(arrayOfActions);
	action->setTag(ACTION_TAG_DRAGGING);
	m_thumbSp->runAction(action);

	arrayOfActions.clear();
	arrayOfActions.pushBack(Hide::create());
	arrayOfActions.pushBack(Place::create(Vec2(0.f, 0.f)));
	arrayOfActions.pushBack(DelayTime::create(0.2f));
	arrayOfActions.pushBack(Show::create());
	arrayOfActions.pushBack(MoveTo::create(0.3f, dest));
	arrayOfActions.pushBack(DelayTime::create(GESTURE_ANIMATION_INTERVAL));
	repeat = Repeat::create(Sequence::create(arrayOfActions), std::pow(2, 30));

	arrayOfActions.clear();
	if (delay > 0)
		arrayOfActions.pushBack(DelayTime::create(delay));
	arrayOfActions.pushBack(repeat);
	action = Sequence::create(arrayOfActions);
	action->setTag(ACTION_TAG_DRAGGING);
	m_handleSp->runAction(action);
}

void MoveControlGesture::stopGestureAnimation()
{
	if (Action* action = m_thumbSp->getActionByTag(ACTION_TAG_DRAGGING))
		m_thumbSp->stopAction(action);

	if (Action* action = m_handleSp->getActionByTag(ACTION_TAG_DRAGGING))
		m_handleSp->stopAction(action);

	m_thumbSp->setPosition(THUMB_RELEASED_POSITION);
	m_thumbSp->setVisible(false);
	m_thumbSp->setScale(1.0f);
	m_handleSp->setPosition(Vec2(0.f, 0.f));
	m_handleSp->setVisible(false);
}

NS_END
