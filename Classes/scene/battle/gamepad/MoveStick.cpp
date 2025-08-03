#include "MoveStick.h"

#include "common/utils/MathTools.h"
#include "common/Machine.h"
#include "common/debugging/DebugDrawer.h"
#include "game/gamecenter/GameCenter.h"
#include "scene/Utils.h"

NS_BEGIN

#define FRAMENAME_STICK_BASE				    "stick_base.png"
#define FRAMENAME_STICK_BASE_CENTER			    "stick_base_center.png"
#define FRAMENAME_MOVE_HANDLE_NORMAL		    "move_handle_normal.png"
#define FRAMENAME_MOVE_HANDLE_PRESS			    "move_handle_press.png"

#define FRAMENAME_TUTORIAL_THUMB			    "tutorial_thumb.png"
#define FRAMENAME_TUTORIAL_CLOCK_DIAL			"tutorial_clock_dial.png"
#define FRAMENAME_TUTORIAL_CLOCK_POINTER		"tutorial_clock_pointer.png"

#if NS_DEBUG

#define DEBUG_STICK						        0
#define DEBUG_TOUCHED_POINTS				    0

#endif // NS_DEBUG

#define HANDLE_NORMAL_OPACITY				    191
#define HANDLE_PRESS_OPACITY				    255

// 摇杆左侧的空间。单位：英寸
#define TABLET_SPACE_LEFT_INCH                 	0.83f
#define TABLET_FULLSCREEN_SPACE_LEFT_INCH       1.26f   // 全屏幕平板
#define PHONE_SPACE_LEFT_INCH                   0.57f
#define PHONE_FULLSCREEN_SPACE_LEFT_INCH        0.81f   // 全屏幕手机

// 摇杆底部边距
#define TABLET_MARGIN_BOTTOM                    35
#define PHONE_MARGIN_BOTTOM                     20

// 摇杆大小。单位：英寸
#define PHONE_STICK_SIZE_INCH                   0.74f
#define TABLET_STICK_SIZE_INCH                  1.02f

MoveStick* MoveStick::create()
{
	MoveStick* ret = new (std::nothrow) MoveStick();
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

bool MoveStick::init()
{
	if (!Layer::init())
	{
		return false;
	}
    
	auto touchListener = EventListenerTouchOneByOne::create();
	touchListener->onTouchBegan = CC_CALLBACK_2(MoveStick::onTouchBegan, this);
	touchListener->onTouchMoved = CC_CALLBACK_2(MoveStick::onTouchMoved, this);
	touchListener->onTouchEnded = CC_CALLBACK_2(MoveStick::onTouchEnded, this);
	touchListener->onTouchCancelled = CC_CALLBACK_2(MoveStick::onTouchCancelled, this);
	this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(touchListener, this);
    
    Rect safeArea = Machine::instance()->getSafeAreaRect();
    m_safeInsetLeft = Utils::getSafeInsetLeft(safeArea);
    m_safeInsetBottom = Utils::getSafeInsetBottom(safeArea);
    
    float stickSizeInPoints;
    if (Machine::instance()->isTablet())
        stickSizeInPoints = Utils::convertDistanceFromInchToPoint(TABLET_STICK_SIZE_INCH);
    else
        stickSizeInPoints = Utils::convertDistanceFromInchToPoint(PHONE_STICK_SIZE_INCH);
    SpriteFrame* baseSf = SpriteFrameCache::getInstance()->getSpriteFrameByName(FRAMENAME_STICK_BASE);
    m_scale = stickSizeInPoints / baseSf->getOriginalSize().width;

	m_baseSp = Sprite::createWithSpriteFrame(baseSf);
	m_baseSp->setScale(m_scale);
	this->addChild(m_baseSp);

	m_baseCenterSp = Sprite::createWithSpriteFrameName(FRAMENAME_STICK_BASE_CENTER);
	m_baseCenterSp->setPosition(m_baseSp->getContentSize().width / 2, m_baseSp->getContentSize().height / 2);
	m_baseCenterSp->setVisible(false);
	m_baseSp->addChild(m_baseCenterSp);

	m_handleSp = Sprite::createWithSpriteFrameName(FRAMENAME_MOVE_HANDLE_NORMAL);
	m_handleSp->setScale(m_scale);
	m_handleSp->setOpacity(HANDLE_NORMAL_OPACITY);
	this->addChild(m_handleSp);

	if (sGameCenter->getLocalPlayer()->isTrainee())
	{
		m_controlGesture = MoveControlGesture::create(m_baseSp->getBoundingBox().size.width / 2);
		m_controlGesture->setScale(m_scale);
		m_controlGesture->setVisible(false);
		this->addChild(m_controlGesture);
	}

    this->initBasePosition();

#if NS_DEBUG
	m_debugDrawNode = DrawNode::create(1.0f);
	this->addChild(m_debugDrawNode);
#endif // NS_DEBUG

	return true;

}

MoveStick::MoveStick() :
    m_safeInsetLeft(0),
    m_safeInsetBottom(0),
	m_isHitted(false),
	m_scale(1.0f),
	m_maxMoveRadius(0),
	m_isMoving(false),
	m_debugDrawNode(nullptr),
	m_baseSp(nullptr),
	m_baseCenterSp(nullptr),
	m_handleSp(nullptr),
	m_isControlGestureEnabled(false),
	m_controlGesture(nullptr)
{

}

MoveStick::~MoveStick()
{
	m_debugDrawNode = nullptr;
	m_baseSp = nullptr;
	m_baseCenterSp = nullptr;
	m_handleSp = nullptr;
	m_controlGesture = nullptr;
}

void MoveStick::moveHandle(Point const& position)
{
	Point handlePos = position;

	auto visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();
	Rect r(origin.x, origin.y, visibleSize.width, visibleSize.height);
	handlePos.x = std::max(std::min(handlePos.x, r.getMaxX()), r.getMinX());
	handlePos.y = std::max(std::min(handlePos.y, r.getMaxY()), r.getMinY());

	Point diff = handlePos - m_baseSp->getPosition();
	float dist = diff.length();
	float direction = std::atan2(diff.y, diff.x);
	float max = m_baseSp->getBoundingBox().size.width / 2;

	if (dist > max)
	{
		Point offset;
		offset.x = std::cos(direction) * max;
		offset.y = std::sin(direction) * max;
		Point basePos = handlePos - offset;
		this->moveBase(basePos, offset);
	}
	else
	{
		m_handleSp->setPosition(handlePos);
		if (onMoveStickMoving)
			onMoveStickMoving(this, direction);
	}

#if DEBUG_TOUCHED_POINTS
	m_debugDrawNode->drawDot(position, 1, Color4F::BLUE);
	m_debugDrawNode->drawDot(m_handleSp->getPosition(), 1, Color4F::GREEN);
	m_debugDrawNode->drawDot(m_baseSp->getPosition(), 1, Color4F::RED);
#endif // DEBUG_TOUCHED_POINTS
}

void MoveStick::moveBase(Point const& position, Point const& handleOffset)
{
	Point handlePos = position + handleOffset;
	Point basePos = position;

	m_baseSp->setPosition(basePos);
	m_handleSp->setPosition(handlePos);

	Point diff = handlePos - basePos;
	if (diff.length() > 0)
	{
		float direction = std::atan2(diff.y, diff.x);
		if (onMoveStickMoving)
            onMoveStickMoving(this, direction);

	}
}

void MoveStick::initBasePosition()
{
    m_maxMoveRadius = (m_baseSp->getContentSize().width / 2 + m_handleSp->getContentSize().width / 2) * m_scale;
    
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    float spaceLeft = 0.f;
    float marginBottom = 0.f;
    if(Machine::instance()->isTablet())
    {
        // 如果平板是全面屏
        if(m_safeInsetBottom > 0 || m_safeInsetLeft > 0)
            spaceLeft = Utils::convertDistanceFromInchToPoint(TABLET_FULLSCREEN_SPACE_LEFT_INCH);
        else
            spaceLeft = Utils::convertDistanceFromInchToPoint(TABLET_SPACE_LEFT_INCH);
		marginBottom = TABLET_MARGIN_BOTTOM;
    }
    else
    {
        // 如果手机是全面屏
        if(m_safeInsetLeft > 0)
			spaceLeft = Utils::convertDistanceFromInchToPoint(PHONE_FULLSCREEN_SPACE_LEFT_INCH);
        else
			spaceLeft = Utils::convertDistanceFromInchToPoint(PHONE_SPACE_LEFT_INCH);
		marginBottom = PHONE_MARGIN_BOTTOM;
    }
	m_baseOriginPosition.x = origin.x + spaceLeft;
    m_baseOriginPosition.y = origin.y + marginBottom + m_maxMoveRadius;

    m_baseSp->setPosition(m_baseOriginPosition);
    m_handleSp->setPosition(m_baseOriginPosition);
	if(m_controlGesture)
		m_controlGesture->setPosition(m_baseOriginPosition);
}

void MoveStick::reset()
{
    this->moveBase(m_baseOriginPosition, Point::ZERO);
	m_handleSp->setSpriteFrame(FRAMENAME_MOVE_HANDLE_NORMAL);
	m_handleSp->setOpacity(HANDLE_NORMAL_OPACITY);
	m_baseCenterSp->setVisible(false);
	if (m_isControlGestureEnabled && m_controlGesture)
	{
		m_controlGesture->setVisible(true);
		m_controlGesture->startGestureAnimation(2.0f);
	}

	m_isHitted = false;
	m_isMoving = false;
}

void MoveStick::draw(Renderer* renderer, Mat4 const& transform, uint32_t flags)
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
	}
#endif // DEBUG_STICK
}

void MoveStick::setControlGestureEnabled(bool enabled, float animationDelay)
{
	if (!m_controlGesture || m_isControlGestureEnabled == enabled)
		return;

	m_isControlGestureEnabled = enabled;

	if (enabled)
	{
		if (!m_isHitted)
		{
			m_controlGesture->setVisible(true);
			m_controlGesture->startGestureAnimation(animationDelay);
		}
	}
	else
	{
		m_controlGesture->stopGestureAnimation();
		m_controlGesture->setVisible(false);
	}
}

bool MoveStick::hitTest(Vec2 const& pt) const
{
	Vec2 origin = Director::getInstance()->getVisibleOrigin();
	auto visibleSize = Director::getInstance()->getVisibleSize();
	Rect rect(origin.x, origin.y + m_safeInsetBottom, visibleSize.width / 2, visibleSize.height - m_safeInsetBottom);
	return rect.containsPoint(pt);
}

void MoveStick::printTouches(std::string const& eventName, std::vector<Touch*> const& touches)
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

bool MoveStick::onTouchBegan(Touch* touch, Event* event)
{
	//this->printTouches("MoveStick::onTouchBegan", { touch });

	if (this->isVisible() && !m_isHitted)
	{
		Vec2 touchLocation = this->convertTouchToNodeSpace(touch);
		if (this->hitTest(touchLocation))
		{
			this->moveBase(touchLocation, Point::ZERO);
			m_handleSp->setSpriteFrame(FRAMENAME_MOVE_HANDLE_PRESS);
			m_handleSp->setOpacity(HANDLE_PRESS_OPACITY);
			m_baseCenterSp->setVisible(true);
			if(m_isControlGestureEnabled && m_controlGesture)
				m_controlGesture->stopGestureAnimation();
			m_isHitted = true;

			return true;
		}
	}

	return false;
}

void MoveStick::onTouchMoved(Touch* touch, Event* event)
{
	//this->printTouches("MoveStick::onTouchMoved", { touch });

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

void MoveStick::onTouchEnded(Touch* touch, Event* event)
{
	//this->printTouches("MoveStick::onTouchEnded", { touch });

	this->reset();
	if (onMoveStickStopped)
		onMoveStickStopped(this);
}

void MoveStick::onTouchCancelled(Touch* touch, Event* event)
{
	//this->printTouches("MoveStick::onTouchCancelled", { touch });

	this->reset();
	if (onMoveStickStopped)
		onMoveStickStopped(this);
}

NS_END
