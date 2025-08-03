#include "GameMapLayer.h"

#include "tilemap/FastTMXLayer.h"
#include "game/maps/BattleMap.h"
#include "game/World.h"
#include "gamble/MyHero.h"
#include "gamble/AntiHero.h"
#include "gamble/Chest.h"
#include "gamble/Prop.h"
#include "gamble/Snowball.h"
#include "gamble/Footprint.h"

NS_BEGIN

GameMapLayer* GameMapLayer::create()
{
	GameMapLayer *ret = new (std::nothrow) GameMapLayer();
	if (ret && ret->init())
	{
		ret->autorelease();
	}
	else
	{
		CC_SAFE_DELETE(ret);
	}

	return ret;
}

GameMapLayer::GameMapLayer() :
	m_map(nullptr),
	m_silhouetteLayer(nullptr),
	m_layer(nullptr)
{
}

GameMapLayer::~GameMapLayer()
{
	FootprintPool::destroyInstance();

	m_map = nullptr;
	m_silhouetteLayer = nullptr;
	m_layer = nullptr;
}

bool GameMapLayer::init()
{
	if (!Node::init())
		return false;

	World* world = World::getInstance();
	DataPlayer* myChar = world->getMyself();
	NS_ASSERT(myChar);

	world->setObjectLifecycleListener(this);
	world->addProjectileListener(this);
	world->addMyCharacterListener(this);
	world->addItemApplicationListener(this);

	FootprintPool::getInstance()->init();

	m_map = TMXTiledMap::createWithMapInfo(myChar->getMapData()->getMapInfo());
	this->addChild(m_map);

	m_silhouetteLayer = SilhouetteLayer::create();
	this->addChild(m_silhouetteLayer);

	m_layer = Layer::create();
	this->addChild(m_layer);

	this->attachToMap(m_map);

	return true;
}

void GameMapLayer::update(float delta)
{
	for (auto p : m_gameObjects)
	{
		GameObject* obj = p.second;
		obj->update(delta);
	}
}

void GameMapLayer::cleanAfterUpdate()
{
	for (auto p : m_gameObjects)
	{
		GameObject* obj = p.second;
		if (obj->isActive())
			obj->cleanUpdateMask();
	}
}

void GameMapLayer::addChild(Node* child, bool silhouetted)
{
	if (silhouetted)
		m_silhouetteLayer->addChild(child);
	else
		m_layer->addChild(child);
}

void GameMapLayer::removeChild(Node* child)
{
	child->removeFromParentAndCleanup(true);
}

void GameMapLayer::setTileOpacity(std::string const& layerName, TileCoord const& coord, GLubyte opacity)
{
	if (layerName.empty())
		return;

	TMXLayer* layer = m_map->getLayer(layerName);
	if (layer)
		layer->setTileOpacity(Vec2(coord.x, coord.y), opacity);
}

void GameMapLayer::setTileColor(std::string const& layerName, TileCoord const& coord, Color4B const& color)
{
	if (layerName.empty())
		return;

	TMXLayer* layer = m_map->getLayer(layerName);
	if (layer)
		layer->setTileColor(Vec2(coord.x, coord.y), color);
}

void GameMapLayer::onObjectDestroyed(ObjectGuid const& guid)
{
	this->removeGameObject(guid);
}

void GameMapLayer::onObjectActivated(DataBasic* data)
{
	NS_ASSERT(data->getGuid() != ObjectGuid::EMPTY);

	auto it = m_gameObjects.find(data->getGuid());
	if (it != m_gameObjects.end())
		(*it).second->onActivated();
	else
		this->addGameObject(data);
}

void GameMapLayer::onObjectInactivated(ObjectGuid const& guid)
{
	auto it = m_gameObjects.find(guid);
	if (it != m_gameObjects.end())
	{
		(*it).second->onInactivated();
	}
}

void GameMapLayer::onLaunchResult(LaunchResult const& result)
{
	ObjectGuid projGuid(result.projectile());
	ObjectGuid targetGuid(result.target());
	Point position(result.position_x(), result.position_y());
	LaunchStatus status = static_cast<LaunchStatus>(result.status());

	Bullet* bullet = this->getGameObject<Bullet>(projGuid);
	if (bullet)
		bullet->onLaunchResult(status, targetGuid, position);
}

void GameMapLayer::onItemPickupResult(ItemPickupResult const& result)
{
	PickupStatus status = static_cast<PickupStatus>(result.status());
	ObjectGuid itemGuid(result.item());

	Prop* prop = this->getGameObject<Prop>(itemGuid);
	if (prop)
		prop->onItemPickupResult(status, result.remaining_time());
}

void GameMapLayer::onItemApplicationUpdate(ItemApplicationUpdate const& update)
{
	ObjectGuid targetGuid(update.target());

	Hero* hero = this->getGameObject<Hero>(targetGuid);
	if (hero)
	{
		auto const& info = update.app();
		hero->onItemApplicationUpdate(info.apply(), info.item_id(), info.duration(), info.remaining_time());
	}
}

void GameMapLayer::onItemApplicationUpdateAll(ItemApplicationUpdateAll const& updateAll)
{
	ObjectGuid targetGuid(updateAll.target());

	Hero* hero = this->getGameObject<Hero>(targetGuid);
	if (hero)
	{
		auto const& appList = updateAll.app_list();
		for (auto it = appList.begin(); it != appList.end(); ++it)
		{
			auto const& info = *it;
			hero->onItemApplicationUpdate(info.apply(), info.item_id(), info.duration(), info.remaining_time());
		}
	}
}

void GameMapLayer::attachToMap(TMXTiledMap* map)
{
	Vector<Node*> covers;
	auto const& layers = map->getChildren();
	for (auto it = layers.begin(); it != layers.end(); ++it)
	{
		TMXLayer* layer = dynamic_cast<TMXLayer*>(*it);
		if (!layer || !layer->isVisible())
			continue;

		if (layer->isUseAutomaticVertexZ())
			covers.pushBack(layer);
	}
	m_silhouetteLayer->setCovers(covers);
	m_silhouetteLayer->setGlobalZOrderRange(INT32_MIN, -1);
}

void GameMapLayer::addGameObject(DataBasic* data)
{
	NS_ASSERT(data->getGuid() != ObjectGuid::EMPTY);
	NS_ASSERT(m_gameObjects.find(data->getGuid()) == m_gameObjects.end());

	GameObject* object = nullptr;
	switch (data->getTypeID())
	{
	case DATA_TYPEID_ROBOT:
		object = AntiHero::createWithData(data->asDataUnit());
		break;
	case DATA_TYPEID_PLAYER:
	{
		DataPlayer* player = data->asDataPlayer();
		if (player->isSelf())
			object = MyHero::createWithData(player);
		else
			object = AntiHero::createWithData(player);
		break;
	}
	case DATA_TYPEID_ITEMBOX:
	{
		object = Chest::createWithData(data->asDataItemBox());
		break;
	}
	case DATA_TYPEID_ITEM:
		object = Prop::createWithData(data->asDataItem());
		break;
	case DATA_TYPEID_PROJECTILE:
		object = Snowball::createWithData(data->asDataProjectile());
		break;
	default:
		NS_ASSERT_LOG(false, "Unsupported data type.");
		break;
	}

	if (object)
	{
		m_gameObjects.insert(object->getData()->getGuid(), object);
		object->setGameMapLayer(this);
		object->onActivated();
		m_silhouetteLayer->addChild(object);
	}
}

void GameMapLayer::removeGameObject(ObjectGuid const& guid)
{
	auto it = m_gameObjects.find(guid);
	if (it != m_gameObjects.end())
	{
		GameObject* obj = (*it).second;
		m_silhouetteLayer->removeChild(obj, true);
		obj->setGameMapLayer(nullptr);
		m_gameObjects.erase(it);
	}
}

NS_END

