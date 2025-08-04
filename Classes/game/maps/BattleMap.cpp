#include "BattleMap.h"

#include "common/debugging/DebugDrawer.h"

#include "game/entities/DataUnitLocator.h"
#include "game/behaviors/Robot.h"
#include "game/behaviors/Player.h"
#include "game/behaviors/MyCharacter.h"
#include "game/behaviors/Item.h"
#include "game/behaviors/ItemBox.h"
#include "game/behaviors/Projectile.h"
#include "game/behaviors/UnitLocator.h"
#include "game/World.h"

NS_BEGIN


#if NS_DEBUG

#define DEBUG_MAP_GRID						0

// The number of rows (columns) of the grid in the map, Grid's total area should be able to be enough to cover the size of the map.
#define MAX_NUMBER_OF_GRIDS					23
// (MAX_NUMBER_OF_TILES-1)*32, 32 is the size of the Tile under positive projection.
#define GRID_SIZE							256

#endif // NS_DEBUG

BattleMap::BattleMap(MapData* data, World* world, uint8 combatGrade) :
	m_mapData(data),
	m_world(world),
	m_myChar(nullptr),
	m_combatGrade(combatGrade),
	m_magicBeanCount(0),
	m_objectLifecycleListener(nullptr),
	m_tileFlagsSet(nullptr),
	m_locatorObjectLifecycleListener(nullptr)
{
	CC_SAFE_RETAIN(m_mapData);
	int32 nTiles = (int32)(m_mapData->getMapSize().width * m_mapData->getMapSize().height);
	m_tileFlagsSet = new uint16[nTiles]();

	this->debugGrids();
}


BattleMap::~BattleMap()
{
	this->removeAllObjects(false);
	this->removeAllLocatorObjects(false);

	if (m_myChar)
	{
		m_myChar->cleanupBeforeDelete();
		CC_SAFE_DELETE(m_myChar);
	}

	m_world = nullptr;
	CC_SAFE_RELEASE_NULL(m_mapData);
	CC_SAFE_DELETE_ARRAY(m_tileFlagsSet);

	m_objectLifecycleListener = nullptr;
	m_locatorObjectLifecycleListener = nullptr;
}

void BattleMap::debugGrids()
{

#if DEBUG_MAP_GRID
	Brush* drawNode = sDebugDrawer->getDrawByTag("BattleMap.Grid");

	for (int x = 0; x < MAX_NUMBER_OF_GRIDS; x++)
	{
		for (int y = 0; y < MAX_NUMBER_OF_GRIDS; y++)
		{
			Rect r(x * GRID_SIZE, y * GRID_SIZE, GRID_SIZE, GRID_SIZE);
			drawNode->drawRect(r.origin, Vec2(r.getMaxX(), r.getMaxY()), Color4F::GRAY);

			Label* label = Label::createWithSystemFont(StringUtils::format("x=%d,y=%d", x, y), DEFAULT_SYSTEM_FONT, 18);
			label->setPosition(Vec2(r.getMidX(), r.getMidY()));
			label->setTextColor(Color4B::GRAY);
			drawNode->addChild(label);
		}
	}
#endif
}

void BattleMap::clearClosedTileFlag(TileCoord const& coord)
{
	if (this->hasTileFlag(coord, TILE_FLAG_CLOSED))
	{
		NS_ASSERT(m_mapData->isValidTileCoord(coord) && !m_mapData->isWall(coord));
		this->clearTileFlag(coord, TILE_FLAG_CLOSED);

		CCLOG("CLEAR CLOSED TILEFLAG[%d, %d]", coord.x, coord.y);
	}
}

void BattleMap::markClosedTileFlag(TileCoord const& coord)
{
	if (m_mapData->isValidTileCoord(coord) && !m_mapData->isWall(coord)
		&& !this->hasTileFlag(coord, TILE_FLAG_CLOSED))
	{
		this->addTileFlag(coord, TILE_FLAG_CLOSED);

		CCLOG("MARK CLOSED TILEFLAG[%d, %d]", coord.x, coord.y);
	}
}

void BattleMap::pause()
{
	if (m_myChar)
		m_myChar->stopMoving();
}

void BattleMap::resume()
{
}

void BattleMap::update(float delta)
{
	this->updateObjects(delta);
	this->clearInactiveObjects();

	this->updateLocatorObjects(delta);
	this->clearInactiveLocatorObjects();
}

void BattleMap::addMyCharToMap(MyCharacter* myChar)
{
	if (!myChar->isInWorld())
	{
		NS_ASSERT_LOG(m_myChar == nullptr, "There shouldn't be two my characters.");
		m_myChar = myChar;
		m_objects[myChar->getData()->getGuid()] = myChar;

		m_myChar->setMap(this);
		m_myChar->addToWorld();
	}
	else
	{
		this->inactivateAllObjects();
		this->inactivateAllLocatorObjects();
	}

	if (WorldSession* session = m_myChar->getSession())
		session->sendQueryCharacterInfo(m_myChar);
}

template<typename T>
bool BattleMap::activateObject(T* obj)
{
	if (!obj->isInWorld())
		return false;

	if (!obj->isActivated())
	{
		auto it = m_objects.find(obj->getData()->getGuid());
		if (it == m_objects.end())
		{
			this->removeIfExistsInInactiveObjects(obj->getData()->getGuid());
			m_objects[obj->getData()->getGuid()] = obj;
		}
		obj->activate();
		this->notifyObjectActivated(obj);
	}

	return true;
}

template<typename T>
void BattleMap::addToMap(T* obj)
{
	if (!obj->isInWorld())
	{
		m_objects[obj->getData()->getGuid()] = obj;

		obj->setMap(this);
		obj->addToWorld();
	}

	if (obj->asUnit())
	{
		if (WorldSession* session = this->getMyChar()->getSession())
			session->sendQueryCharacterInfo(obj->asUnit());
	}
}

void BattleMap::removeMyChar()
{
	this->removeAllObjects(true);
	this->removeAllLocatorObjects(true);

	if (m_myChar)
	{
		this->notifyObjectDestroyed(m_myChar);
		m_objects.erase(m_myChar->getData()->getGuid());
		m_myChar->cleanupBeforeDelete();
		CC_SAFE_DELETE(m_myChar);
	}
}

void BattleMap::removeFromMap(ObjectGuid const& guid, bool cleanup)
{
	WorldObject* obj = nullptr;

	auto it = m_objects.find(guid);
	if (it != m_objects.end())
		obj = (*it).second;

	NS_ASSERT_LOG(obj != nullptr, "Removed Object does not exist in the map");

	if (obj == m_myChar)
	{
		this->removeMyChar();
		return;
	}

	if (cleanup)
	{
		this->notifyObjectDestroyed(obj);
		obj->cleanupBeforeDelete();

		this->removeIfExistsInInactiveObjects(guid);
		m_objects.erase(it);
		CC_SAFE_DELETE(obj);
	}
	else
	{
		this->notifyObjectInactivated(obj);
		obj->inactivate();

		m_inactiveObjects[guid] = obj;
		m_objects.erase(it);
	}
}

void BattleMap::updateObjects(float delta)
{
	for (auto it = m_objects.begin(); it != m_objects.end(); ++it)
	{
		WorldObject* obj = (*it).second;
		obj->update(delta);
	}

}

void BattleMap::removeAllObjects(bool needToNotify)
{
	for (auto it = m_objects.begin(); it != m_objects.end(); )
	{
		WorldObject* obj = (*it).second;
		if(needToNotify)
			this->notifyObjectDestroyed(obj);

		it = m_objects.erase(it);
		if (obj != m_myChar)
		{
			obj->cleanupBeforeDelete();
			CC_SAFE_DELETE(obj);
		}
	}

	for (auto it = m_inactiveObjects.begin(); it != m_inactiveObjects.end();)
	{
		WorldObject* obj = (*it).second;
		if (needToNotify)
			this->notifyObjectDestroyed(obj);

		obj->cleanupBeforeDelete();
		it = m_inactiveObjects.erase(it);
		CC_SAFE_DELETE(obj);
	}
}

void BattleMap::inactivateAllObjects()
{
	for (auto it = m_objects.begin(); it != m_objects.end();)
	{
		WorldObject* obj = (*it).second;
		if (!obj->isActivated())
		{
			++it;
			continue;
		}

		this->notifyObjectInactivated(obj);
		obj->inactivate();

		m_inactiveObjects[obj->getData()->getGuid()] = obj;
		it = m_objects.erase(it);
	}
}

void BattleMap::clearInactiveObjects()
{
	for (auto it = m_inactiveObjects.begin(); it != m_inactiveObjects.end();)
	{
		WorldObject* obj = (*it).second;
		if (obj->canRemoveFromWorld())
		{
			this->notifyObjectDestroyed(obj);
			obj->cleanupBeforeDelete();

			it = m_inactiveObjects.erase(it);
			CC_SAFE_DELETE(obj);
		}
		else
			++it;
	}
}

void BattleMap::removeIfExistsInInactiveObjects(ObjectGuid const& guid)
{
	auto it = m_inactiveObjects.find(guid);
	if (it != m_inactiveObjects.end())
		m_inactiveObjects.erase(it);
}

void BattleMap::removeAllLocatorObjects(bool needToNotify)
{
	for (auto it = m_locatorObjects.begin(); it != m_locatorObjects.end(); )
	{
		LocatorObject* object = (*it).second;
		if(needToNotify)
			this->notifyLocatorObjectDestroyed(object);
		object->removeFromWorld();

		it = m_locatorObjects.erase(it);
		CC_SAFE_DELETE(object);
	}

	for (auto it = m_inactiveLocatorObjects.begin(); it != m_inactiveLocatorObjects.end();)
	{
		LocatorObject* object = (*it).second;
		if (needToNotify)
			this->notifyLocatorObjectDestroyed(object);
		object->removeFromWorld();

		it = m_inactiveLocatorObjects.erase(it);
		CC_SAFE_DELETE(object);
	}
}

void BattleMap::inactivateAllLocatorObjects()
{
	for (auto it = m_locatorObjects.begin(); it != m_locatorObjects.end(); )
	{
		LocatorObject* object = (*it).second;
		if (!object->isActivated())
		{
			++it;
			continue;
		}

		this->notifyLocatorObjectInactivated(object);
		object->inactivate();

		m_inactiveLocatorObjects[object->getData()->getGuid()] = object;
		it = m_locatorObjects.erase(it);
	}
}

void BattleMap::updateLocatorObjects(float delta)
{
	for (auto it = m_locatorObjects.begin(); it != m_locatorObjects.end(); ++it)
	{
		LocatorObject* object = (*it).second;
		object->update(delta);
	}
}

void BattleMap::clearInactiveLocatorObjects()
{
	for (auto it = m_inactiveLocatorObjects.begin(); it != m_inactiveLocatorObjects.end();)
	{
		LocatorObject* object = (*it).second;
		if (object->canRemoveFromWorld())
		{
			this->notifyLocatorObjectDestroyed(object);
			object->removeFromWorld();

			it = m_inactiveLocatorObjects.erase(it);
			CC_SAFE_DELETE(object);
		}
		else
			++it;
	}
}

void BattleMap::removeIfExistsInInactiveLocatorObjects(ObjectGuid const& guid)
{
	auto it = m_inactiveLocatorObjects.find(guid);
	if (it != m_inactiveLocatorObjects.end())
		m_inactiveLocatorObjects.erase(it);
}

void BattleMap::notifyLocatorObjectDestroyed(LocatorObject* object)
{
	if (m_locatorObjectLifecycleListener)
		m_locatorObjectLifecycleListener->onLocatorObjectDestroyed(object->getData()->getGuid());
}

void BattleMap::notifyLocatorObjectActivated(LocatorObject* object)
{
	if (m_locatorObjectLifecycleListener)
		m_locatorObjectLifecycleListener->onLocatorObjectActivated(object->getData());
}

void BattleMap::notifyLocatorObjectInactivated(LocatorObject* object)
{
	if (m_locatorObjectLifecycleListener)
		m_locatorObjectLifecycleListener->onLocatorObjectInactivated(object->getData()->getGuid());
}

WorldObject* BattleMap::findObject(ObjectGuid const& guid, bool includeInactiveObjects) const
{
	WorldObject* result = nullptr;

	auto it = m_objects.find(guid);
	if (it != m_objects.end())
	{
		WorldObject* obj = (*it).second;
		result = obj;
	}
	else if(includeInactiveObjects)
	{
		auto it = m_inactiveObjects.find(guid);
		if (it != m_inactiveObjects.end())
			result = (*it).second;
	}
	return result;
}

void BattleMap::notifyObjectDestroyed(WorldObject* obj)
{
	if (m_objectLifecycleListener)
		m_objectLifecycleListener->onObjectDestroyed(obj->getData()->getGuid());
}

void BattleMap::notifyObjectInactivated(WorldObject* obj)
{
	if (m_objectLifecycleListener)
		m_objectLifecycleListener->onObjectInactivated(obj->getData()->getGuid());
}

void BattleMap::notifyObjectActivated(WorldObject* obj)
{
	if (m_objectLifecycleListener)
		m_objectLifecycleListener->onObjectActivated(obj->getData());
}

template<typename T>
void BattleMap::addLocatorObject(T* object)
{
	if (!object->isInWorld())
	{
		m_locatorObjects[object->getData()->getGuid()] = object;

		object->setMap(this);
		object->addToWorld();
	}
}

void BattleMap::removeLocatorObject(ObjectGuid const& guid, bool cleanup)
{
	LocatorObject* object = nullptr;

	auto it = m_locatorObjects.find(guid);
	if (it != m_locatorObjects.end())
		object = (*it).second;

	NS_ASSERT_LOG(object != nullptr, "Removed LocatorObject does not exist in the map");

	if (cleanup)
	{
		this->notifyLocatorObjectDestroyed(object);
		object->removeFromWorld();

		this->removeIfExistsInInactiveLocatorObjects(guid);
		m_locatorObjects.erase(it);
		CC_SAFE_DELETE(object);
	}
	else
	{
		this->notifyLocatorObjectInactivated(object);
		object->inactivate();

		m_inactiveLocatorObjects[guid] = object;
		m_locatorObjects.erase(it);
	}
}

template<typename T>
bool BattleMap::activateLocatorObject(T* object)
{
	if (!object->isInWorld())
		return false;

	if (!object->isActivated())
	{
		auto it = m_locatorObjects.find(object->getData()->getGuid());
		if (it == m_locatorObjects.end())
		{
			this->removeIfExistsInInactiveLocatorObjects(object->getData()->getGuid());
			m_locatorObjects[object->getData()->getGuid()] = object;
		}
		object->activate();
		this->notifyLocatorObjectActivated(object);
	}

	return true;
}

LocatorObject* BattleMap::findLocatorObject(ObjectGuid const& guid, bool includeInactiveObjects) const
{
	LocatorObject* result = nullptr;

	auto it = m_locatorObjects.find(guid);
	if (it != m_locatorObjects.end())
	{
		LocatorObject* object = (*it).second;
		result = object;
	}
	else if (includeInactiveObjects)
	{
		auto it = m_inactiveLocatorObjects.find(guid);
		if (it != m_inactiveLocatorObjects.end())
			result = (*it).second;
	}
	return result;
}

void BattleMap::setTileClosedPosition(Point const& position, bool isClosed)
{
	TileCoord tileCoord(m_mapData->getMapSize(), position);
	this->setTileClosed(tileCoord, isClosed);
}

void BattleMap::setTileClosed(TileCoord const& coord, bool isClosed)
{
	if (isClosed)
		this->markClosedTileFlag(coord);
	else
		this->clearClosedTileFlag(coord);
}

void BattleMap::setTileClosedToNewPosition(Point const& oldPosition, Point const& newPosition)
{
	TileCoord oldTileCoord(m_mapData->getMapSize(), oldPosition);
	TileCoord newTileCoord(m_mapData->getMapSize(), newPosition);

	if (oldTileCoord != newTileCoord)
	{
		this->clearClosedTileFlag(oldTileCoord);
		this->markClosedTileFlag(newTileCoord);
	}
}

void BattleMap::setTileFlags(TileCoord const& coord, uint16 flags)
{
	m_tileFlagsSet[m_mapData->getTileIndex(coord)] = flags;
}

void BattleMap::addTileFlag(TileCoord const& coord, uint16 flag)
{
	if (!this->hasTileFlag(coord.x, coord.y, flag))
		m_tileFlagsSet[m_mapData->getTileIndex(coord)] |= flag;
}

bool BattleMap::hasTileFlag(int32 x, int32 y, uint16 flag) const
{
	return (m_tileFlagsSet[m_mapData->getTileIndex(x, y)] & flag) != 0;
}

void BattleMap::clearTileFlag(TileCoord const& coord, uint16 flag)
{
	if (hasTileFlag(coord, flag))
		m_tileFlagsSet[m_mapData->getTileIndex(coord)] &= ~flag;
}

uint16 BattleMap::getTileFlags(TileCoord const& coord) const
{
	return m_tileFlagsSet[m_mapData->getTileIndex(coord)];
}

template void BattleMap::addToMap(WorldObject*);
template bool BattleMap::activateObject(MyCharacter*);
template bool BattleMap::activateObject(Player*);
template bool BattleMap::activateObject(Robot*);
template bool BattleMap::activateObject(Projectile*);
template bool BattleMap::activateObject(Item*);
template bool BattleMap::activateObject(ItemBox*);

template void BattleMap::addLocatorObject(LocatorObject*);
template bool BattleMap::activateLocatorObject(UnitLocator*);

NS_END