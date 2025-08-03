#include "LocatorObject.h"

#include "common/utils/TimeUtil.h"

NS_BEGIN

LocatorObject::LocatorObject() :
	m_map(nullptr)
{
	m_type |= TypeMask::TYPEMASK_LOCATOR_OBJECT;
	m_typeId = TypeID::TYPEID_LOCATOR_OBJECT;
}

LocatorObject::~LocatorObject()
{
	m_map = nullptr;
}

void LocatorObject::removeFromWorld()
{
	if (!this->isInWorld())
		return;

	Object::removeFromWorld();
}

NS_END