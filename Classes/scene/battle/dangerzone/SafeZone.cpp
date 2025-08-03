#include "SafeZone.h"

#include "common/utils/TimeUtil.h"

NS_BEGIN

SafeZone::SafeZone(MapData* mapData, TileCoord const& center, int32 radius, NSTime duration) :
	m_mapData(mapData),
	m_center(center),
	m_radius(radius),
	m_duration(duration)
{
	CC_SAFE_RETAIN(m_mapData);
}

SafeZone::~SafeZone()
{
	this->clearCloudDataSet();

	CC_SAFE_RELEASE_NULL(m_mapData);
}

void SafeZone::createClouds()
{
	NSTime currTime = time_util::getUptimeMillis();

	int32 nTiles = (int32)(m_mapData->getMapSize().width * m_mapData->getMapSize().height);
	m_cloudDataSet.clear();
	m_cloudDataSet.resize(nTiles);
	this->createCloudData(CloudGenerator::POSITION_TOP, m_center, m_radius, m_duration);
	this->createCloudData(CloudGenerator::POSITION_LEFT, m_center, m_radius, m_duration);
	this->createCloudData(CloudGenerator::POSITION_BOTTOM, m_center, m_radius, m_duration);
	this->createCloudData(CloudGenerator::POSITION_RIGHT, m_center, m_radius, m_duration);

	int32 elapsed = time_util::getUptimeMillis() - currTime;
    NS_UNUSED_VARIABLE(elapsed);
	CCLOG("Create clouds for safe zone in %d ms", elapsed);
}

void SafeZone::createCloudData(CloudGenerator::Position pos, TileCoord const& safeZoneCenter, int32 initialSafeZoneRadius, NSTime duration)
{
	CloudGenerator generator;
	generator.init(pos, m_mapData, safeZoneCenter, initialSafeZoneRadius, duration);
	std::vector<CloudDataRow> const& clouds = generator.spawnClouds();
	for (auto it = clouds.begin(); it != clouds.end(); ++it)
	{
		auto const& row = *it;
		for (auto rowIt = row.begin(); rowIt != row.end(); ++rowIt)
		{
			CloudData* data = *rowIt;
			if (data->tileCoord.x >= 0 && data->tileCoord.x < m_mapData->getMapSize().width && data->tileCoord.y >= 0 && data->tileCoord.y < m_mapData->getMapSize().height)
			{
				int32 tileIndex = data->tileCoord.x * (int32)m_mapData->getMapSize().height + data->tileCoord.y;
				data->retain();
				m_cloudDataSet[tileIndex] = data;
			}
		}
	}
}

void SafeZone::clearCloudDataSet()
{
	for (auto it = m_cloudDataSet.begin(); it != m_cloudDataSet.end();)
	{
		CloudData* data = *it;
		if (data)
			data->release();
		it = m_cloudDataSet.erase(it);
	}
}

NS_END
