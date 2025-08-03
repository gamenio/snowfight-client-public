#ifndef __EQUIPMENT_BAR_H__
#define __EQUIPMENT_BAR_H__

#include "common/Common.h"
#include "game/WorldListeners.h"
#include "scene/gamble/CarriedProp.h"
#include "EquipmentSlot.h"

USING_NS_CC;

NS_BEGIN


class EquipmentBar : public Node, EquipmentItemLifecycleListener,
								  CarriedItemListener
{
public:
	static EquipmentBar* create();

	EquipmentBar();
	~EquipmentBar();

	bool init() override;

	void update(float delta) override;
	void cleanAfterUpdate();

	void show(cocos2d::Point const& moveToPosition);
	void hide();
	bool isShown() const { return m_isShown; }

	// EquipmentItemLifecycleListener
	void onEquipmentItemDestroyed(ObjectGuid const& guid) override;
	void onEquipmentItemActivated(DataCarriedItem* data) override;
	void onEquipmentItemInactivated(ObjectGuid const& guid) override;

private:
	void initSlots();

	CarriedProp* findCarriedProp(ObjectGuid const& guid) const;
	EquipmentSlot* getEquipmentSlot(int32 slot) const;
	void setEquipmentSlot(int32 slot, EquipmentSlot* equipmentSlot);

	bool m_isShown;

	Map<ObjectGuid, CarriedProp*> m_carriedProps;
	std::array<EquipmentSlot*, EQUIPMENT_SLOTS_COUNT> m_equipmentSlots;
};

NS_END

#endif // __EQUIPMENT_BAR_H__
