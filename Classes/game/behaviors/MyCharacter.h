#ifndef __MYCHARACTER_H__
#define __MYCHARACTER_H__

#include "common/utils/Timer.h"
#include "common/utils/TimeUtil.h"
#include "game/entities/DataPlayer.h"
#include "game/entities/DataCarriedItem.h"
#include "game/entities/StaminaInfo.h"
#include "game/utils/TrajectoryGenerator.h"
#include "game/movement/MyMoveSpline.h"
#include "game/client/WorldSession.h"
#include "game/movement/MyMovementGenerator.h"
#include "game/combat/MyStaminaUpdater.h"
#include "game/item/ItemCooldownProcesser.h"
#include "game/WorldListeners.h"
#include "Unit.h"


NS_BEGIN

enum AttackFlag
{
	ATTACK_FLAG_NONE			= 0,
	ATTACK_FLAG_ALL_OUT			= 1 << 0,
};

class CarriedItem;

class MyCharacter: public Unit
{
	typedef std::unordered_map<ObjectGuid, CarriedItem*> CarriedItemMap;

public:
	explicit MyCharacter(WorldSession* session);
	virtual ~MyCharacter();

	void setSession(WorldSession* session) { m_session = session; }
	WorldSession* getSession() const { return m_session; }

	bool canRemoveFromWorld() const override { return false; }
	void cleanupBeforeDelete() override;
	void activate() override;
	void inactivate() override;

	void setDeathState(DeathState state) override;

	void charge();
	void chargeStop();
	bool isInCharge() const;
	void setFullStamina();
	MyStaminaUpdater* getStaminaUpdater() const { return static_cast<MyStaminaUpdater*>(Unit::getStaminaUpdater()); }
	void buildStaminaInfo(StaminaInfo& stamina) const;
	void resetStaminaSyncState();
	void startRegenStamina();

	bool moveBy(float direction);
	void moveStop();
	void setMoveEnabled(bool enabled);
	bool isMoveEnabled() const { return m_isMoveEnabled; }
	MyMoveSpline* getMoveSpline() const { return static_cast<MyMoveSpline*>(Unit::getMoveSpline()); }
	MyMovementGenerator* getMovementGenerator() const { return static_cast<MyMovementGenerator*>(Unit::getMovementGenerator()); }
	void stopMoving() override;
	void resetMoveSyncState();
	void setMoveTurnAngle(float rad);
	void buildMovementInfo(MovementInfo& movement) const;

    void lockFacingDirection();
    void unlockFacingDirection();

	bool canHandDown() const;
	void startHandDownTimer();
	void stopHandDownTimer();
	bool isHandDownTimerEnabled() const { return m_isHandDownTimerEnabled; }
	
	bool attack(float direction);
	bool attackStop() override;
    void combatStop() override;
	uint32 getAttackInfoCounter() const { return m_attackInfoCounter; }
	bool isAttackTimerEnabled() const { return m_isAttackTimerEnabled; }
	bool getClosestTargetPosition(cocos2d::Point& position);

	DataBasic* loadData(ObjectGuid const& guid);
	DataPlayer* getData() override { return static_cast<DataPlayer*>(m_data); }
	DataPlayer const* getData() const override { return static_cast<DataPlayer*>(m_data); }
	void updateObject(UpdateType updateType, uint32 updateFlags, DataBasic* data) override;

	void update(float delta) override;

	void sendMoveTurn();
	void sendMoveSync();

	PickupStatus canStoreItem(ItemTemplate const* itemTemplate) const;

	void addCarriedItem(CarriedItem* item);
	void removeCarriedItem(ObjectGuid const& guid, bool cleanup);
	// Find item carried with the specified guid in the character
	CarriedItem* findCarriedItem(ObjectGuid const& guid, bool includeInactiveItems = false) const;

	ItemCooldownProcesser* getItemCooldownProcesser() const { return m_itemCooldownProcesser; }

	void setInventoryItemLifecycleListener(InventoryItemLifecycleListener* listener) { m_inventoryItemLifecycleListener = listener; }
	void setEquipmentItemLifecycleListener(EquipmentItemLifecycleListener* listener) { m_equipmentItemLifecycleListener = listener; }

private:
	void sendAttackInfo(float direction, uint32 flags);
	void updateAttackTimer(float delta);
	void attackDelayed(float direction, uint32 flags, int32 delay);
	void stopAttackTimer();
	void updateHandDownTimer(float delta);
	void updateFacingDirectionUnlockTimer(float delta);

	PickupStatus canStoreItemInInventoryCustomSlots(ItemTemplate const* itemTemplate) const;
	PickupStatus canStoreStackableItemInSpecificSlot(int32 slot, ItemTemplate const* itemTemplate) const;
	PickupStatus canStoreItemInEquipmentSlot(ItemTemplate const* itemTemplate) const;
	void updateItemAvailabilitiesOnMap();
	
	void activateCarriedItem(ObjectGuid const& guid, int32 slot);
	void activateCarriedItemsInSlots();
	void removeAllCarriedItems();
	void inactivateAllCarriedItems();
	void updateCarriedItems(float delta);
	void clearInactiveCarriedItems();
	CarriedItem* removeIfExistsInInactiveCarriedItems(ObjectGuid const& guid);

	void notifyCarriedItemDestroyed(CarriedItem* item);
	void notifyCarriedItemInactivated(CarriedItem* item);
	void notifyCarriedItemActivated(CarriedItem* item);

	WorldSession* m_session;
	bool m_isMoveEnabled;

	DelayTimer m_attackTimer;
	bool m_isAttackTimerEnabled;
	float m_direction;
	uint32 m_attackFlags;
	uint32 m_attackInfoCounter;

	bool m_isHandDownTimerEnabled;
	DelayTimer m_handDownTimer;

	bool m_isFacingDirectionLocked;
	DelayTimer m_facingDirectionUnlockTimer;

	bool m_isItemAvailabilitiesChanged;

	CarriedItemMap m_carriedItems;
	CarriedItemMap m_inactiveCarriedItems;
	ItemCooldownProcesser* m_itemCooldownProcesser;
	InventoryItemLifecycleListener* m_inventoryItemLifecycleListener;
	EquipmentItemLifecycleListener* m_equipmentItemLifecycleListener;
};

NS_END

#endif // __MYCHARACTER_H__