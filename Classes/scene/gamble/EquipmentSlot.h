#ifndef __EQUIPMENT_SLOT_H__
#define __EQUIPMENT_SLOT_H__

#include "common/Common.h"
#include "game/WorldListeners.h"
#include "scene/gamble/CarriedProp.h"
#include "scene/gui/silhouette/SILSprite.h"

USING_NS_CC;

NS_BEGIN

class EquipmentSlot : public Node
{
public:
	enum SlotType
	{
		SNOWBALL_MAKER,
		GLOVES,
		HAT,
		JACKET,
		SHOES,
		MAX_SLOT_TYPES,
	};

	static EquipmentSlot* create(SlotType type);

	EquipmentSlot();
	virtual ~EquipmentSlot();

	bool init(SlotType type);

	void update(float delta) override;

	void setProp(CarriedProp* prop);
	void removeProp();
	CarriedProp* getProp() const { return m_prop; }

	SlotType getType() const { return m_type; }

	void onPropActivated();
	void onPropInactivated();

private:
	void clearAfterPropInactivated();

	SlotType m_type;

	SILSprite* m_bgSp;
	SILSprite* m_fgSp;
	CarriedProp* m_prop;
};

NS_END

#endif // __EQUIPMENT_SLOT_H__
