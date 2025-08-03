#include "DataItemBox.h"

NS_BEGIN

DataItemBox::DataItemBox():
	m_launchCenter(Point::ZERO),
	m_position(Point::ZERO),
	m_direction(LEFT_DOWN),
	m_health(0),
	m_maxHealth(0),
	m_isLocked(false)
{
	m_type |= DataTypeMask::DATA_TYPEMASK_ITEMBOX;
	m_typeId = DataTypeID::DATA_TYPEID_ITEMBOX;
    
    m_updateMask.setCount(CITEMBOX_END);
}

DataItemBox::~DataItemBox()
{
}

void DataItemBox::clearFields()
{
	m_position = Point::ZERO;
	m_direction = LEFT_DOWN;
	m_health = 0;
	m_maxHealth = 0;
	m_isLocked = false;

	DataWorldObject::clearFields();
}

bool DataItemBox::readFields(UpdateType updateType, uint32 updateFlags, FieldUpdateMask const& updateMask, DataInputStream* input)
{
    CHECK_READ(DataWorldObject::readFields(updateType, updateFlags, updateMask, input));
    
	if (Updatable::isNeedReadField(updateMask, SITEMBOX_FIELD_POSITION))
	{
		CHECK_READ(Parcel::readFloat(input, &m_position.x));
		CHECK_READ(Parcel::readFloat(input, &m_position.y));
	}

	if (Updatable::isNeedReadField(updateMask, SITEMBOX_FIELD_DIRECTION))
	{
		uint32 direction;
		CHECK_READ((Parcel::readUInt32(input, &direction)));
		m_direction = static_cast<uint8>(direction);
	}

	if (Updatable::isNeedReadField(updateMask, SITEMBOX_FIELD_HEALTH))
	{
		CHECK_READ((Parcel::readInt32(input, &m_health)));
		this->setUpdatedField(CITEMBOX_FIELD_HEALTH);

		this->setLocked(m_health > 0);
	}

	if (Updatable::isNeedReadField(updateMask, SITEMBOX_FIELD_MAX_HEALTH))
	{
		CHECK_READ((Parcel::readInt32(input, &m_maxHealth)));
		this->setUpdatedField(CITEMBOX_FIELD_MAX_HEALTH);
	}
    
    return true;
}

void DataItemBox::setLocked(bool isLocked)
{
	if (m_isLocked != isLocked)
	{
		m_isLocked = isLocked;
		this->setUpdatedField(CITEMBOX_FIELD_IS_LOCKED);
	}
}

NS_END

