#ifndef __PROP_H__
#define __PROP_H__

#include "common/Common.h"
#include "game/entities/DataItem.h"
#include "scene/actions/BezierCurve.h"
#include "scene/gui/silhouette/SILSprite.h"
#include "scene/gui/silhouette/SILLabel.h"
#include "GameObject.h"
#include "PickupProgressBar.h"

USING_NS_CC;

NS_BEGIN

struct BounceConfig
{
	float riseDuration;
	float fallDuration;
	Vec2 deltaPosition;
};

class Prop : public GameObject
{
	enum GoldStackSize
	{
		GOLD_STACK_TINY						= 0,
		GOLD_STACK_SMALL,
		GOLD_STACK_MEDIUM,
		GOLD_STACK_LARGE,
		GOLD_STACK_HUGE,
	};

	enum MagicBeanStyle
	{
		MAGICBEAN_SINGLE					= 0,
		MAGICBEAN_STACKED,
	};

public:
	Prop();
	virtual ~Prop();

	static Prop* createWithData(DataItem* data);
	virtual bool initWithData(DataItem* data);

	void onActivated() override;
	void onInactivated() override;

	void update(float delta) override;
	void setGlobalZOrder(float globalZOrder) override;

	void onItemPickupResult(PickupStatus status, NSTime remainingTime);

	DataItem* getData() const override { return m_data; }
    
private:
	void launch();
	void reset();

	void displayGold(ItemTemplate const* tmpl);
	void displayMagicBean(ItemTemplate const* tmpl);
	void displayOtherProp(ItemTemplate const* tmpl);

	void startDropAnimation();
	void stopDropAnimation();
	void onDropAnimationEnded();
	void playDropSound();

	void startJumpAnimation();
	void stopJumpAnimation(bool restore, bool delayed);
	bool isJumpAnimationRunning() const;
	void onJumpCompleted();

	void showPickupProgress(NSTime elapsedTime, NSTime duration);
	void hidePickupProgress();

	void showForbiddenSymbol();
	void hideForbiddenSymbol();

    void updatePosition();
	void updateShadow();
	void updateAvailability(bool force);
	void updatePickupProgressVisibility(bool force);

	GoldStackSize getGoldStackSize(int32 golds) const;
	void startBounceAnimation(Node* node, BounceConfig const& config, float elapsed);

	void debugDraw();

	DataItem* m_data;
	bool m_isJumpAnimationStopping;

	ActionInterval* m_parabola;
	Node* m_mainNode;
	SILSprite* m_shadowSp;
	SILSprite* m_levelSp;
	PickupProgressBar* m_pickupProgressBar;
	SILSprite* m_forbiddenSp;
	SILLabel* m_nameLabel;
};


NS_END


#endif // __PROP_H__