#ifndef __WORLD_OBJECT_H__
#define __WORLD_OBJECT_H__

#include "common/Common.h"
#include "game/entities/DataWorldObject.h"
#include "game/maps/BattleMap.h"
#include "Object.h"

NS_BEGIN

class WorldObject: public Object
{
public:
	WorldObject();
	virtual ~WorldObject();

	void setMap(BattleMap* map);
	BattleMap* getMap() const { return m_map; }

	virtual void update(float delta) { }

	virtual void removeFromWorld() override;
	virtual void cleanupBeforeDelete();
		
	bool isWithinDist(cocos2d::Point const& pos, float dist) const;
	bool isWithinDist(WorldObject const* obj, float dist) const;;

	DataWorldObject* getData() override { return static_cast<DataWorldObject*>(m_data); }
	DataWorldObject const* getData() const override { return static_cast<DataWorldObject*>(m_data); }

protected:
	BattleMap* m_map;
};

NS_END

#endif // __WORLD_OBJECT_H__