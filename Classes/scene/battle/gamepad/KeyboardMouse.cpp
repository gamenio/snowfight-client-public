#include "KeyboardMouse.h"

#include "common/utils/MathTools.h"
#include "common/utils/TimeUtil.h"
#include "common/debugging/DebugDrawer.h"

NS_BEGIN

#if NS_DEBUG
#define DEBUG_KEYBOARD_MOUSE				0
#define USE_ARROW_KEYS						0

#endif // NS_DEBUG

// 按键方向的角度
#define DIRECTION_UP			(float)(M_PI_2)
#define DIRECTION_DOWN			(float)(3 * M_PI_2)
#define DIRECTION_LEFT			(float)(M_PI)
#define DIRECTION_RIGHT			0.f
#define DIRECTION_LEFT_UP		(float)(M_PI_2 + M_PI_2 / 2)
#define DIRECTION_RIGHT_UP		(float)(M_PI_2 / 2)
#define DIRECTION_LEFT_DOWN		(float)(M_PI + M_PI_2 / 2)
#define DIRECTION_RIGHT_DOWN	(float)(M_PI * 2 - M_PI_2 / 2)

// 使用箭头键控制方向
#if USE_ARROW_KEYS
#define KEY_UP					EventKeyboard::KeyCode::KEY_UP_ARROW
#define KEY_DOWN				EventKeyboard::KeyCode::KEY_DOWN_ARROW
#define KEY_LEFT				EventKeyboard::KeyCode::KEY_LEFT_ARROW
#define KEY_RIGHT				EventKeyboard::KeyCode::KEY_RIGHT_ARROW

#else // 使用WASD键控制方向
#define KEY_UP					EventKeyboard::KeyCode::KEY_W
#define KEY_DOWN				EventKeyboard::KeyCode::KEY_S
#define KEY_LEFT				EventKeyboard::KeyCode::KEY_A
#define KEY_RIGHT				EventKeyboard::KeyCode::KEY_D

#endif // USE_ARROW_KEYS

#define CHARGE_STATE_DELAY					500			// 蓄力状态的延迟，单位：毫秒

KeyboardMouse* KeyboardMouse::create()
{
	KeyboardMouse* ret = new (std::nothrow) KeyboardMouse();
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

bool KeyboardMouse::init()
{
	if (!Layer::init())
	{
		return false;
	}

	auto mouseListener = EventListenerMouse::create();
	mouseListener->onMouseDown = CC_CALLBACK_1(KeyboardMouse::onMouseDown, this);
	mouseListener->onMouseUp = CC_CALLBACK_1(KeyboardMouse::onMouseUp, this);
	mouseListener->onMouseMove = CC_CALLBACK_1(KeyboardMouse::onMouseMove, this);
	mouseListener->onMouseScroll = CC_CALLBACK_1(KeyboardMouse::onMouseScroll, this);
	this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(mouseListener, this);

	auto keyboardListener = EventListenerKeyboard::create();
	keyboardListener->onKeyPressed = CC_CALLBACK_2(KeyboardMouse::onKeyPressed, this);
	keyboardListener->onKeyReleased = CC_CALLBACK_2(KeyboardMouse::onKeyReleased, this);
	this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(keyboardListener, this);

	auto touchListener = EventListenerTouchOneByOne::create();
	touchListener->onTouchBegan = CC_CALLBACK_2(KeyboardMouse::onTouchBegan, this);
	touchListener->onTouchMoved = CC_CALLBACK_2(KeyboardMouse::onTouchMoved, this);
	touchListener->onTouchEnded = CC_CALLBACK_2(KeyboardMouse::onTouchEnded, this);
	touchListener->onTouchCancelled = CC_CALLBACK_2(KeyboardMouse::onTouchCancelled, this);
	this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(touchListener, this);

#if NS_DEBUG
	m_debugDrawNode = DrawNode::create(1.0f);
	this->addChild(m_debugDrawNode);
#endif // NS_DEBUG

	this->scheduleUpdate();

	return true;

}

KeyboardMouse::KeyboardMouse() :
	m_pressedTime(0),
	m_pressedMouseButton(EventMouse::MouseButton::BUTTON_UNSET),
	isTouched(false),
	m_isChargeEnabled(false),
	m_isCharging(false),
	m_debugDrawNode(nullptr)
{

}

KeyboardMouse::~KeyboardMouse()
{    
	m_debugDrawNode = nullptr;
}

void KeyboardMouse::setChargeEnabled(bool enabled)
{
	m_isChargeEnabled = enabled;
}

void KeyboardMouse::onKeyPressed(EventKeyboard::KeyCode keyCode, Event* event)
{
	//CCLOG("onKeyPressed :%d", keyCode);
	if(this->pressDirectionKey(keyCode))
		m_pressedKeys.push_back(keyCode);
}

void KeyboardMouse::onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event)
{
	//CCLOG("onKeyReleased :%d", keyCode);
	auto it = std::find(m_pressedKeys.begin(), m_pressedKeys.end(), keyCode);
	if (it != m_pressedKeys.end())
	{
//		auto releasedKey = *it;
		auto currKey = m_pressedKeys.back();
		it = m_pressedKeys.erase(it);
		if (!m_pressedKeys.empty())
		{
			if (it == m_pressedKeys.end())
				this->pressDirectionKey(m_pressedKeys.back());
			else
				this->pressDirectionKey(currKey);
		}
		else
		{
			if (onKeyboardMouseMoveStopped)
				onKeyboardMouseMoveStopped(this);
		}
	}
}

bool KeyboardMouse::pressDirectionKey(EventKeyboard::KeyCode keyCode)
{
	auto prevKey = EventKeyboard::KeyCode::KEY_NONE;
	if (!m_pressedKeys.empty())
	{
		auto rIt = m_pressedKeys.rbegin();
		if (*rIt == keyCode)
			++rIt;
		if(rIt != m_pressedKeys.rend())
			prevKey = *rIt;
	}

	bool isDirKey = true;
	float direction = 0.f;

	switch (keyCode)
	{
	case KEY_UP:
		if(prevKey == KEY_LEFT)
			direction = DIRECTION_LEFT_UP;
		else if(prevKey == KEY_RIGHT)
			direction = DIRECTION_RIGHT_UP;
		else
			direction = DIRECTION_UP;
		break;
	case KEY_DOWN:
		if (prevKey == KEY_LEFT)
			direction = DIRECTION_LEFT_DOWN;
		else if (prevKey == KEY_RIGHT)
			direction = DIRECTION_RIGHT_DOWN;
		else
			direction = DIRECTION_DOWN;
		break;
	case KEY_LEFT:
		if (prevKey == KEY_UP)
			direction = DIRECTION_LEFT_UP;
		else if (prevKey == KEY_DOWN)
			direction = DIRECTION_LEFT_DOWN;
		else
			direction = DIRECTION_LEFT;
		break;
	case KEY_RIGHT:
		if (prevKey == KEY_UP)
			direction = DIRECTION_RIGHT_UP;
		else if (prevKey == KEY_DOWN)
			direction = DIRECTION_RIGHT_DOWN;
		else
			direction = DIRECTION_RIGHT;
		break;
	default:
		isDirKey = false;
		break;
	}

	if (isDirKey)
	{
		if (onKeyboardMouseMoving)
			onKeyboardMouseMoving(this, direction);
	}

	return isDirKey;
}

void KeyboardMouse::update(float delta)
{
	if (m_pressedMouseButton == EventMouse::MouseButton::BUTTON_LEFT && m_isChargeEnabled && !m_isCharging)
	{
		NSTime diff = time_util::getUptimeMillis() - m_pressedTime;
		if (diff >= CHARGE_STATE_DELAY)
		{
			if (onKeyboardMouseCharging)
				onKeyboardMouseCharging(this);

			m_isCharging = true;
		}
	}
}

void KeyboardMouse::draw(Renderer* renderer, Mat4 const& transform, uint32_t flags)
{
	Layer::draw(renderer, transform, flags);

#if DEBUG_KEYBOARD_MOUSE
	if (m_debugDrawNode)
	{
		m_debugDrawNode->clear();
	}
#endif // DEBUG_KEYBOARD_MOUSE
}

void KeyboardMouse::onMouseDown(EventMouse* event)
{
	Point touchLocation = this->convertToNodeSpace(event->getLocationInView());
	CC_UNUSED_PARAM(touchLocation);
	//CCLOG("KeyboardMouse::onMouseDown location: [%f,%f] button: %d", touchLocation.x, touchLocation.y, event->getMouseButton());

	// 如果触控事件被截获则忽略鼠标左键按下事件
	if (event->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT && !isTouched)
		return;

	if (m_pressedMouseButton == EventMouse::MouseButton::BUTTON_UNSET)
	{
		m_pressedTime = time_util::getUptimeMillis();
		m_pressedMouseButton = event->getMouseButton();
	}
}

void KeyboardMouse::onMouseUp(EventMouse* event)
{
	Point touchLocation = this->convertToNodeSpace(event->getLocationInView());
	//CCLOG("KeyboardMouse::onMouseUp location: [%f,%f] button: %d", touchLocation.x, touchLocation.y, event->getMouseButton());

	if (m_pressedMouseButton == event->getMouseButton())
	{
		switch (m_pressedMouseButton)
		{
		case EventMouse::MouseButton::BUTTON_RIGHT:
		case EventMouse::MouseButton::BUTTON_LEFT:
		{
			Point aimPoint(-1, -1);
			if (m_isCharging || m_pressedMouseButton == EventMouse::MouseButton::BUTTON_RIGHT)
				aimPoint = touchLocation;
			if (onKeyboardMouseFire)
				onKeyboardMouseFire(this, aimPoint);
			break;
		}
		default:
			break;
		}

		m_isCharging = false;
		m_pressedTime = 0;
		m_pressedMouseButton = EventMouse::MouseButton::BUTTON_UNSET;
	}
}

void KeyboardMouse::onMouseMove(EventMouse* event)
{
	//CCLOG("KeyboardMouse::onMouseMove");
}

void KeyboardMouse::onMouseScroll(EventMouse* event)
{
	//CCLOG("KeyboardMouse::onMouseScroll");
}

bool KeyboardMouse::onTouchBegan(Touch* touch, Event* event)
{
	//CCLOG("KeyboardMouse::onTouchBegan");
	isTouched = true;
	return true;
}

void KeyboardMouse::onTouchMoved(Touch* touch, Event* event)
{
	//CCLOG("KeyboardMouse::onTouchMoved");
}

void KeyboardMouse::onTouchEnded(Touch* touch, Event* event)
{
	//CCLOG("KeyboardMouse::onTouchEnded");
	isTouched = false;
}

void KeyboardMouse::onTouchCancelled(Touch* touch, Event* event)
{
	//CCLOG("KeyboardMouse::onTouchCancelled");
	isTouched = false;
}

NS_END
