#include "DataCarriedItem.h"


NS_BEGIN

DataCarriedItem::DataCarriedItem() :
	m_itemId(0),
	m_level(0),
	m_count(0),
	m_stackable(ITEM_STACK_NON_STACKABLE),
	m_slot(SLOT_INVALID),
	m_cooldownDuration(0)
{
	m_type |= DataTypeMask::DATA_TYPEMASK_CARRIED_ITEM;
	m_typeId = DataTypeID::DATA_TYPEID_CARRIED_ITEM;
    
    m_updateMask.setCount(SCARRIEDITEM_END);
}

DataCarriedItem::~DataCarriedItem()
{
}

void DataCarriedItem::clearFields()
{
	m_itemId = 0;
	m_level = 0;
	m_count = 0;
	m_stackable = ITEM_STACK_NON_STACKABLE;
	m_cooldownDuration = 0;
}

bool DataCarriedItem::readFields(UpdateType updateType, uint32 updateFlags, FieldUpdateMask const& updateMask, DataInputStream* input)
{
	if (Updatable::isNeedReadField(updateMask, SCARRIEDITEM_FIELD_ITEMID))
	{
		CHECK_READ(Parcel::readUInt32(input, &m_itemId));
	}

	if (Updatable::isNeedReadField(updateMask, SCARRIEDITEM_FIELD_LEVEL))
	{
		int32 level;
		CHECK_READ(Parcel::readInt32(input, &level));
		m_level = static_cast<uint8>(level);
	}

	if (Updatable::isNeedReadField(updateMask, SCARRIEDITEM_FIELD_COUNT))
	{
		CHECK_READ(Parcel::readInt32(input, &m_count));
		this->setUpdatedField(CCARRIEDITEM_FIELD_COUNT);
	}

	if (Updatable::isNeedReadField(updateMask, SCARRIEDITEM_FIELD_STACKABLE))
	{
		CHECK_READ(Parcel::readInt32(input, &m_stackable));
	}

	if (Updatable::isNeedReadField(updateMask, SCARRIEDITEM_FIELD_COOLDOWN_DURATION))
	{
		CHECK_READ(Parcel::readInt32(input, &m_cooldownDuration));
	}

	return true;
}

NS_END

