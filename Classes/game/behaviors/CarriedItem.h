#ifndef __CARRIED_ITEM_H__
#define __CARRIED_ITEM_H__

#include "game/entities/DataCarriedItem.h"
#include "game/WorldListeners.h"
#include "Object.h"

NS_BEGIN

class CarriedItem : public Object
{
public:
	CarriedItem();
    virtual ~CarriedItem();

	virtual void update(float delta) {}
	void removeFromWorld() override;

	PickupStatus canBeMergedPartlyWith(ItemTemplate const* itemTemplate) const;
    
    DataBasic* loadData(ObjectGuid const& guid);
    DataCarriedItem* getData() const override { return static_cast<DataCarriedItem*>(m_data); }

	void updateObject(UpdateType updateType, uint32 updateFlags, DataBasic* data) override;

private:
};


NS_END

#endif // __CARRIED_ITEM_H__
