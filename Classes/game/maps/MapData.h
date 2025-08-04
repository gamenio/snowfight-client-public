#ifndef __MAP_DATA_H__
#define __MAP_DATA_H__

#include "common/Common.h"
#include "common/debugging/DebugDrawer.h"
#include "game/tiles/TileCoord.h"
#include "game/maps/mapparser/TMXXMLParser.h"

NS_BEGIN

#define MAP_MARGIN_IN_TILES					11		// Map margin in number of tiles.

struct HidingSpotInfo
{
	HidingSpotInfo() :
		originCoord(TileCoord::INVALID),
		layerName("")
	{}

	TileCoord originCoord;
	std::string layerName;

	bool isValid() const { return originCoord != TileCoord::INVALID; }
};

class MapData: public cocos2d::Ref
{
	enum TileType : uint8
	{
		TILE_TYPE_NONE,
		TILE_TYPE_PENETRABLE,
		TILE_TYPE_COLLIDABLE,
		TILE_TYPE_CONCEALABLE,
	};

	enum GroundType : uint8
	{
		GROUND_TYPE_NONE,
		GROUND_TYPE_SNOW,
		GROUND_TYPE_WATER,
	};

	struct TileObject
	{
		int32 width;
		int32 height;
		int32 offsetX;
		int32 offsetY;
		std::vector<uint8> data;
	};

public:
	MapData();
	~MapData();

	bool loadData(uint16 mapId);
	void loadDataAsync(uint16 mapId, std::function<void(bool)> const& loadingStateCallback);

	bool isWall(int32 x, int32 y) const { return this->isCollidable(x, y) || this->isPenetrable(x, y); }
	bool isWall(TileCoord const& tileCoord) const { return this->isWall(tileCoord.x, tileCoord.y); }
	bool isSnow(TileCoord const& tileCoord) const { return m_groundData[this->getTileIndex(tileCoord)] == GROUND_TYPE_SNOW; }
	bool isWater(TileCoord const& tileCoord) const { return m_groundData[this->getTileIndex(tileCoord)] == GROUND_TYPE_WATER; }
	bool isPenetrable(int32 x, int32 y) const { return m_tileData[this->getTileIndex(x, y)] == TILE_TYPE_PENETRABLE; }
	bool isPenetrable(TileCoord const& tileCoord) const { return this->isPenetrable(tileCoord.x, tileCoord.y); }
	bool isCollidable(int32 x, int32 y) const { return m_tileData[this->getTileIndex(x, y)] == TILE_TYPE_COLLIDABLE; }
	bool isCollidable(TileCoord const& tileCoord) const { return this->isCollidable(tileCoord.x, tileCoord.y); }
	bool isConcealable(TileCoord const& tileCoord) const { return m_tileData[this->getTileIndex(tileCoord)] == TILE_TYPE_CONCEALABLE; }
	bool isValidTileCoord(int32 x, int32 y) const { return x >= 0 && y >= 0 && x < m_mapInfo->getMapSize().width && y < m_mapInfo->getMapSize().height; }
	bool isValidTileCoord(TileCoord const& tileCoord) const { return this->isValidTileCoord(tileCoord.x, tileCoord.y); }

	bool isSoundTriggered(std::string& triggeredName, cocos2d::Point const& position);
	bool getHidingSpotInfo(TileCoord const& tileCoord, HidingSpotInfo& result) const;

	uint16 getMapId() const { return m_mapId; }
    TMXMapInfo* getMapInfo() const { return m_mapInfo; }
	cocos2d::Size const& getMapSize() const { return m_mapInfo->getMapSize(); }
	cocos2d::Size const& getTileSize() const { return m_mapInfo->getTileSize(); }
	int32 getTileIndex(TileCoord const& tileCoord) const { return this->getTileIndex(tileCoord.x, tileCoord.y); }
	int32 getTileIndex(int32 x, int32 y) const { return x + y * (int32)m_mapInfo->getMapSize().width; }

	int32 getTileZForPos(cocos2d::Point const& pos, float* offsetZInTile = nullptr) const;
	int32 getMinTileZ() const;
	int32 getMaxTileZ() const;

	cocos2d::Point mapToOpenGLPos(cocos2d::Point const& mapPos) const;
	cocos2d::Point openGLToMapPos(cocos2d::Point const& glPos) const;

private:
	bool initWithData(unsigned char* data, ssize_t len, uint16 mapId);
	bool initMapInfo(unsigned char* data, ssize_t len, uint16 mapId);
	unsigned char* unzipData(std::string const& filename, ssize_t *size);
	std::unordered_map<int32, TileObject> parseTileObjects();
	void classifyTiles();

	Value const& getTileProperty(std::string const& propertyName, TileCoord const& tileCoord, TMXLayerInfo* layer) const;
	bool getTilePropertyAsBool(std::string const& propertyName, TileCoord const& tileCoord, TMXLayerInfo* layer) const;
	bool isUseAutomaticVertexZ(TMXLayerInfo* layer) const;

	cocos2d::Point screenToMapPos(cocos2d::Point const& screenPos) const;
	cocos2d::Point mapToScreenPos(cocos2d::Point const& mapPos) const;

	int32 getTileZ(TileCoord const& tileCoord) const;

	uint16 m_mapId;
	TMXMapInfo* m_mapInfo;
	TMXObjectGroup* m_soundTriggerGroup;

	TileType* m_tileData;
	GroundType* m_groundData;
	std::unordered_map<int32 /* TileIndex */, HidingSpotInfo> m_hidingSpotInfos;
};

NS_END

#endif //__MAP_DATA_H__
