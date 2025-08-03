#ifndef __BATTLE_MAP_H__
#define __BATTLE_MAP_H__

#include "common/Common.h"
#include "game/maps/MapData.h"
#include "game/entities/DataTypes.h"
#include "game/entities/DataLocatorObject.h"
#include "game/WorldListeners.h"

NS_BEGIN

enum BattleUpdateFlag
{
	BATTLE_UPDATEFLAG_NONE					= 0,
	BATTLE_UPDATEFLAG_STATE					= 1 << 0,
	BATTLE_UPDATEFLAG_ALIVE_COUNT			= 1 << 1,
	BATTLE_UPDATEFLAG_MAGICBEAN_COUNT		= 1 << 2,
	BATTLE_UPDATEFLAG_ALL					= BATTLE_UPDATEFLAG_STATE 
											| BATTLE_UPDATEFLAG_ALIVE_COUNT 
											| BATTLE_UPDATEFLAG_MAGICBEAN_COUNT,
};

enum BattleState
{
	BATTLE_STATE_NONE,
	BATTLE_STATE_PREPARING,
	BATTLE_STATE_IN_PROGRESS,
	BATTLE_STATE_ENDING,
	BATTLE_STATE_ENDED,
};

enum BattleOutcome
{
	BATTLE_VICTORY,
	BATTLE_DEFEAT,
};

enum TileFlag
{
	TILE_FLAG_NONE							= 0,
	TILE_FLAG_CLOSED						= 1 << 0,
	TILE_FLAG_ITEM_PLACED					= 1 << 1,
};

class World;
class WorldObject;
class Unit;
class Robot;
class Player;
class MyCharacter;
class LocatorObject;

class BattleMap
{
	typedef std::unordered_map<ObjectGuid, WorldObject*> ObjectMap;
	typedef std::unordered_map<ObjectGuid, LocatorObject*> LocatorObjectMap;

public:
	BattleMap(MapData* data, World* world, uint8 combatGrade);
	~BattleMap();

	int32 getMapId() const { return m_mapData->getMapId(); }
	MapData* getMapData() const { return m_mapData; }
	World* getWorld() const { return m_world; }

	uint8 getCombatGrade() const { return m_combatGrade; }
	int32 getMagicBeanCount() const { return m_magicBeanCount; }
	void setMagicBeanCount(int32 count) { m_magicBeanCount = count; }

	void pause();
	void resume();
	void update(float delta);

	void debugGrids();

	void addMyCharToMap(MyCharacter* myChar);
	template<typename T> void addToMap(T* obj);
	// 激活地图上的世界对象。如果被激活则返回true，否则返回false
	template<typename T> bool activateObject(T* obj);

	void removeMyChar();
	void removeFromMap(ObjectGuid const& guid, bool cleanup);

	MyCharacter* getMyChar() const { return m_myChar; }
	// 在地图中查找指定guid的对象
	// 如果includeInactiveObjects为true则查找范围将包含不活跃对象列表
	WorldObject* findObject(ObjectGuid const& guid, bool includeInactiveObjects = false) const;
	std::unordered_map<ObjectGuid, WorldObject*> const& getObjects() const { return m_objects; }
	std::unordered_map<ObjectGuid, WorldObject*> const& getInactiveObjects() const { return m_inactiveObjects; }
	void setObjectLifecycleListener(ObjectLifecycleListener* listener) { m_objectLifecycleListener = listener; }

	template<typename T> void addLocatorObject(T* object);
	void removeLocatorObject(ObjectGuid const& guid, bool cleanup);
	// 激活地图上的定位器对象。如果被激活则返回true，否则返回false
	template<typename T> bool activateLocatorObject(T* object);
	// 在地图中查找指定guid的定位器对象
	LocatorObject* findLocatorObject(ObjectGuid const& guid, bool includeInactiveObjects = false) const;
	void setLocatorObjectLifecycleListener(LocatorObjectLifecycleListener* listener) { m_locatorObjectLifecycleListener = listener; }

	// 单位在寻路时将避开状态为封闭的瓦片
	bool isTileClosed(int32 x, int32 y) const { return this->hasTileFlag(x, y, TILE_FLAG_CLOSED); }
	bool isTileClosed(TileCoord const& coord) const { return this->hasTileFlag(coord, TILE_FLAG_CLOSED); }
	void setTileClosedPosition(cocos2d::Point const& position, bool isClosed = true);
	void setTileClosed(TileCoord const& coord, bool isClosed = true);
	void setTileClosedToNewPosition(cocos2d::Point const& oldPosition, cocos2d::Point const& newPosition);

	void setTileFlags(TileCoord const& coord, uint16 flags);
	void addTileFlag(TileCoord const& coord, uint16 flag);
	bool hasTileFlag(TileCoord const& coord, uint16 flag) const { return this->hasTileFlag(coord.x, coord.y, flag); }
	bool hasTileFlag(int32 x, int32 y, uint16 flag) const;
	void clearTileFlag(TileCoord const& coord, uint16 flag);
	uint16 getTileFlags(TileCoord const& coord) const;

private:
	void clearClosedTileFlag(TileCoord const& coord);
	void markClosedTileFlag(TileCoord const& coord);

	void updateObjects(float delta);
	void removeAllObjects(bool needToNotify);
	void inactivateAllObjects();
	void clearInactiveObjects();
	void removeIfExistsInInactiveObjects(ObjectGuid const& guid);

	void notifyObjectDestroyed(WorldObject* obj);
	void notifyObjectInactivated(WorldObject* obj);
	void notifyObjectActivated(WorldObject* obj);

	void removeAllLocatorObjects(bool needToNotify);
	void inactivateAllLocatorObjects();
	void updateLocatorObjects(float delta);
	void clearInactiveLocatorObjects();
	void removeIfExistsInInactiveLocatorObjects(ObjectGuid const& guid);

	void notifyLocatorObjectDestroyed(LocatorObject* object);
	void notifyLocatorObjectInactivated(LocatorObject* object);
	void notifyLocatorObjectActivated(LocatorObject* object);

	MapData* m_mapData;
	World* m_world;
	MyCharacter* m_myChar;

	uint8 m_combatGrade;
	int32 m_magicBeanCount;

	ObjectMap m_objects;
	ObjectMap m_inactiveObjects;
	ObjectLifecycleListener* m_objectLifecycleListener;

	uint16* m_tileFlagsSet;

	LocatorObjectMap m_locatorObjects;
	LocatorObjectMap m_inactiveLocatorObjects;
	LocatorObjectLifecycleListener* m_locatorObjectLifecycleListener;
};

NS_END

#endif // __BATTLE_MAP_H__

