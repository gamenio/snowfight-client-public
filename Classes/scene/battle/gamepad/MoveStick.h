#ifndef __MOVE_STICK_H__
#define __MOVE_STICK_H__

#include "common/Common.h"
#include "MoveControlGesture.h"

USING_NS_CC;

NS_BEGIN

typedef std::function<void(Ref*, float direction)> MoveStickMovingCallback;
typedef std::function<void(Ref*)> MoveStickStoppedCallback;

class MoveStick : public cocos2d::Layer
{
public:
    static MoveStick* create();
    virtual bool init() override;

	MoveStick();
	~MoveStick();

	bool onTouchBegan(Touch* touch, Event* event) override;
	void onTouchMoved(Touch* touch, Event* event) override;
	void onTouchEnded(Touch* touch, Event* event) override;
	void onTouchCancelled(Touch* touch, Event* event) override;

	MoveStickMovingCallback onMoveStickMoving;
	MoveStickStoppedCallback onMoveStickStopped;

	void draw(Renderer *renderer, Mat4 const& transform, uint32_t flags) override;

	void setControlGestureEnabled(bool enabled, float animationDelay = 0.f);

private:
	bool hitTest(Vec2 const& pt) const;
	void printTouches(std::string const& eventName, std::vector<Touch*> const& touches);

	void moveHandle(cocos2d::Point const& position);
	void moveBase(cocos2d::Point const& position, cocos2d::Point const& handleOffset);
    void initBasePosition();
	void reset();

    float m_safeInsetLeft;
    float m_safeInsetBottom;
    
	bool m_isHitted;
	float m_scale;
    Point m_baseOriginPosition;
	float m_maxMoveRadius;
	bool m_isMoving;

	DrawNode* m_debugDrawNode;
	Sprite* m_baseSp;
	Sprite* m_baseCenterSp;
	Sprite* m_handleSp;

	bool m_isControlGestureEnabled;
	MoveControlGesture* m_controlGesture;
};

NS_END

#endif // __MOVE_STICK_H__
