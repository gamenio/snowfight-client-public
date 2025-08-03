#include "MapData.h"

#include "common/utils/Intersection.h"
#include "game/utils/UnzipEncryptFile.h"


#define TMX_FILENAME						"map.tmx"
#define MAPDATA_FILE_PATH_FORMAT			MAP_ROOT "%d.data"

NS_BEGIN

MapData::MapData() :
	m_mapId(0),
	m_mapInfo(nullptr),
	m_soundTriggerGroup(nullptr),
	m_tileData(nullptr),
	m_groundData(nullptr)
{

}


MapData::~MapData()
{
	m_soundTriggerGroup = nullptr;
	CC_SAFE_RELEASE_NULL(m_mapInfo);

	CC_SAFE_DELETE_ARRAY(m_tileData);
	CC_SAFE_DELETE_ARRAY(m_groundData);
}


bool MapData::loadData(uint16 mapId)
{
	bool ret = false;

	std::string dataFile = StringUtils::format(MAPDATA_FILE_PATH_FORMAT, mapId);
	std::string fullPath = FileUtils::getInstance()->fullPathForFilename(dataFile);
	do
	{
		ssize_t len;
		unsigned char* data = this->unzipData(fullPath, &len);
		CC_BREAK_IF(!data);
		CC_BREAK_IF(!this->initWithData(data, len, mapId));

		ret = true;
	} while (0);

	return ret;
}

void MapData::loadDataAsync(uint16 mapId, std::function<void(bool)> const& loadingStateCallback)
{
	std::string dataFile = StringUtils::format(MAPDATA_FILE_PATH_FORMAT, mapId);
	std::string fullPath = FileUtils::getInstance()->fullPathForFilename(dataFile);
	AsyncTaskPool::getInstance()->enqueue(AsyncTaskPool::TaskType::TASK_OTHER, [this, fullPath, loadingStateCallback, mapId]() {
		ssize_t len;
		unsigned char* data = this->unzipData(fullPath, &len);

		Director::getInstance()->getScheduler()->performFunctionInCocosThread([this, mapId, data, len, loadingStateCallback]() mutable {
			if (!data)
			{
				if (loadingStateCallback)
					loadingStateCallback(false);
			}
			else
			{
				bool ret = this->initWithData(data, len, mapId);
				if (loadingStateCallback)
					loadingStateCallback(ret);
			}
		});
	});
}

bool MapData::initWithData(unsigned char* data, ssize_t len, uint16 mapId)
{
	bool ret = false;
	do
	{
		CC_BREAK_IF(!this->initMapInfo(data, len, mapId));
		this->classifyTiles();

		ret = true;
	} while (0);

	return ret;
}

bool MapData::initMapInfo(unsigned char* data, ssize_t len, uint16 mapId)
{
	std::string tmxString(reinterpret_cast<const char*>(data), len);
	CC_SAFE_FREE(data);

	std::string resPath = MAP_ROOT;
	auto pos = resPath.find_last_of("/");
	if (pos != std::string::npos)
		resPath.erase(pos);
	m_mapInfo = TMXMapInfo::createWithXML(tmxString, resPath);
	if (m_mapInfo)
	{
		m_mapId = mapId;
		CC_SAFE_RETAIN(m_mapInfo);

		m_soundTriggerGroup = m_mapInfo->getObjectGroup("SoundTriggerGroup");

		return true;
	}

	return false;
}
unsigned char* MapData::unzipData(std::string const& filename, ssize_t* size)
{
#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
	Data zipData = FileUtils::getInstance()->getDataFromFile(filename);
	UnzipEncryptFile* unzip = new UnzipEncryptFile(zipData.getBytes(), zipData.getSize());
#else
	UnzipEncryptFile* unzip = new UnzipEncryptFile(filename);
#endif
	unsigned char* data = unzip->getFileData(TMX_FILENAME, size, MAPDATA_KEY);
	CC_SAFE_DELETE(unzip);

	return data;
}

std::unordered_map<int32, MapData::TileObject> MapData::parseTileObjects()
{
	std::unordered_map<int32, TileObject> tileObjects;
	auto const& tilePropeties = m_mapInfo->getTileProperties();
	for (auto const& pr : tilePropeties)
	{
		int32 gid = pr.first;
		auto const& properties = pr.second;
		if (properties.isNull() || properties.getType() != Value::Type::MAP)
			continue;

		auto& valueMap = properties.asValueMap();
		//for (auto const& p : valueMap)
		//	CCLOG("gid: %d %s : %s", gid, p.first.c_str(), p.second.asString().c_str());
		auto it = valueMap.find("ObjectData");
		if (it != valueMap.end())
		{
			TileObject& object = tileObjects[gid];
			std::string data = (*it).second.asString();
			object.width = valueMap.at("ObjectWidth").asInt();
			object.height = valueMap.at("ObjectHeight").asInt();
			it = valueMap.find("ObjectOffsetX");
			object.offsetX = it != valueMap.end() ? (*it).second.asInt() : 0;
			it = valueMap.find("ObjectOffsetY");
			object.offsetY = it != valueMap.end() ? (*it).second.asInt() : 0;

			std::vector<std::string> elemTokens;
			std::istringstream dataStm(data);
			std::string sRow;
			while (getline(dataStm, sRow, '\n'))
			{
				std::string sElem;
				std::istringstream rowStm(sRow);
				while (getline(rowStm, sElem, ','))
				{
					elemTokens.push_back(sElem);
				}
			}

			CC_ASSERT(elemTokens.size() == object.width * object.height);
			object.data.resize(elemTokens.size());
			for (std::size_t i = 0; i < elemTokens.size(); ++i)
			{
				auto elem = (uint8)strtoul(elemTokens[i].c_str(), nullptr, 10);
				object.data[i] = elem;
			}
		}
	}

	return tileObjects;
}

void MapData::classifyTiles()
{
	CC_ASSERT(!m_groundData);
	CC_ASSERT(!m_tileData);

	int32 width = (int32)this->getMapSize().width;
	int32 height = (int32)this->getMapSize().height;
	int32 nTiles = width * height;

	m_groundData = new GroundType[nTiles];
	std::fill(m_groundData, m_groundData + nTiles, GROUND_TYPE_NONE);

	m_tileData = new TileType[nTiles];
	std::fill(m_tileData, m_tileData + nTiles, TILE_TYPE_NONE);

	TMXLayerInfo* groundLayer = nullptr;
	TMXLayerInfo* bridgeLayer = nullptr;
	std::unordered_map<int32, TileObject> tileObjects = this->parseTileObjects();
	for (auto layer : m_mapInfo->getLayers())
	{
		bool useAutomaticVertexZ = this->isUseAutomaticVertexZ(layer);
		for (int32 x = 0; x < width; ++x)
		{
			for (int32 y = 0; y < height; ++y)
			{
				int32 gid = layer->getTileGIDAt(Vec2((float)x, (float)y));
				if (!gid)
					continue;

				auto it = tileObjects.find(gid);
				if (it == tileObjects.end())
					continue;

				TileObject const& object = (*it).second;
				for (int32 i = 0; i < object.width; ++i)
				{
					for (int32 j = 0; j < object.height; ++j)
					{
						TileCoord coord;
						coord.x = x + i + object.offsetX;
						coord.y = y + j + object.offsetY;
						int32 tileIndex = this->getTileIndex(coord);
						uint8 tileType = object.data[i + j * object.width];
						if (m_tileData[tileIndex] == TILE_TYPE_NONE || !useAutomaticVertexZ)
							m_tileData[tileIndex] = static_cast<TileType>(tileType);

						if (m_tileData[tileIndex] == TILE_TYPE_CONCEALABLE)
						{
							HidingSpotInfo& info = m_hidingSpotInfos[tileIndex];
							info.originCoord.x = x;
							info.originCoord.y = y;
							info.layerName = layer->_name;
						}
					}
				}
			}
		}

		if (layer->_name == "Ground")
			groundLayer = layer;
		if (layer->_name == "Bridge")
			bridgeLayer = layer;
	}

	for (int32 x = 0; x < width; ++x)
	{
		for (int32 y = 0; y < height; ++y)
		{
			TileCoord coord(x, y);
			int32 tileIndex = this->getTileIndex(coord);

			if ((x < MAP_MARGIN_IN_TILES || x >= width - MAP_MARGIN_IN_TILES)
				|| (y < MAP_MARGIN_IN_TILES || y >= height - MAP_MARGIN_IN_TILES))
			{
				if (m_tileData[tileIndex] == TILE_TYPE_NONE || m_tileData[tileIndex] == TILE_TYPE_CONCEALABLE)
					m_tileData[tileIndex] = TILE_TYPE_PENETRABLE;
			}

			if (groundLayer)
			{
				if (this->getTilePropertyAsBool("Snow", coord, groundLayer))
					m_groundData[tileIndex] = GROUND_TYPE_SNOW;
				else if (this->getTilePropertyAsBool("Water", coord, groundLayer))
				{
					int32 gid = 0;
					if(bridgeLayer)
						gid = bridgeLayer->getTileGIDAt(Point((float)x, (float)y));
					if (!gid)
						m_groundData[tileIndex] = GROUND_TYPE_WATER;
				}

			}
		}
	}
}

Value const& MapData::getTileProperty(std::string const& propertyName, TileCoord const& tileCoord, TMXLayerInfo* layer) const
{
	if (!this->isValidTileCoord(tileCoord))
		return Value::Null;

	int gid = layer->getTileGIDAt(Vec2(tileCoord.x, tileCoord.y));
	auto& properties = m_mapInfo->getPropertiesForGID(gid);
	if (properties.isNull() || properties.getType() != Value::Type::MAP)
		return Value::Null;

	auto& valueMap = properties.asValueMap();
	auto it = valueMap.find(propertyName);
	if (it != valueMap.end())
		return (*it).second;

	return Value::Null;
}

bool MapData::getTilePropertyAsBool(std::string const& propertyName, TileCoord const& tileCoord, TMXLayerInfo* layer) const
{
	Value const& value = this->getTileProperty(propertyName, tileCoord, layer);
	return !value.isNull() && value.asBool();
}

bool MapData::isUseAutomaticVertexZ(TMXLayerInfo* layer) const
{
	auto const& properties = layer->getProperties();
	auto it = properties.find("cc_vertexz");
	if (it != properties.end())
	{
		auto const& val = (*it).second.asString();
		if (val == "automatic")
			return true;
	}

	return false;
}

Point MapData::screenToMapPos(Point const& screenPos) const
{
	const int tileWidth = m_mapInfo->getTileSize().width;
	const int tileHeight = m_mapInfo->getTileSize().height;

	const float x = screenPos.x - m_mapInfo->getMapSize().height * tileWidth / 2;
	const float tileY = screenPos.y / tileHeight;
	const float tileX = x / tileWidth;

	return Point((tileY + tileX) * tileHeight,
		(tileY - tileX) * tileHeight);
}

Point MapData::mapToScreenPos(Point const& mapPos) const
{
	const int tileWidth = m_mapInfo->getTileSize().width;
	const int tileHeight = m_mapInfo->getTileSize().height;
	const int originX = m_mapInfo->getMapSize().height * tileWidth / 2;
	const float tileY = mapPos.y / tileHeight;
	const float tileX = mapPos.x / tileHeight;

	return Point((tileX - tileY) * tileWidth / 2 + originX,
		(tileX + tileY) * tileHeight / 2);
}

int32 MapData::getTileZ(TileCoord const& tileCoord) const
{
	int32 tileZ = -int32((m_mapInfo->getMapSize().width + m_mapInfo->getMapSize().height) - (tileCoord.x + tileCoord.y));
	return tileZ;
}

bool MapData::getHidingSpotInfo(TileCoord const& tileCoord, HidingSpotInfo& result) const
{
	int32 tileIndex = this->getTileIndex(tileCoord);
	auto it = m_hidingSpotInfos.find(tileIndex);
	if (it != m_hidingSpotInfos.end())
	{
		result = (*it).second;
		return true;
	}

	return false;
}

Point MapData::mapToOpenGLPos(Point const& mapPos) const
{
	Point glPos = this->mapToScreenPos(mapPos);
	glPos.y = m_mapInfo->getMapSize().height * m_mapInfo->getTileSize().height - 1 - glPos.y;

	return glPos;
}

Point MapData::openGLToMapPos(Point const& glPos) const
{
	float glY = m_mapInfo->getMapSize().height * m_mapInfo->getTileSize().height - 1 - glPos.y;
	return this->screenToMapPos(Vec2(glPos.x, glY));
}

bool MapData::isSoundTriggered(std::string& triggeredName, Point const& position)
{
	triggeredName = "";
	if (m_soundTriggerGroup)
	{
		ValueVector& objects = m_soundTriggerGroup->getObjects();
		for (auto it = objects.begin(); it != objects.end(); ++it)
		{
			Value& val = *it;
			ValueMap& properties = val.asValueMap();
			std::string name = properties.at("name").asString();
			float x = properties.at("x").asFloat();
			float y = properties.at("y").asFloat();
			float width = properties.at("width").asFloat();
			float height = properties.at("height").asFloat();
			Rect r(x, y, width, height);
			Point p = openGLToMapPos(position);
			if (r.containsPoint(p))
			{
				triggeredName = name;
				return true;
			}
		}
	}

	return false;
}

int32 MapData::getTileZForPos(Point const& pos, float* offsetZInTile) const
{
	//cc_vertexz(TileZ)的给值规律:
	//[x,y,z]: [0,0,-60] [0,29,-31] [29,0,-31]  [29,29,-2];

	Size mapSize = this->getMapSize();

	float ix = pos.x - mapSize.width * TILE_WIDTH_HALF;
	float iy = mapSize.height * TILE_HEIGHT - pos.y;
	float tx = (ix / TILE_WIDTH_HALF + iy / TILE_HEIGHT_HALF) / 2;
	float ty = (iy / TILE_HEIGHT_HALF - ix / TILE_WIDTH_HALF) / 2;
	float tileX = std::floor(tx);
	float tileY = std::floor(ty);
	int32 tileZ = -int32((mapSize.width + mapSize.height) - (tileX + tileY));
	if (offsetZInTile)
	{
		float offX = (tx - tileX) * TILE_HEIGHT;
		float offY = (ty - tileY) * TILE_HEIGHT;
		*offsetZInTile = (offX + offY) / (TILE_HEIGHT * 2);
		//CC_LOG("tile: %f,%f,%d offset: %f,%f,%f z: %f floor(z): %f", tx, ty, tileZ, offX, offY, *offsetZInTile, tileZ + *offsetZInTile, std::floor(tileZ + *offsetZInTile));
	}

	return tileZ;
}


int32 MapData::getMinTileZ() const
{
	return this->getTileZ(TileCoord::ZERO);
}


int32 MapData::getMaxTileZ() const
{
	return this->getTileZ(TileCoord((int32)this->getMapSize().width - 1, (int32)this->getMapSize().height - 1));
}


NS_END
