#ifndef __GAME_PAD_H__
#define __GAME_PAD_H__

#include "common/Common.h"
#include "game/entities/DataPlayer.h"
#include "KeyboardMouse.h"
#include "AttackStick.h"
#include "MoveStick.h"

USING_NS_CC;

NS_BEGIN

class GamePadListener
{
public:
	virtual void onMoveControlMoving(Ref* sender, float direction) {}
	virtual void onMoveControlStopped(Ref* sender) {}

	virtual void onAttackControlAiming(Ref* sender, float direction) {}
	virtual void onAttackControlFire(Ref* sender, float direction) {}
	virtual void onAttackControlFire(Ref* sender, cocos2d::Point const& aimPoint) {}
	virtual void onAttackControlCharging(Ref* sender) {}
	virtual void onAttackControlCanceling(Ref* sender) {}
	virtual void onAttackControlCanceled(Ref* sender) {}
};


class GamePad : public Layer
{
public:
	static GamePad* create(ControllerType type);
	bool init(ControllerType type);

	GamePad();
	virtual ~GamePad();

	void setGamePadListener(GamePadListener* listener) { m_listener = listener; }

	MoveStick* getMoveStick() const { return m_moveStick; }
	AttackStick* getAttackStick() const { return m_attackStick; }

	void setChargeEnabled(bool enabled);

private:
	void onKeyboardMouseMoving(Ref* sender, float direction);
	void onKeyboardMouseMoveStopped(Ref* sender);
	void onKeyboardMouseFire(Ref* sender, cocos2d::Point const& aimPoint);
	void onKeyboardMouseCharging(Ref* sender);

	void onMoveStickMoving(Ref* sender, float direction);
	void onMoveStickStopped(Ref* sender);

	void onAttackStickAiming(Ref* sender, float direction);
	void onAttackStickCanceling(Ref* sender);
	void onAttackStickCanceled(Ref* sender);
	void onAttackStickFire(Ref* sender, float direction);
	void onAttackStickCharging(Ref* sender);

	void initKeyboardMouse();
	void initDualSticks();

	ControllerType m_controllerType;
	GamePadListener* m_listener;
	KeyboardMouse* m_keyboardMouse;
	MoveStick* m_moveStick;
	AttackStick* m_attackStick;
	DrawNode* m_debugDrawNode;
};

NS_END

#endif // __GAME_PAD_H__
