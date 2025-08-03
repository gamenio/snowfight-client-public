#include "AttackControlGesture.h"

#include "common/utils/MathTools.h"

NS_BEGIN

#define FRAMENAME_ATTACK_HANDLE_PRESS			"attack_handle_press.png"
#define FRAMENAME_ATTACK_HANDLE_CHARGE_PRESS	"attack_handle_charge_press.png"
#define FRAMENAME_TUTORIAL_THUMB				"tutorial_thumb.png"
#define FRAMENAME_TUTORIAL_CLOCK_DIAL			"tutorial_clock_dial.png"
#define FRAMENAME_TUTORIAL_CLOCK_POINTER		"tutorial_clock_pointer.png"

#define HANDLE_MOVE_DIRECTION					45.f	
#define THUMB_RELEASED_POSITION					Vec2(9.f, -5.f)
#define THUMB_PRESSED_POSITION					Vec2(-6.f, 6.f)
#define ACTION_TAG_DRAGGING						1
#define ACTION_TAG_LONG_PRESSING				2
#define GESTURE_ANIMATION_INTERVAL				0.5f

AttackControlGesture* AttackControlGesture::create(float maxMoveRadius)
{
	AttackControlGesture* ret = new (std::nothrow) AttackControlGesture();
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

bool AttackControlGesture::init(float maxMoveRadius)
{
	if (!Node::init())
	{
		return false;
	}

	m_maxMoveRadius = maxMoveRadius;

	m_handleSp = Sprite::createWithSpriteFrameName(FRAMENAME_ATTACK_HANDLE_PRESS);
	m_handleSp->setVisible(false);
	this->addChild(m_handleSp);

	m_thumbSp = Sprite::createWithSpriteFrameName(FRAMENAME_TUTORIAL_THUMB);
	m_thumbSp->setAnchorPoint(Point::ANCHOR_TOP_LEFT);
	m_thumbSp->setPosition(THUMB_RELEASED_POSITION);
	this->addChild(m_thumbSp);

	m_clockDialSp = Sprite::createWithSpriteFrameName(FRAMENAME_TUTORIAL_CLOCK_DIAL);
	m_clockDialSp->setPosition(40, 28);
	m_clockDialSp->setVisible(false);
	this->addChild(m_clockDialSp);

	m_clockPointerSp = Sprite::createWithSpriteFrameName(FRAMENAME_TUTORIAL_CLOCK_POINTER);
	m_clockPointerSp->setPosition(40, 28);
	m_clockPointerSp->setVisible(false);
	this->addChild(m_clockPointerSp);

	return true;

}

AttackControlGesture::AttackControlGesture() :
	m_maxMoveRadius(0.f),
	m_handleSp(nullptr),
	m_thumbSp(nullptr),
	m_clockDialSp(nullptr),
	m_clockPointerSp(nullptr)
{

}

AttackControlGesture::~AttackControlGesture()
{
	m_handleSp = nullptr;
	m_thumbSp = nullptr;
	m_clockDialSp = nullptr;
	m_clockPointerSp = nullptr;
}

void AttackControlGesture::startGestureAnimation(GestureType gesture, float delay)
{
	this->stopGestureAnimation();

	switch (gesture)
	{
	case GESTURE_TYPE_DRAG:
		this->startDragAnimation(delay);
		break;
	case GESTURE_TYPE_LONG_PRESS:
		this->startLongPressAnimation(delay);
		break;
	default:
		break;
	}
}

void AttackControlGesture::stopGestureAnimation()
{
	if (Action* action = m_thumbSp->getActionByTag(ACTION_TAG_DRAGGING))
		m_thumbSp->stopAction(action);
	if (Action* action = m_handleSp->getActionByTag(ACTION_TAG_DRAGGING))
		m_handleSp->stopAction(action);

	if (Action* action = m_thumbSp->getActionByTag(ACTION_TAG_LONG_PRESSING))
		m_thumbSp->stopAction(action);
	if (Action* action = m_handleSp->getActionByTag(ACTION_TAG_LONG_PRESSING))
		m_handleSp->stopAction(action);
	if (Action* action = m_clockPointerSp->getActionByTag(ACTION_TAG_LONG_PRESSING))
		m_clockPointerSp->stopAction(action);
	if (Action* action = m_clockDialSp->getActionByTag(ACTION_TAG_LONG_PRESSING))
		m_clockDialSp->stopAction(action);

	m_thumbSp->setPosition(THUMB_RELEASED_POSITION);
	m_thumbSp->setVisible(false);
	m_thumbSp->setScale(1.0f);
	m_handleSp->setPosition(Vec2(0.f, 0.f));
	m_handleSp->setVisible(false);
	m_clockDialSp->setVisible(false);
	m_clockPointerSp->setVisible(false);
}

void AttackControlGesture::startDragAnimation(float delay)
{
	m_handleSp->setSpriteFrame(FRAMENAME_ATTACK_HANDLE_PRESS);

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

	// Handle
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

void AttackControlGesture::startLongPressAnimation(float delay)
{
	m_handleSp->setSpriteFrame(FRAMENAME_ATTACK_HANDLE_CHARGE_PRESS);

	Point dest;
	float rad = MathTools::degrees2Radians(HANDLE_MOVE_DIRECTION);
	dest.x = std::cos(rad) * m_maxMoveRadius;
	dest.y = std::sin(rad) * m_maxMoveRadius;

	Action* action = nullptr;
	Repeat* repeat = nullptr;
	Vector<FiniteTimeAction*> arrayOfActions;

	// Thumb
	arrayOfActions.pushBack(ScaleTo::create(0.f, 1.0f));
	arrayOfActions.pushBack(Place::create(THUMB_RELEASED_POSITION));
	Spawn* spawn = Spawn::createWithTwoActions(MoveTo::create(0.2f, THUMB_PRESSED_POSITION), ScaleTo::create(0.2f, 0.85f));
	arrayOfActions.pushBack(spawn);
	arrayOfActions.pushBack(DelayTime::create(0.8f));
	arrayOfActions.pushBack(MoveTo::create(0.3f, dest + THUMB_PRESSED_POSITION));
	arrayOfActions.pushBack(DelayTime::create(GESTURE_ANIMATION_INTERVAL));
	repeat = Repeat::create(Sequence::create(arrayOfActions), std::pow(2, 30));

	arrayOfActions.clear();
	if (delay > 0)
		arrayOfActions.pushBack(DelayTime::create(delay));
	arrayOfActions.pushBack(Show::create());
	arrayOfActions.pushBack(repeat);
	action = Sequence::create(arrayOfActions);
	action->setTag(ACTION_TAG_LONG_PRESSING);
	m_thumbSp->runAction(action);

	// Handle
	arrayOfActions.clear();
	arrayOfActions.pushBack(Hide::create());
	arrayOfActions.pushBack(ScaleTo::create(0.f, 1.0f));
	arrayOfActions.pushBack(Place::create(Vec2(0.f, 0.f)));
	arrayOfActions.pushBack(DelayTime::create(0.2f));
	arrayOfActions.pushBack(Show::create());
	arrayOfActions.pushBack(ScaleTo::create(0.6f, 1.15f));
	arrayOfActions.pushBack(ScaleTo::create(0.0f, 1.f));
	arrayOfActions.pushBack(DelayTime::create(0.2f));
	arrayOfActions.pushBack(MoveTo::create(0.3f, dest));
	arrayOfActions.pushBack(DelayTime::create(GESTURE_ANIMATION_INTERVAL));
	repeat = Repeat::create(Sequence::create(arrayOfActions), std::pow(2, 30));

	arrayOfActions.clear();
	if (delay > 0)
		arrayOfActions.pushBack(DelayTime::create(delay));
	arrayOfActions.pushBack(repeat);
	action = Sequence::create(arrayOfActions);
	action->setTag(ACTION_TAG_LONG_PRESSING);
	m_handleSp->runAction(action);

	// Clock pointer
	arrayOfActions.clear();
	arrayOfActions.pushBack(RotateTo::create(0.f, 0.f));
	arrayOfActions.pushBack(DelayTime::create(0.2f));
	arrayOfActions.pushBack(Show::create());
	arrayOfActions.pushBack(RotateBy::create(0.6f, 360.f));
	arrayOfActions.pushBack(DelayTime::create(0.1f));
	arrayOfActions.pushBack(Hide::create());
	arrayOfActions.pushBack(DelayTime::create(0.4f + GESTURE_ANIMATION_INTERVAL));
	repeat = Repeat::create(Sequence::create(arrayOfActions), std::pow(2, 30));

	arrayOfActions.clear();
	if (delay > 0)
		arrayOfActions.pushBack(DelayTime::create(delay));
	arrayOfActions.pushBack(repeat);
	action = Sequence::create(arrayOfActions);
	action->setTag(ACTION_TAG_LONG_PRESSING);
	m_clockPointerSp->runAction(action);

	// Clock dial
	arrayOfActions.clear();
	arrayOfActions.pushBack(DelayTime::create(0.2f));
	arrayOfActions.pushBack(Show::create());
	arrayOfActions.pushBack(DelayTime::create(0.7f));
	arrayOfActions.pushBack(Hide::create());
	arrayOfActions.pushBack(DelayTime::create(0.4f + GESTURE_ANIMATION_INTERVAL));
	repeat = Repeat::create(Sequence::create(arrayOfActions), std::pow(2, 30));

	arrayOfActions.clear();
	if (delay > 0)
		arrayOfActions.pushBack(DelayTime::create(delay));
	arrayOfActions.pushBack(repeat);
	action = Sequence::create(arrayOfActions);
	action->setTag(ACTION_TAG_LONG_PRESSING);
	m_clockDialSp->runAction(action);
}

NS_END
