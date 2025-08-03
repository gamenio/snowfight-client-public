#ifndef __ITEM_H__
#define __ITEM_H__

#include "game/entities/DataItem.h"
#include "WorldObject.h"

NS_BEGIN

class Item : public WorldObject
{
public:
    Item();
    virtual ~Item();

	void activate() override;
	void inactivate() override;

	void update(float delta) override;

	void updateAvailability();
    
    DataBasic* loadData(ObjectGuid const& guid);
    DataItem* getData() const override { return static_cast<DataItem*>(m_data); }

	void updateObject(UpdateType updateType, uint32 updateFlags, DataBasic* data) override;

private:
	void availabilityChange();

	bool m_isAvailabilityChanged;
};


NS_END


#endif // __ITEM_H__
