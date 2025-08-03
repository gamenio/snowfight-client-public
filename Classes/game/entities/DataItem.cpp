#include "DataItem.h"

NS_BEGIN

DataItem::DataItem():
	m_itemId(0),
	m_position(Point::ZERO),
	m_count(0),
	m_holder(ObjectGuid::EMPTY),
	m_holderOrigin(Point::ZERO),
	m_launchCenter(Point::ZERO),
	m_launchRadiusInMap(0),
	m_dropDuration(0),
	m_dropElapsed(0),
	m_isAvailable(true)
{
	m_type |= DataTypeMask::DATA_TYPEMASK_ITEM;
	m_typeId = DataTypeID::DATA_TYPEID_ITEM;
    
    m_updateMask.setCount(CITEM_END);
}

DataItem::~DataItem()
{
}

void DataItem::clearFields()
{
	m_itemId = 0;
	m_position = Point::ZERO;
	m_count = 0;
	m_holder = ObjectGuid::EMPTY;
	m_holderOrigin = Point::ZERO;
	m_launchCenter = Point::ZERO;
	m_launchRadiusInMap = 0;
	m_dropDuration = 0;
	m_dropElapsed = 0;

	DataWorldObject::clearFields();
}

bool DataItem::readFields(UpdateType updateType, uint32 updateFlags, FieldUpdateMask const& updateMask, DataInputStream* input)
{
    CHECK_READ(DataWorldObject::readFields(updateType, updateFlags, updateMask, input));
    
	if (Updatable::isNeedReadField(updateMask, SITEM_FIELD_ITEMID))
	{
		CHECK_READ(Parcel::readUInt32(input, &m_itemId));
	}

	if (Updatable::isNeedReadField(updateMask, SITEM_FIELD_POSITION))
	{
		CHECK_READ(Parcel::readFloat(input, &m_position.x));
		CHECK_READ(Parcel::readFloat(input, &m_position.y));
	}

	if (Updatable::isNeedReadField(updateMask, SITEM_FIELD_COUNT))
	{
		CHECK_READ(Parcel::readInt32(input, &m_count));
	}

	if (Updatable::isNeedReadField(updateMask, SITEM_FIELD_HOLDER))
	{
		uint32 _guid;
		CHECK_READ((Parcel::readUInt32(input, &_guid)));
		m_holder = ObjectGuid(_guid);
	}

	if (Updatable::isNeedReadField(updateMask, SITEM_FIELD_HOLDER_ORIGIN))
	{
		CHECK_READ(Parcel::readFloat(input, &m_holderOrigin.x));
		CHECK_READ(Parcel::readFloat(input, &m_holderOrigin.y));
	}

	if (Updatable::isNeedReadField(updateMask, SITEM_FIELD_LAUNCH_CENTER))
	{
		CHECK_READ(Parcel::readFloat(input, &m_launchCenter.x));
		CHECK_READ(Parcel::readFloat(input, &m_launchCenter.y));
	}

	if (Updatable::isNeedReadField(updateMask, SITEM_FIELD_LAUNCH_RADIUS_IN_MAP))
		CHECK_READ(Parcel::readFloat(input, &m_launchRadiusInMap));

	if (Updatable::isNeedReadField(updateMask, SITEM_FIELD_DROP_DURATION))
		CHECK_READ((Parcel::readInt32(input, &m_dropDuration)));

	if (Updatable::isNeedReadField(updateMask, SITEM_FIELD_DROP_ELAPSED))
		CHECK_READ((Parcel::readInt32(input, &m_dropElapsed)));
    
    return true;
}

void DataItem::setAvailable(bool isAvailable)
{
	if (m_isAvailable != isAvailable)
	{
		m_isAvailable = isAvailable;
		this->setUpdatedField(CITEM_FIELD_IS_AVAILABLE);
	}
}

NS_END

