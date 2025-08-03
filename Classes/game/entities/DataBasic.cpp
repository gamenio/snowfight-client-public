#include "DataBasic.h"

NS_BEGIN

DataBasic::DataBasic():
	m_type(DataTypeMask::DATA_TYPEMASK_BASIC),
	m_typeId(DataTypeID::DATA_TYPEID_BASIC),
	m_guid(ObjectGuid::EMPTY)
{
}

DataBasic::~DataBasic()
{
}

NS_END
