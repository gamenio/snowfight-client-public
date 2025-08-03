#include "DataLocatorObject.h"

NS_BEGIN

DataLocatorObject::DataLocatorObject()
{
	m_type |= DataTypeMask::DATA_TYPEMASK_LOCATOR_OBJECT;
	m_typeId = DataTypeID::DATA_TYPEID_LOCATOR_OBJECT;
}

DataLocatorObject::~DataLocatorObject()
{
}

NS_END