#ifndef __MOVE_CONTROL_GESTURE_H__
#define __MOVE_CONTROL_GESTURE_H__

#include "common/Common.h"

USING_NS_CC;

NS_BEGIN

class MoveControlGesture : public cocos2d::Node
{
public:
    static MoveControlGesture* create(float maxMoveRadius);
    virtual bool init(float maxMoveRadius);

	MoveControlGesture();
	~MoveControlGesture();

	void startGestureAnimation(float delay = 0.f);
	void stopGestureAnimation();

private:
	float m_maxMoveRadius;

	Sprite* m_handleSp;
	Sprite* m_thumbSp;
};

NS_END

#endif // __MOVE_CONTROL_GESTURE_H__
