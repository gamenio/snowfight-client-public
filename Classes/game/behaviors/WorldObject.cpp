#include "WorldObject.h"

NS_BEGIN

WorldObject::WorldObject() :
	m_map(nullptr)
{
	m_type |= TypeMask::TYPEMASK_WORLDOBJECT;
	m_typeId = TypeID::TYPEID_WORLDOBJECT;
}


WorldObject::~WorldObject()
{
	m_map = nullptr;
}

void WorldObject::setMap(BattleMap* map)
{
	if (m_map != map)
	{
		DataWorldObject* data = this->getData();
		NS_ASSERT_LOG(data, "Need to set data for WorldObject before setMap()");
		if (map)
			data->setMapData(map->getMapData());
		else
			data->setMapData(nullptr);
		m_map = map;
	}
}

void WorldObject::removeFromWorld()
{
	if (!this->isInWorld())
		return;

	Object::removeFromWorld();
}

void WorldObject::cleanupBeforeDelete()
{
	if (this->isInWorld())
		this->removeFromWorld();
}

bool WorldObject::isWithinDist(Point const& pos, float dist) const
{
	bool ret = this->getData()->getPosition().getDistance(pos) <= dist;
	return  ret;
}

bool WorldObject::isWithinDist(WorldObject const* obj, float dist) const
{
	return isWithinDist(obj->getData()->getPosition(), dist);
}

NS_END
