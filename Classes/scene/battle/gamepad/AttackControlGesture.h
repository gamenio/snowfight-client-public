#ifndef __ATTACK_CONTROL_GESTURE_H__
#define __ATTACK_CONTROL_GESTURE_H__

#include "common/Common.h"

USING_NS_CC;

NS_BEGIN

class AttackControlGesture : public cocos2d::Node
{
public:
	enum GestureType
	{
		GESTURE_TYPE_NONE,
		GESTURE_TYPE_DRAG,
		GESTURE_TYPE_LONG_PRESS,
	};

    static AttackControlGesture* create(float maxMoveRadius);
    virtual bool init(float maxMoveRadius);

	AttackControlGesture();
	~AttackControlGesture();

	void startGestureAnimation(GestureType gesture, float delay = 0.f);
	void stopGestureAnimation();

private:
	void startDragAnimation(float delay);
	void startLongPressAnimation(float delay);

	float m_maxMoveRadius;

	Sprite* m_handleSp;
	Sprite* m_thumbSp;
	Sprite* m_clockDialSp;
	Sprite* m_clockPointerSp;
};

NS_END

#endif // __ATTACK_CONTROL_GESTURE_H__
