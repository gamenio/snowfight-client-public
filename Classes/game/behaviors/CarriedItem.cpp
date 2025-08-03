#include "CarriedItem.h"

#include "common/utils/TimeUtil.h"

NS_BEGIN

CarriedItem::CarriedItem()
{
    m_type |= TypeMask::TYPEMASK_CARRIED_ITEM;
    m_typeId = TypeID::TYPEID_CARRIED_ITEM;
}

CarriedItem::~CarriedItem()
{
}

void CarriedItem::removeFromWorld()
{
	if (!this->isInWorld())
		return;

	Object::removeFromWorld();
}

PickupStatus CarriedItem::canBeMergedPartlyWith(ItemTemplate const* itemTemplate) const
{
	if (this->getData()->getItemId() != itemTemplate->id)
		return PICKUP_STATUS_ITEM_CANT_STACK;

	if (itemTemplate->stackable != ITEM_STACK_UNLIMITED && this->getData()->getCount() >= itemTemplate->stackable)
		return PICKUP_STATUS_ITEM_STACK_LIMIT_EXCEEDED;

	return PICKUP_STATUS_OK;
}

DataBasic* CarriedItem::loadData(ObjectGuid const& guid)
{
    DataCarriedItem* data = new DataCarriedItem();
    data->autorelease();
    data->setGuid(guid);
    
    this->setData(data);
    
    return data;
}

void CarriedItem::updateObject(UpdateType updateType, uint32 updateFlags, DataBasic* data)
{
}

NS_END
