#include "Object.h"

#include "game/entities/DataBasic.h"
#include "common/utils/TimeUtil.h"

NS_BEGIN

#define  INACTIVE_OBJECT_CLEANUP_DELAY			60000 //1 minutes

Object::Object() :
	m_type(TypeMask::TYPEMASK_OBJECT),
	m_typeId(TypeID::TYPEID_OBJECT),
	m_data(nullptr),
	m_isInWorld(false),
	m_isActivated(false),
	m_inactiveTime(0)
{
}

Object::~Object()
{
	CC_SAFE_RELEASE_NULL(m_data);
}

void Object::setData(DataBasic* data)
{
	CC_SAFE_RETAIN(data);
	CC_SAFE_RELEASE_NULL(m_data);
	m_data = data;
}

void Object::addToWorld()
{
	if (m_isInWorld)
		return;

	m_isInWorld = true;
}

void Object::removeFromWorld()
{
	if (!m_isInWorld)
		return;

	if (m_isActivated)
		this->inactivate();

	m_isInWorld = false;
}

bool Object::canRemoveFromWorld() const
{
	return time_util::getUptimeMillis() - m_inactiveTime >= INACTIVE_OBJECT_CLEANUP_DELAY;
}

void Object::activate()
{
	if (m_isActivated)
		return;

	m_isActivated = true;

	m_inactiveTime = 0;
	if (m_data)
		m_data->clearUpdateFlags();
	//CCLOG("ACTIVATED guid:0x%08X %s", this->getData()->getGuid(), this->getData()->getUpdateFlagsString().c_str());
}

void Object::inactivate()
{
	if (!m_isActivated)
		return;

	m_isActivated = false;

	m_inactiveTime = time_util::getUptimeMillis();
	if (m_data)
		m_data->clearUpdateFlags();
	//CCLOG("INACTIVATED guid:0x%08X %s", this->getData()->getGuid(), this->getData()->getUpdateFlagsString().c_str());
}

NS_END

