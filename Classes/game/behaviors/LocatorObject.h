#ifndef __LOCATOR_OBJECT_H__
#define __LOCATOR_OBJECT_H__

#include "common/Common.h"
#include "Object.h"
#include "game/entities/DataLocatorObject.h"
#include "game/maps/BattleMap.h"

NS_BEGIN

class LocatorObject: public Object
{
public:
	LocatorObject();
	virtual ~LocatorObject();

	void setMap(BattleMap* map) { m_map = map; }
	BattleMap* getMap() const { return m_map; }

	virtual void update(float delta) { }

	virtual void removeFromWorld() override;

	DataLocatorObject* getData() override { return static_cast<DataLocatorObject*>(m_data); }
	DataLocatorObject const* getData() const override { return static_cast<DataLocatorObject*>(m_data); }

private:
	BattleMap* m_map;
};

NS_END

#endif // __LOCATOR_OBJECT_H__