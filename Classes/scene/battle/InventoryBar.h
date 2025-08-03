#ifndef __INVENTORY_BAR_H__
#define __INVENTORY_BAR_H__

#include "common/Common.h"
#include "game/WorldListeners.h"
#include "scene/gamble/CarriedProp.h"
#include "InventorySlot.h"

USING_NS_CC;

NS_BEGIN


class InventoryBar : public Node, InventoryItemLifecycleListener,
								  CarriedItemListener
{
public:
	static InventoryBar* create();

	InventoryBar();
	~InventoryBar();

	bool init() override;

	void update(float delta) override;
	void cleanAfterUpdate();

	void setEnabled(bool enabled);
	bool isEnabled() const { return m_isEnabled; }

	// InventoryItemLifecycleListener
	void onInventoryItemDestroyed(ObjectGuid const& guid) override;
	void onInventoryItemActivated(DataCarriedItem* data) override;
	void onInventoryItemInactivated(ObjectGuid const& guid) override;

	// CarriedItemListener
	void onItemUseResult(ItemUseResult const& result) override;

private:
	void initSlots();

	CarriedProp* findCarriedProp(ObjectGuid const& guid) const;
	InventorySlot* getInventorySlot(int32 slot) const;

	bool m_isEnabled;
	Map<ObjectGuid, CarriedProp*> m_carriedProps;
	std::array<InventorySlot*, INVENTORY_SLOTS_COUNT> m_inventorySlots;
  
};

NS_END

#endif // __INVENTORY_BAR_H__
