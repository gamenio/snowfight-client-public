#ifndef __ATTACK_STICK_H__
#define __ATTACK_STICK_H__

#include "common/Common.h"
#include "AttackControlGesture.h"

USING_NS_CC;

NS_BEGIN

typedef std::function<void(Ref*, float direction)> AttackStickAimingCallback;
typedef std::function<void(Ref*, float direction)> AttackStickFireCallback;
typedef std::function<void(Ref*)> AttackStickChargingCallback;
typedef std::function<void(Ref*)> AttackStickCancelingCallback;
typedef std::function<void(Ref*)> AttackStickCanceledCallback; 

class AttackStick : public cocos2d::Layer
{
public:
    static AttackStick* create();
    virtual bool init() override;

	AttackStick();
	~AttackStick();

	void setChargeEnabled(bool enabled);
	bool isChargeEnabled() const { return m_isChargeEnabled; }

	bool onTouchBegan(Touch* touch, Event* event) override;
	void onTouchMoved(Touch* touch, Event* event) override;
	void onTouchEnded(Touch* touch, Event* event) override;
	void onTouchCancelled(Touch* touch, Event* event) override;

	AttackStickAimingCallback onAttackStickAiming;
	AttackStickFireCallback onAttackStickFire;
	AttackStickChargingCallback onAttackStickCharging;
	AttackStickCancelingCallback onAttackStickCanceling;
	AttackStickCanceledCallback onAttackStickCanceled;

	void update(float delta) override;
	void draw(Renderer *renderer, Mat4 const& transform, uint32_t flags) override;

	void setControlGestureEnabled(AttackControlGesture::GestureType gesture, float animationDelay = 0.f);

private:
	bool hitTest(Vec2 const& pt) const;
	void printTouches(std::string const& eventName, std::vector<Touch*> const& touches);

	void moveHandle(cocos2d::Point const& position);
	void moveBase(cocos2d::Point const& position);
    void initBasePosition();
	void reset();

	void startHandleZoomAnimation();
	void stopHandleZoomAnimation();

    float m_safeInsetRight;
    float m_safeInsetBottom;
    
	bool m_isHitted;
	NSTime m_pressedTime;
	float m_scale;
    Point m_baseOriginPosition;
	float m_maxMoveRadius;

	bool m_isChargeEnabled;
	float m_direction;
	bool m_isAiming;
    bool m_isMoving;
	bool m_isCharging;
	bool m_isCanceling;

	DrawNode* m_debugDrawNode;
	Sprite* m_baseSp;
	Sprite* m_baseCenterSp;
	Sprite* m_handleSp;

	AttackControlGesture::GestureType m_gestureType;
	AttackControlGesture* m_controlGesture;
};

NS_END

#endif // __ATTACK_STICK_H__
