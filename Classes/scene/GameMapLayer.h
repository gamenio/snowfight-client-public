#ifndef __GAME_MAP_LAYER_H__
#define __GAME_MAP_LAYER_H__

#include "common/Common.h"
#include "tilemap/FastTMXTiledMap.h"
#include "gui/silhouette/SilhouetteLayer.h"
#include "game/WorldListeners.h"

USING_NS_CC;

NS_BEGIN

class GameObject;

class GameMapLayer : public Node, ObjectLifecycleListener, 
								  ProjectileListener,
								  MyCharacterListener,
								  ItemApplicationListener
{
public:
	static GameMapLayer* create();

	GameMapLayer();
	~GameMapLayer();

	bool init() override;

	void update(float delta) override;
	void cleanAfterUpdate();

	void addChild(Node* child, bool silhouetted);
	void removeChild(Node* child);

	TMXTiledMap* getMap() const { return m_map; }

	void setTileOpacity(std::string const& layerName, TileCoord const& coord, GLubyte opacity);
	void setTileColor(std::string const& layerName, TileCoord const& coord, Color4B const& color);

	// ObjectLifecycleListener
	void onObjectDestroyed(ObjectGuid const& guid) override;
	void onObjectActivated(DataBasic* data) override;
	void onObjectInactivated(ObjectGuid const& guid) override;

	// ProjectileListener
	void onLaunchResult(LaunchResult const& result) override;

	// MyCharacterListener
	void onItemPickupResult(ItemPickupResult const& result) override;

	// ItemApplicationListener
	void onItemApplicationUpdate(ItemApplicationUpdate const& update) override;
	void onItemApplicationUpdateAll(ItemApplicationUpdateAll const& updateAll) override;

	GameObject* getGameObject(ObjectGuid const& guid) const { return m_gameObjects.at(guid); }
	template<class T> T* getGameObject(ObjectGuid const& guid) const { return static_cast<T*>(m_gameObjects.at(guid)); }
	Map<ObjectGuid, GameObject*> const& getAllGameObjects() const { return m_gameObjects; }

private:
	using Node::addChild;
	using Node::getChildByTag;
	using Node::getChildByName;
	using Node::enumerateChildren;
	using Node::getChildren;
	using Node::getChildrenCount;
	using Node::removeChildByName;
	using Node::reorderChild;
	using Node::sortAllChildren;
	using Node::removeAllChildren;
	using Node::removeAllChildrenWithCleanup;

	void attachToMap(TMXTiledMap* map);

	void addGameObject(DataBasic* data);
	void removeGameObject(ObjectGuid const& guid);

	Map<ObjectGuid, GameObject*> m_gameObjects;

	TMXTiledMap* m_map;
	SilhouetteLayer* m_silhouetteLayer;
	Layer* m_layer;
};

NS_END

#endif // __GAME_MAP_LAYER_H__
