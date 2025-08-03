#ifndef __KEYBOARD_MOUSE_H__
#define __KEYBOARD_MOUSE_H__

#include "common/Common.h"

USING_NS_CC;

NS_BEGIN

typedef std::function<void(Ref*, float direction)> KeyboardMouseMovingCallback;
typedef std::function<void(Ref*)> KeyboardMouseMoveStoppedCallback;
typedef std::function<void(Ref*, cocos2d::Point const& aimPoint)> KeyboardMouseFireCallback;
typedef std::function<void(Ref*)> KeyboardMouseChargingCallback;

class KeyboardMouse : public cocos2d::Layer
{
public:
    static KeyboardMouse* create();
    virtual bool init() override;

	KeyboardMouse();
	~KeyboardMouse();

	void setChargeEnabled(bool enabled);
	bool isChargeEnabled() const { return m_isChargeEnabled; }

	void onKeyPressed(EventKeyboard::KeyCode keyCode, Event* event) override;
	void onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event) override;

	void onMouseDown(EventMouse* event);
	void onMouseUp(EventMouse* event);
	void onMouseMove(EventMouse* event);
	void onMouseScroll(EventMouse* event);

	bool onTouchBegan(Touch* touch, Event* event) override;
	void onTouchMoved(Touch* touch, Event* event) override;
	void onTouchEnded(Touch* touch, Event* event) override;
	void onTouchCancelled(Touch* touch, Event* event) override;

	KeyboardMouseMovingCallback onKeyboardMouseMoving;
	KeyboardMouseMoveStoppedCallback onKeyboardMouseMoveStopped;
	KeyboardMouseFireCallback onKeyboardMouseFire;
	KeyboardMouseChargingCallback onKeyboardMouseCharging;

	void update(float delta) override;
	void draw(Renderer *renderer, Mat4 const& transform, uint32_t flags) override;

private:
	bool pressDirectionKey(EventKeyboard::KeyCode keyCode);

	std::vector<EventKeyboard::KeyCode> m_pressedKeys;
	NSTime m_pressedTime;
	EventMouse::MouseButton m_pressedMouseButton;
	bool isTouched;
	bool m_isChargeEnabled;
	bool m_isCharging;

	DrawNode* m_debugDrawNode;
};

NS_END

#endif // __KEYBOARD_MOUSE_H__
