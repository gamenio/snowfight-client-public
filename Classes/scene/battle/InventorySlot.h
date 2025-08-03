#ifndef __INVENTORY_SLOT_H__
#define __INVENTORY_SLOT_H__

#include "common/Common.h"
#include "scene/gamble/CarriedProp.h"
#include "game/WorldListeners.h"

USING_NS_CC;

NS_BEGIN

class InventorySlot : public Node
{
	enum CooldownState
	{
		COOLDOWN_STATE_NONE,
		COOLDOWN_STATE_PENDING,
		COOLDOWN_STATE_STARTED,
	};
public:
	enum SlotType
	{
		FIRST_AID,
		CUSTOM,
	};

	static InventorySlot* create(SlotType type);

	InventorySlot();
	virtual ~InventorySlot();

	bool init(SlotType type);

	void update(float delta) override;

	void setEnabled(bool enabled);
	bool isEnabled() const { return m_isEnabled; }

	void setProp(CarriedProp* prop);
	void removeProp();
	CarriedProp* getProp() const { return m_prop; }

	void onPropActivated();
	void onPropInactivated();

	void onItemUseResult(ItemUseStatus status);

	bool onTouchBegan(Touch* touch, Event* event);
	void onTouchMoved(Touch* touch, Event* event);
	void onTouchEnded(Touch* touch, Event* event);
	void onTouchCancelled(Touch* touch, Event* event);

private:
	bool hitTest(Vec2 const& pt, Camera const* camera, Vec3* p) const;

	void changeToNormalState();
	void changeToPressedState();

	void startCooldownProgress(float elapsed, float duration);
	void stopCooldownProgress();
	void onCooldownCompleted();

	void clearAfterPropInactivated();

	void updatePropCount(bool force);
	void updateCooldown();

	SlotType m_type;
	CooldownState m_cooldownState;

	bool m_isEnabled;
	EventListenerTouchOneByOne* m_touchListener;
	bool m_hitted;
	Camera const* m_hittedByCamera;

	Sprite* m_bgSp;
	Sprite* m_fgSp;
	CarriedProp* m_prop;
	Label* m_countLabel;
	ProgressTimer* m_progTimer;
};

NS_END

#endif // __INVENTORY_SLOT_H__
