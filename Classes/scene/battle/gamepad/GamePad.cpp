#include "GamePad.h"

#include "common/Machine.h"


NS_BEGIN

#if NS_DEBUG

#define DEBUG_GAME_PAD						0

#endif // NS_DEBUG

GamePad* GamePad::create(ControllerType type)
{
	auto ret = new (std::nothrow) GamePad();
	if (ret && ret->init(type))
	{
		ret->autorelease();
	}
	else
	{
		CC_SAFE_DELETE(ret);
	}

	return ret;
}

bool GamePad::init(ControllerType type)
{
	if (!Layer::init())
		return false;

	m_controllerType = type;

	Machine::instance()->setMultipleTouchEnabled(true);

	switch (type)
	{
	case CONTROLLER_TYPE_KEYBOARD_MOUSE:
		this->initKeyboardMouse();
		break;
	default:
		this->initDualSticks();
		break;
	}

#if DEBUG_GAME_PAD
	m_debugDrawNode = DrawNode::create(1.0f);
	this->addChild(m_debugDrawNode);
	m_debugDrawNode->drawLine(Vec2(origin.x + visibleSize.width / 2, origin.y), Vec2(origin.x + visibleSize.width / 2, origin.y + visibleSize.height), Color4F::YELLOW);
#endif // DEBUG_GAME_PAD

	return true;
}

GamePad::GamePad() :
	m_controllerType(CONTROLLER_TYPE_DUAL_STICKS),
	m_listener(nullptr),
	m_keyboardMouse(nullptr),
	m_moveStick(nullptr),
	m_attackStick(nullptr)
{

}

GamePad::~GamePad()
{
	Machine::instance()->setMultipleTouchEnabled(false);

	m_keyboardMouse = nullptr;
	m_moveStick = nullptr;
	m_attackStick = nullptr;
	m_listener = nullptr;
}

void GamePad::setChargeEnabled(bool enabled)
{
	switch (m_controllerType)
	{
	case CONTROLLER_TYPE_DUAL_STICKS:
		m_attackStick->setChargeEnabled(enabled);
		break;
	case CONTROLLER_TYPE_KEYBOARD_MOUSE:
		m_keyboardMouse->setChargeEnabled(enabled);
		break;
	default:
		break;
	}
}

void GamePad::onKeyboardMouseMoving(Ref* sender, float direction)
{
	if (m_listener)
		m_listener->onMoveControlMoving(this, direction);
}

void GamePad::onKeyboardMouseMoveStopped(Ref* sender)
{
	if (m_listener)
		m_listener->onMoveControlStopped(this);
}

void GamePad::onKeyboardMouseFire(Ref* sender, Point const& aimPoint)
{
	if (m_listener)
		return m_listener->onAttackControlFire(this, aimPoint);
}

void GamePad::onKeyboardMouseCharging(Ref* sender)
{
	if (m_listener)
		m_listener->onAttackControlCharging(this);
}

void GamePad::onMoveStickMoving(Ref* sender, float direction)
{
	if (m_listener)
		m_listener->onMoveControlMoving(this, direction);
}

void GamePad::onMoveStickStopped(Ref* sender)
{
	if (m_listener)
		m_listener->onMoveControlStopped(this);
}

void GamePad::onAttackStickAiming(Ref* sender, float direction)
{
	if (m_listener)
		m_listener->onAttackControlAiming(this, direction);
}

void GamePad::onAttackStickCanceling(Ref* sender)
{
	if (m_listener)
		m_listener->onAttackControlCanceling(this);
}

void GamePad::onAttackStickCanceled(Ref* sender)
{
	if (m_listener)
		m_listener->onAttackControlCanceled(this);
}

void GamePad::onAttackStickFire(Ref* sender, float direction)
{
	if (m_listener)
		m_listener->onAttackControlFire(this, direction);
}

void GamePad::onAttackStickCharging(Ref* sender)
{
	if (m_listener)
		m_listener->onAttackControlCharging(this);
}

void GamePad::initKeyboardMouse()
{
	m_keyboardMouse = KeyboardMouse::create();
	m_keyboardMouse->onKeyboardMouseMoving = CC_CALLBACK_2(GamePad::onKeyboardMouseMoving, this);
	m_keyboardMouse->onKeyboardMouseMoveStopped = CC_CALLBACK_1(GamePad::onKeyboardMouseMoveStopped, this);
	m_keyboardMouse->onKeyboardMouseFire = CC_CALLBACK_2(GamePad::onKeyboardMouseFire, this);
	m_keyboardMouse->onKeyboardMouseCharging = CC_CALLBACK_1(GamePad::onKeyboardMouseCharging, this);
	this->addChild(m_keyboardMouse);
}

void GamePad::initDualSticks()
{
	m_moveStick = MoveStick::create();
	m_moveStick->onMoveStickMoving = CC_CALLBACK_2(GamePad::onMoveStickMoving, this);
	m_moveStick->onMoveStickStopped = CC_CALLBACK_1(GamePad::onMoveStickStopped, this);
	this->addChild(m_moveStick);

	m_attackStick = AttackStick::create();
	m_attackStick->onAttackStickAiming = CC_CALLBACK_2(GamePad::onAttackStickAiming, this);
	m_attackStick->onAttackStickCanceling = CC_CALLBACK_1(GamePad::onAttackStickCanceling, this);
	m_attackStick->onAttackStickCanceled = CC_CALLBACK_1(GamePad::onAttackStickCanceled, this);
	m_attackStick->onAttackStickFire = CC_CALLBACK_2(GamePad::onAttackStickFire, this);
	m_attackStick->onAttackStickCharging = CC_CALLBACK_1(GamePad::onAttackStickCharging, this);
	this->addChild(m_attackStick);
}

NS_END

