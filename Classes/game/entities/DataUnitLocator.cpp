#include "DataUnitLocator.h"

NS_BEGIN

DataUnitLocator::DataUnitLocator():
	m_displayId(0),
	m_isAlive(true),
	m_moveSpeed(0)
{
	m_type |= DataTypeMask::DATA_TYPEMASK_UNIT_LOCATOR;
	m_typeId = DataTypeID::DATA_TYPEID_UNIT_LOCATOR;

	m_updateMask.setCount(CUNIT_LOCATOR_END);
}

DataUnitLocator::~DataUnitLocator()
{
}

void DataUnitLocator::clearFields()
{
	m_locationInfo.Clear();
	m_displayId = 0;
	m_isAlive = true;
	m_moveSpeed = 0;

	DataLocatorObject::clearFields();
}

bool DataUnitLocator::readFields(UpdateType updateType, uint32 updateFlags, FieldUpdateMask const& updateMask, DataInputStream* input)
{
	CHECK_READ(DataLocatorObject::readFields(updateType, updateFlags, updateMask, input));

	if (Updatable::isNeedReadField(updateMask, SUNIT_LOCATOR_FIELD_LOCATION))
	{
		m_locationInfo.Clear();
		CHECK_READ(m_locationInfo.readFromStream(input));
	}

	if (Updatable::isNeedReadField(updateMask, SUNIT_LOCATOR_FIELD_DISPLAYID)) 
	{
		CHECK_READ(Parcel::readUInt32(input, &m_displayId));
	}

	if (Updatable::isNeedReadField(updateMask, SUNIT_LOCATOR_FIELD_IS_ALIVE))
	{
		CHECK_READ(Parcel::readBool(input, &m_isAlive));
		this->setUpdatedField(CUNIT_LOCATOR_FIELD_IS_ALIVE);
	}

	if (Updatable::isNeedReadField(updateMask, SUNIT_LOCATOR_FIELD_MOVE_SPEED))
	{
		CHECK_READ(Parcel::readInt32(input, &m_moveSpeed));
	}

	return true;
}

NS_END