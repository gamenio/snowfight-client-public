#include "AttackStick.h"

#include "common/utils/MathTools.h"
#include "common/utils/TimeUtil.h"
#include "common/Machine.h"
#include "common/debugging/DebugDrawer.h"
#include "game/gamecenter/GameCenter.h"
#include "scene/Utils.h"

NS_BEGIN

#define FRAMENAME_STICK_BASE						"stick_base.png"
#define FRAMENAME_STICK_BASE_CENTER					"stick_base_center.png"
#define FRAMENAME_ATTACK_HANDLE_NORMAL				"attack_handle_normal.png"
#define FRAMENAME_ATTACK_HANDLE_PRESS				"attack_handle_press.png"
#define FRAMENAME_ATTACK_HANDLE_CHARGE_NORMAL		"attack_handle_charge_normal.png"
#define FRAMENAME_ATTACK_HANDLE_CHARGE_PRESS		"attack_handle_charge_press.png"

#if NS_DEBUG

#define DEBUG_STICK							    0
#define DEBUG_TOUCHED_POINTS				    0

#endif // NS_DEBUG

#define HANDLE_NEUTRAL_RADIUS                   18
#define HANDLE_NORMAL_OPACITY				    191
#define HANDLE_PRESS_OPACITY				    255	
#define HANDLE_ACTION_TAG_ZOOMING				1
#define HANDLE_ZOOM_DELAY						0.1f
#define HANDLE_ZOOM_SCALE						1.15f

// Space to the right of the stick. Unit: inches
#define TABLET_SPACE_RIGHT_INCH               	0.83f
#define TABLET_FULLSCREEN_SPACE_RIGHT_INCH     	1.26f	// Full-screen tablet
#define PHONE_SPACE_RIGHT_INCH                  0.57f
#define PHONE_FULLSCREEN_SPACE_RIGHT_INCH       0.81f   // Full-screen phone

// Stick bottom margin
#define TABLET_MARGIN_BOTTOM                    35
#define PHONE_MARGIN_BOTTOM                     20

// Stick size. Unit: inches
#define TABLET_STICK_SIZE_INCH                  1.02f
#define PHONE_STICK_SIZE_INCH                   0.74f

#define CHARGE_STATE_DELAY					    500			// The delay of the charging state. Unit: milliseconds

AttackStick* AttackStick::create()
{
	AttackStick* ret = new (std::nothrow) AttackStick();
	if (ret && ret->init())
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

bool AttackStick::init()
{
	if (!Layer::init())
	{
		return false;
	}
    
	this->setCascadeOpacityEnabled(true);

	auto touchListener = EventListenerTouchOneByOne::create();
	touchListener->onTouchBegan = CC_CALLBACK_2(AttackStick::onTouchBegan, this);
	touchListener->onTouchMoved = CC_CALLBACK_2(AttackStick::onTouchMoved, this);
	touchListener->onTouchEnded = CC_CALLBACK_2(AttackStick::onTouchEnded, this);
	touchListener->onTouchCancelled = CC_CALLBACK_2(AttackStick::onTouchCancelled, this);
	this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(touchListener, this);
    
    Rect safeArea = Machine::instance()->getSafeAreaRect();
    m_safeInsetRight = Utils::getSafeInsetRight(safeArea);
    m_safeInsetBottom = Utils::getSafeInsetBottom(safeArea);
    
    float stickSizeInPoints;
    if (Machine::instance()->isTablet())
        stickSizeInPoints = Utils::convertDistanceFromInchToPoint(TABLET_STICK_SIZE_INCH);
    else
        stickSizeInPoints = Utils::convertDistanceFromInchToPoint(PHONE_STICK_SIZE_INCH);
    SpriteFrame* baseSf = SpriteFrameCache::getInstance()->getSpriteFrameByName(FRAMENAME_STICK_BASE);
    m_scale = stickSizeInPoints / baseSf->getOriginalSize().width;

	m_baseSp = Sprite::createWithSpriteFrameName(FRAMENAME_STICK_BASE);
	m_baseSp->setScale(m_scale);
	this->addChild(m_baseSp);

	m_baseCenterSp = Sprite::createWithSpriteFrameName(FRAMENAME_STICK_BASE_CENTER);
	m_baseCenterSp->setPosition(m_baseSp->getContentSize().width / 2, m_baseSp->getContentSize().height / 2);
	m_baseCenterSp->setVisible(false);
	m_baseSp->addChild(m_baseCenterSp);

	m_handleSp = Sprite::createWithSpriteFrameName(FRAMENAME_ATTACK_HANDLE_NORMAL);
	m_handleSp->setScale(m_scale);
	m_handleSp->setOpacity(HANDLE_NORMAL_OPACITY);
	this->addChild(m_handleSp);

	if (sGameCenter->getLocalPlayer()->isTrainee())
	{
		m_controlGesture = AttackControlGesture::create(m_baseSp->getBoundingBox().size.width / 2);
		m_controlGesture->setScale(m_scale);
		m_controlGesture->setVisible(false);
		this->addChild(m_controlGesture);
	}

	this->initBasePosition();

	this->scheduleUpdate();

#if NS_DEBUG
	m_debugDrawNode = DrawNode::create(1.0f);
	this->addChild(m_debugDrawNode);
#endif // NS_DEBUG

	return true;

}

AttackStick::AttackStick() :
    m_safeInsetRight(0),
    m_safeInsetBottom(0),
	m_isHitted(false),
	m_pressedTime(0),
	m_scale(1.0f),
	m_maxMoveRadius(0),
	m_isChargeEnabled(false),
	m_direction(FLT_MAX),
	m_isAiming(false),
	m_isMoving(false),
	m_isCharging(false),
	m_isCanceling(false),
	m_debugDrawNode(nullptr),
	m_baseSp(nullptr),
	m_baseCenterSp(nullptr),
	m_handleSp(nullptr),
	m_gestureType(AttackControlGesture::GESTURE_TYPE_NONE),
	m_controlGesture(nullptr)
{

}

AttackStick::~AttackStick()
{
	m_debugDrawNode = nullptr;
	m_baseSp = nullptr;
	m_baseCenterSp = nullptr;
	m_handleSp = nullptr;
}

void AttackStick::setChargeEnabled(bool enabled)
{
	if (m_isChargeEnabled == enabled)
		return;

	if (enabled)
		m_handleSp->setSpriteFrame(FRAMENAME_ATTACK_HANDLE_CHARGE_NORMAL);
	else
		m_handleSp->setSpriteFrame(FRAMENAME_ATTACK_HANDLE_NORMAL);

	m_isChargeEnabled = enabled;
}

void AttackStick::moveHandle(Point const& position)
{
	Point handlePos = position;

	Point diff = handlePos - m_baseSp->getPosition();
	float dist = diff.length();
    float dir = std::atan2(diff.y, diff.x);
	float max = m_baseSp->getBoundingBox().size.width / 2;

	if (dist > max)
	{
		float rad = std::atan2(diff.y, diff.x);
		float dx = std::cos(rad) * max;
		float dy = std::sin(rad) * max;
		handlePos = m_baseSp->getPosition() + Vec2(dx, dy);
		dist = max;
	}
	m_handleSp->setPosition(handlePos);

	//CCLOG("AttackStick::moveHandle direction: %f touchPoint: [%f,%f]", MathTools::radians2Degrees(dir), touchPoint.x, touchPoint.y);

#if DEBUG_TOUCHED_POINTS
	m_debugDrawNode->drawDot(position, 1, Color4F::BLUE);
	m_debugDrawNode->drawDot(m_handleSp->getPosition(), 1, Color4F::GREEN);
	m_debugDrawNode->drawDot(m_baseSp->getPosition(), 1, Color4F::RED);
#endif // DEBUG_TOUCHED_POINTS

	if (dist > HANDLE_NEUTRAL_RADIUS * m_scale)
	{
		m_direction = dir;
		m_isAiming = true;
		m_isCanceling = false;

		this->stopHandleZoomAnimation();

		if (onAttackStickAiming)
			onAttackStickAiming(this, m_direction);
	}
	else if(m_isAiming && !m_isCanceling)
	{
		if (onAttackStickCanceling)
			onAttackStickCanceling(this);
		m_isAiming = false;
		m_isCanceling = true;
	}
}

void AttackStick::moveBase(Point const& position)
{
	Point basePos = position;
	m_baseSp->setPosition(basePos);
	m_handleSp->setPosition(basePos);
}

void AttackStick::initBasePosition()
{
    m_maxMoveRadius = (m_baseSp->getContentSize().width / 2 + m_handleSp->getContentSize().width / 2) * m_scale;
    
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    auto visibleSize = Director::getInstance()->getVisibleSize();
    
    float spaceRight = 0.f;
    float marginBottom = 0.f;
    if(Machine::instance()->isTablet())
    {
		// If the tablet is full screen
        if(m_safeInsetBottom > 0 || m_safeInsetRight > 0)
            spaceRight = Utils::convertDistanceFromInchToPoint(TABLET_FULLSCREEN_SPACE_RIGHT_INCH);
        else
            spaceRight = Utils::convertDistanceFromInchToPoint(TABLET_SPACE_RIGHT_INCH);
		marginBottom = TABLET_MARGIN_BOTTOM;
    }
    else
    {
		// If the phone is full screen
        if(m_safeInsetRight > 0)
			spaceRight = Utils::convertDistanceFromInchToPoint(PHONE_FULLSCREEN_SPACE_RIGHT_INCH);
        else
			spaceRight = Utils::convertDistanceFromInchToPoint(PHONE_SPACE_RIGHT_INCH);
		marginBottom = PHONE_MARGIN_BOTTOM;
    }
	m_baseOriginPosition.x = origin.x + visibleSize.width - spaceRight;
	m_baseOriginPosition.y = origin.y + marginBottom + m_maxMoveRadius;

    m_baseSp->setPosition(m_baseOriginPosition);
    m_handleSp->setPosition(m_baseOriginPosition);
	if (m_controlGesture)
		m_controlGesture->setPosition(m_baseOriginPosition);
}

void AttackStick::reset()
{
    this->moveBase(m_baseOriginPosition);
	if (m_isChargeEnabled)
		m_handleSp->setSpriteFrame(FRAMENAME_ATTACK_HANDLE_CHARGE_NORMAL);
	else
		m_handleSp->setSpriteFrame(FRAMENAME_ATTACK_HANDLE_NORMAL);
	m_handleSp->setOpacity(HANDLE_NORMAL_OPACITY);
	this->stopHandleZoomAnimation();
	m_baseCenterSp->setVisible(false);
	if (m_gestureType != AttackControlGesture::GESTURE_TYPE_NONE && m_controlGesture)
	{
		m_controlGesture->setVisible(true);
		m_controlGesture->startGestureAnimation(m_gestureType, 2.0f);
	}

	m_direction = FLT_MAX;
	m_isAiming = false;
	m_isMoving = false;
	m_isCharging = false;
	m_isCanceling = false;

	m_isHitted = false;
	m_pressedTime = 0;
}

void AttackStick::startHandleZoomAnimation()
{
	if (Action* action = m_handleSp->getActionByTag(HANDLE_ACTION_TAG_ZOOMING))
		m_handleSp->stopAction(action);

	Vector<FiniteTimeAction*> arrayOfActions;
	arrayOfActions.pushBack(DelayTime::create(HANDLE_ZOOM_DELAY));
	arrayOfActions.pushBack(ScaleTo::create(time_util::toGameTimeSeconds(CHARGE_STATE_DELAY) - HANDLE_ZOOM_DELAY, m_scale * HANDLE_ZOOM_SCALE));
	Action* action = Sequence::create(arrayOfActions);
	action->setTag(HANDLE_ACTION_TAG_ZOOMING);
	m_handleSp->runAction(action);
}

void AttackStick::stopHandleZoomAnimation()
{
	m_handleSp->setScale(m_scale);
	if (Action* action = m_handleSp->getActionByTag(HANDLE_ACTION_TAG_ZOOMING))
		m_handleSp->stopAction(action);
}

void AttackStick::update(float delta)
{
	if (!m_isHitted)
		return;

	if (m_isChargeEnabled && !m_isCanceling && !m_isAiming && !m_isCharging)
	{
		NSTime diff = time_util::getUptimeMillis() - m_pressedTime;
		if (diff >= CHARGE_STATE_DELAY)
		{
			if (onAttackStickCharging)
				onAttackStickCharging(this);

			this->stopHandleZoomAnimation();

            Machine::instance()->vibrate(VIBRATION_TYPE_TOUCH_HOLD);
			m_isCharging = true;
		}
	}
}

void AttackStick::draw(Renderer* renderer, Mat4 const& transform, uint32_t flags)
{
	Layer::draw(renderer, transform, flags);

#if DEBUG_STICK
	if (m_debugDrawNode)
	{
		m_debugDrawNode->clear();

		m_debugDrawNode->drawDot(m_baseSp->getPosition(), 1, Color4F::YELLOW);
		m_debugDrawNode->drawCircle(m_baseSp->getPosition(), m_baseSp->getBoundingBox().size.width / 2, 0, 360, false, Color4F::YELLOW);

		m_debugDrawNode->drawDot(m_handleSp->getPosition(), 1, Color4F::GREEN);
		m_debugDrawNode->drawCircle(m_handleSp->getPosition(), m_handleSp->getBoundingBox().size.width / 2, 0, 360, false, Color4F::GREEN);

		m_debugDrawNode->drawCircle(m_baseSp->getPosition(), m_maxMoveRadius, 0, 360, false, Color4F::RED);
		m_debugDrawNode->drawCircle(m_baseSp->getPosition(), HANDLE_NEUTRAL_RADIUS * m_scale, 0, 360, false, Color4F::RED);
	}
#endif // DEBUG_STICK
}

void AttackStick::setControlGestureEnabled(AttackControlGesture::GestureType gesture, float animationDelay)
{
	if (!m_controlGesture || m_gestureType == gesture)
		return;

	m_gestureType = gesture;

	if (m_gestureType != AttackControlGesture::GESTURE_TYPE_NONE)
	{
		if (!m_isHitted)
		{
			m_controlGesture->setVisible(true);
			m_controlGesture->startGestureAnimation(m_gestureType, animationDelay);
		}
	}
	else
	{
		m_controlGesture->stopGestureAnimation();
		m_controlGesture->setVisible(false);
	}
}

bool AttackStick::hitTest(Vec2 const& pt) const
{
	Vec2 origin = Director::getInstance()->getVisibleOrigin();
	auto visibleSize = Director::getInstance()->getVisibleSize();
	Rect rect(origin.x + visibleSize.width / 2, origin.y + m_safeInsetBottom, visibleSize.width / 2, visibleSize.height - m_safeInsetBottom);
	return rect.containsPoint(pt);
}

void AttackStick::printTouches(std::string const& eventName, std::vector<Touch*> const& touches)
{
	std::stringstream ss;
	ss << "{ ";
	for (std::size_t i = 0; i < touches.size(); i++)
	{
		Touch* touch = touches.at(i);
		Point touchLocation = this->convertTouchToNodeSpace(touch);
		ss << "ID=" << touch->getID();
		ss << "[" << touchLocation.x << "," << touchLocation.y << "]";
		if (i < touches.size() - 1)
			ss << ", ";
	}
	ss << " }";
	CCLOG("%s %s", eventName.c_str(), ss.str().c_str());
}

bool AttackStick::onTouchBegan(Touch* touch, Event* event)
{
	//this->printTouches("AttackStick::onTouchBegan", { touch });

	if (this->isVisible() && !m_isHitted)
	{
        Vec2 touchLocation = this->convertTouchToNodeSpace(touch);
		if (this->hitTest(touchLocation))
		{
			this->moveBase(touchLocation);
			if (m_isChargeEnabled)
			{
				m_handleSp->setSpriteFrame(FRAMENAME_ATTACK_HANDLE_CHARGE_PRESS);
				this->startHandleZoomAnimation();
			}
			else
				m_handleSp->setSpriteFrame(FRAMENAME_ATTACK_HANDLE_PRESS);
			m_handleSp->setOpacity(HANDLE_PRESS_OPACITY);
			m_baseCenterSp->setVisible(true);

			if (m_gestureType != AttackControlGesture::GESTURE_TYPE_NONE && m_controlGesture)
				m_controlGesture->stopGestureAnimation();

			m_pressedTime = time_util::getUptimeMillis();
			m_isHitted = true;

			return true;
		}
	}

	return false;
}

void AttackStick::onTouchMoved(Touch* touch, Event* event)
{
	//this->printTouches("AttackStick::onTouchMoved", { touch });

	float offset = touch->getStartLocation().distance(touch->getLocation());
	if(offset == 0)
	    return;

	float inch = Utils::convertDistanceFromPointToInch(offset);
	if (inch > GESTURE_THRESHOLD_INCH || m_isMoving)
	{
		m_isMoving = true;
		Point touchPoint = this->convertTouchToNodeSpace(touch);
		this->moveHandle(touchPoint);
	}
}

void AttackStick::onTouchEnded(Touch* touch, Event* event)
{
	//this->printTouches("AttackStick::onTouchEnded", { touch });

	if (!m_isCanceling)
	{
		if (onAttackStickFire)
			onAttackStickFire(this, m_direction);
	}
	else
	{
		if (onAttackStickCanceled)
			onAttackStickCanceled(this);
	}

	this->reset();
}

void AttackStick::onTouchCancelled(Touch* touch, Event* event)
{
    //this->printTouches("AttackStick::onTouchCancelled", { touch });

	if (m_isCanceling || m_isAiming || m_isCharging)
	{
		if (onAttackStickCanceled)
			onAttackStickCanceled(this);
	}

	this->reset();
}

NS_END
