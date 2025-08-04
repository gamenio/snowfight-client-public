#include "CloudGenerator.h"

#include "common/utils/TimeUtil.h"
#include "CloudCluster.h"

NS_BEGIN

#if NS_DEBUG
#define ENABLE_SPAWN_CLOUDS_LOG			0
#endif

void CloudGenerator::init(Position position, MapData* mapData, TileCoord const& safeZoneCenter, int32 initialSafeZoneRadius, NSTime duration)
{
	m_position = position;
	m_mapData = mapData;
	CC_SAFE_RETAIN(mapData);

	m_safeZoneCenter = safeZoneCenter;
	m_initialSafeZoneRadius = initialSafeZoneRadius;

	int32 mapWidth = (int32)m_mapData->getMapSize().width;
	int32 mapHeight = (int32)m_mapData->getMapSize().height;

	int32 paddingTop = m_safeZoneCenter.y + 1;
	int32 paddingRight = mapWidth - (m_safeZoneCenter.x + 1);
	int32 paddingBottom = mapHeight - (m_safeZoneCenter.y + 1);
	int32 paddingLeft = m_safeZoneCenter.x + 1;
	switch (m_position)
	{
	case POSITION_TOP:
		m_firstRowIndex = m_initialSafeZoneRadius - paddingTop;
		break;
	case POSITION_RIGHT:
		m_firstRowIndex = m_initialSafeZoneRadius - paddingRight;
		break;
	case POSITION_BOTTOM:
		m_firstRowIndex = m_initialSafeZoneRadius - paddingBottom;
		break;
	case POSITION_LEFT:
		m_firstRowIndex = m_initialSafeZoneRadius - paddingLeft;
		break;
	}
	m_startSafeZoneRadius = std::max(paddingLeft, std::max(paddingBottom, std::max(paddingRight, paddingTop)));
	if (m_initialSafeZoneRadius < m_startSafeZoneRadius)
		CCLOG("CloudGenerator: WARNING: The safe zone radius does not cover the whole map.");

	float durInSec = duration / 1000.f;
	m_interval = durInSec / m_initialSafeZoneRadius;
	m_startTime = durInSec - m_startSafeZoneRadius * m_interval + m_interval;
	CCLOG("position: %d center: %d,%d radius: %d duration: %f interval: %f", position, safeZoneCenter.x, safeZoneCenter.y, initialSafeZoneRadius, durInSec, m_interval);
}

CloudGenerator::CloudGenerator() :
	m_position(POSITION_TOP),
	m_startSafeZoneRadius(0),
	m_initialSafeZoneRadius(0),
	m_startTime(0),
	m_firstRowIndex(0),
	m_interval(0)
{
}

CloudGenerator::~CloudGenerator()
{
	CC_SAFE_RELEASE_NULL(m_mapData);
}

void CloudGenerator::createParentClouds(float time)
{
	int32 mapWidth = (int32)m_mapData->getMapSize().width;
	int32 mapHeight = (int32)m_mapData->getMapSize().height;

	CloudDataRow dataRow;

	if (m_position == POSITION_LEFT)
	{
//		int32 lastRowX = m_safeZoneCenter.x;
		int32 lastRowY = m_safeZoneCenter.y;
		int32 rowClouds = (m_initialSafeZoneRadius * 2 - 1) - m_firstRowIndex * 2;
		int32 rowCloudsHalf = (int32)std::ceil((float)rowClouds / 2);
		int32 rowMarginTop = lastRowY + 1 - rowCloudsHalf;
		int32 rowMarginBottom =  mapHeight - (lastRowY + 1) - (rowClouds - rowCloudsHalf);
		int32 startIndex = std::abs(std::min(0, rowMarginTop)) + m_firstRowIndex;
		int32 endIndex = std::min(rowClouds, rowClouds + rowMarginBottom) - 1 + m_firstRowIndex;
		int32 y = std::max(0, rowMarginTop);
		int32 paddingLeft = m_safeZoneCenter.x + 1;
		int32 x = paddingLeft - m_startSafeZoneRadius;
		for (int32 i = startIndex; i <= endIndex; i++)
		{
			TileCoord coord(x, y);
			CloudData* data = new CloudData();
			data->afterLoopsAction = CloudData::ACTION_ENABLE_RANDOM_POSITION;
			float delay;
			bool isLarge;
			if (i % 2 == 0)
			{
				data->isSpawnable = true;
				isLarge = i % 4 != 0;
			}
			else
			{
				data->isSpawnable = false;
				isLarge = (i + 1) % 4 == 0;
			}
			if (isLarge)
				delay = CloudCluster::LARGE_CLOUD_DELAY;
			else
				delay = CloudCluster::SMALL_CLOUD_DELAY;

			data->tileCoord = coord;
			data->time = time;
			data->delay = delay;
			dataRow.pushBack(data);
			data->release();

			y++;
		}
	}
	else if (m_position == POSITION_TOP)
	{
		int32 lastRowX = m_safeZoneCenter.x + 1;
//		int32 lastRowY = m_safeZoneCenter.y;
		int32 rowClouds = (m_initialSafeZoneRadius * 2 - 1) - m_firstRowIndex * 2;
		int32 rowCloudsHalf = (int32)std::ceil((float)rowClouds / 2);
		int32 rowMarginLeft = lastRowX + 1 - rowCloudsHalf;
		int32 rowMarginRight = mapWidth - (lastRowX + 1) - (rowClouds - rowCloudsHalf);
		int32 startIndex = std::abs(std::min(0, rowMarginLeft)) + m_firstRowIndex;
		int32 endIndex = std::min(rowClouds, rowClouds + rowMarginRight) - 1 + m_firstRowIndex;
		int32 x = std::max(0, rowMarginLeft);
		int32 paddingTop = m_safeZoneCenter.y + 1;
		int32 y = paddingTop - m_startSafeZoneRadius;
		for (int32 i = startIndex; i <= endIndex; i++)
		{
			TileCoord coord(x, y);
			CloudData* data = new CloudData();
			data->afterLoopsAction = CloudData::ACTION_ENABLE_RANDOM_POSITION;
			float delay;
			bool isLarge;
			if (i % 2 != 0)
			{
				data->isSpawnable = true;

				isLarge = (i + 1) % 4 != 0;
			}
			else
			{
				data->isSpawnable = false;

				isLarge = i % 4 == 0;
			}
			if (isLarge)
				delay = CloudCluster::LARGE_CLOUD_DELAY;
			else
				delay = CloudCluster::SMALL_CLOUD_DELAY;

			data->tileCoord = coord;
			data->time = time;
			data->delay = delay;
			dataRow.pushBack(data);
			data->release();

			x++;
		}

	}
	else if (m_position == POSITION_RIGHT)
	{
//		int32 lastRowX = m_safeZoneCenter.x + 1;
		int32 lastRowY = m_safeZoneCenter.y + 1;
		int32 rowClouds = (m_initialSafeZoneRadius * 2 - 1) - m_firstRowIndex * 2;
		int32 rowCloudsHalf = (int32)std::ceil((float)rowClouds / 2);
		int32 rowMarginTop = lastRowY + 1 - rowCloudsHalf;
		int32 rowMarginBottom = mapHeight - (lastRowY + 1) - (rowClouds - rowCloudsHalf);
		int32 startIndex = std::abs(std::min(0, rowMarginTop)) + m_firstRowIndex;
		int32 endIndex = std::min(rowClouds, rowClouds + rowMarginBottom) - 1 + m_firstRowIndex;
		int32 y = std::max(0, rowMarginTop);
		int32 paddingRight = mapWidth - (m_safeZoneCenter.x + 1);
		int32 x = mapWidth + (m_startSafeZoneRadius - paddingRight) - 1;
		for (int32 i = startIndex; i <= endIndex; i++)
		{
			TileCoord coord(x, y);
			CloudData* data = new CloudData();
			data->afterLoopsAction = CloudData::ACTION_ENABLE_RANDOM_POSITION;
			float delay;
			bool isLarge;
			if (i % 2 == 0)
			{
				data->isSpawnable = true;
				isLarge = i % 4 == 0;
				// Reverse large and small
				if (m_initialSafeZoneRadius % 2 == 0)
					isLarge = !isLarge;
			}
			else
			{
				data->isSpawnable = false;
				isLarge = (i + 1) % 4 == 0;
			}
			if (isLarge)
				delay = CloudCluster::LARGE_CLOUD_DELAY;
			else
				delay = CloudCluster::SMALL_CLOUD_DELAY;

			data->tileCoord = coord;
			data->time = time;
			data->delay = delay;
			dataRow.pushBack(data);
			data->release();

			y++;
		}
	}
	else if (m_position == POSITION_BOTTOM)
	{
		int32 lastRowX = m_safeZoneCenter.x;
//		int32 lastRowY = m_safeZoneCenter.y + 1;
		int32 rowClouds = (m_initialSafeZoneRadius * 2 - 1) - m_firstRowIndex * 2;
		int32 rowCloudsHalf = (int32)std::ceil((float)rowClouds / 2);
		int32 rowMarginLeft = lastRowX + 1 - rowCloudsHalf;
		int32 rowMarginRight = mapWidth - (lastRowX + 1) - (rowClouds - rowCloudsHalf);
		int32 startIndex = std::abs(std::min(0, rowMarginLeft)) + m_firstRowIndex;
		int32 endIndex = std::min(rowClouds, rowClouds + rowMarginRight) - 1 + m_firstRowIndex;
		int32 x = std::max(0, rowMarginLeft);
		int32 paddingBottom = mapHeight - (m_safeZoneCenter.y + 1);
		int32 y = mapHeight + (m_startSafeZoneRadius - paddingBottom) - 1;
		for (int32 i = startIndex; i <= endIndex; i++)
		{
			TileCoord coord(x, y);
			CloudData* data = new CloudData();
			data->afterLoopsAction = CloudData::ACTION_ENABLE_RANDOM_POSITION;
			float delay;
			bool isLarge;
			if (i % 2 != 0)
			{
				data->isSpawnable = true;

				isLarge = (i + 1) % 4 != 0;
				// Reverse large and small
				if (m_initialSafeZoneRadius % 2 == 0)
					isLarge = !isLarge;
			}
			else
			{
				data->isSpawnable = false;

				isLarge = i % 4 != 0;
			}
			if (isLarge)
				delay = CloudCluster::LARGE_CLOUD_DELAY;
			else
				delay = CloudCluster::SMALL_CLOUD_DELAY;

			data->tileCoord = coord;
			data->time = time;
			data->delay = delay;
			dataRow.pushBack(data);
			data->release();

			x++;
		}
	}

	m_cloudDataRowList.push_back(dataRow);
}

void CloudGenerator::spawnRowClouds(int32 rowIndex, float interval, float time)
{
	if (m_cloudDataRowList.empty())
		return;

	CloudDataRow& cloudList = m_cloudDataRowList.back();
	if (cloudList.empty())
		return;

	CloudDataRow newRow;

	// Calculate the margin of the last row
	int32 mapWidth = (int32)m_mapData->getMapSize().width;
	int32 mapHeight = (int32)m_mapData->getMapSize().height;
	int32 rowMarginFront = 0;
	int32 rowMarginBack = 0;
	TileCoord offset;
	if (m_position == POSITION_LEFT)
	{
//		int32 lastRowX = m_safeZoneCenter.x;
		int32 lastRowY = m_safeZoneCenter.y;
		int32 rowClouds = (m_initialSafeZoneRadius * 2 - 1) - (rowIndex - 1) * 2;
		int32 rowCloudsHalf = (int32)std::ceil((float)rowClouds / 2);
		rowMarginFront = lastRowY + 1 - rowCloudsHalf;
		rowMarginBack = mapHeight - (lastRowY + 1) - (rowClouds - rowCloudsHalf);
		offset.x = 1;
		offset.y = 0;
	}
	else if (m_position == POSITION_TOP)
	{
		int32 lastRowX = m_safeZoneCenter.x + 1;
//		int32 lastRowY = m_safeZoneCenter.y; 
		int32 rowClouds = (m_initialSafeZoneRadius * 2 - 1) - (rowIndex - 1) * 2;
		int32 rowCloudsHalf = (int32)std::ceil((float)rowClouds / 2);
		rowMarginFront = lastRowX + 1 - rowCloudsHalf;
		rowMarginBack = mapWidth - (lastRowX + 1) - (rowClouds - rowCloudsHalf);
		offset.x = 0;
		offset.y = 1;
	}
	else if (m_position == POSITION_RIGHT)
	{
//		int32 lastRowX = m_safeZoneCenter.x + 1;
		int32 lastRowY = m_safeZoneCenter.y + 1;
		int32 rowClouds = (m_initialSafeZoneRadius * 2 - 1) - (rowIndex - 1) * 2;
		int32 rowCloudsHalf = (int32)std::ceil((float)rowClouds / 2);
		rowMarginFront = lastRowY + 1 - rowCloudsHalf;
		rowMarginBack = mapHeight - (lastRowY + 1) - (rowClouds - rowCloudsHalf);
		offset.x = -1;
		offset.y = 0;
	}
	else if (m_position == POSITION_BOTTOM)
	{
		int32 lastRowX = m_safeZoneCenter.x;
//		int32 lastRowY = m_safeZoneCenter.y + 1;
		int32 rowClouds = (m_initialSafeZoneRadius * 2 - 1) - (rowIndex - 1) * 2;
		int32 rowCloudsHalf = (int32)std::ceil((float)rowClouds / 2);
		rowMarginFront = lastRowX + 1 - rowCloudsHalf;
		rowMarginBack = mapWidth - (lastRowX + 1) - (rowClouds - rowCloudsHalf);
		offset.x = 0;
		offset.y = -1;
	}

	int32 nClouds = (int32)cloudList.size();
	int32 startIndex;
	int32 endIndex;
	if (rowMarginFront >= 0 && rowIndex > m_firstRowIndex)
		startIndex = 1;
	else
		startIndex = 0;
	if (rowMarginBack >= 0 && rowIndex > m_firstRowIndex)
		endIndex = nClouds - 2;
	else
		endIndex = nClouds - 1;
	for (auto i = 0; i < nClouds; i++)
	{
		CloudData* data = cloudList.at(i);
		float delay = 0.f;

		if (i >= startIndex && i <= endIndex)
		{
			TileCoord newCoord = data->tileCoord + offset;
			CloudData* newData = new CloudData();
			newData->tileCoord = newCoord;
			newData->isSpawnable = data->isSpawnable;
			newData->afterLoopsAction = CloudData::ACTION_ENABLE_RANDOM_POSITION;

			// Reverse large and small
			if (newData->isSpawnable && rowIndex % 2 == 0)
			{
				if (interval > data->delay)
				{
					float d = interval - data->delay;
					float m = std::fmod(d, CloudCluster::ACTIVE_CLOUD_DURATION);
					if (m > CloudCluster::SMALL_CLOUD_DELAY)
						delay = CloudCluster::ACTIVE_CLOUD_DURATION - m + CloudCluster::SMALL_CLOUD_DELAY;
					else
						delay = CloudCluster::SMALL_CLOUD_DELAY - m;
				}
				else
				{
					float d = data->delay - interval;
					if (d >= CloudCluster::SMALL_CLOUD_DELAY)
						delay = d - CloudCluster::SMALL_CLOUD_DELAY;
					else
						delay = d + CloudCluster::SMALL_CLOUD_DELAY;
				}
			}
			else
			{
				if (interval > data->delay)
				{
					float d = interval - data->delay;
					float m = std::fmod(d, CloudCluster::ACTIVE_CLOUD_DURATION);
					if (m != 0)
						delay = CloudCluster::ACTIVE_CLOUD_DURATION - m;
					else
						delay = 0;
				}
				else
				{
					float d = data->delay - interval;
					delay = std::fmod(d, CloudCluster::ACTIVE_CLOUD_DURATION);
				}
			}
			if (rowIndex == m_initialSafeZoneRadius - 1)
			{
				newData->loops = (int32)std::ceil((m_interval - data->delay) / CloudCluster::ACTIVE_CLOUD_DURATION);
				newData->loops = std::max(1, newData->loops);
				if (rowIndex % 2 != 0 || !newData->isSpawnable)
				{
					newData->afterLoopsAction = CloudData::ACTION_STOP;
				}
			}
			newData->delay = delay;
			newData->time = time;
			newRow.pushBack(newData);
			newData->release();
		}

		data->loops = (int32)std::ceil((m_interval - data->delay + delay) / CloudCluster::ACTIVE_CLOUD_DURATION);
		data->loops = std::max(1, data->loops);
		if (!data->isSpawnable || rowIndex % 2 == 0)
		{
			data->afterLoopsAction = CloudData::ACTION_STOP;
		}
	}

	m_cloudDataRowList.push_back(newRow);
}

std::vector<CloudDataRow> const& CloudGenerator::spawnClouds()
{
	m_cloudDataRowList.clear();

	float time = m_startTime;
	this->createParentClouds(time);
	for (int32 r = m_startSafeZoneRadius - 1; r > 0; --r)
	{
		time += m_interval;
		int32 currRowIndex = m_initialSafeZoneRadius - r;
		this->spawnRowClouds(currRowIndex, m_interval, time);
	}

#if ENABLE_SPAWN_CLOUDS_LOG
	int32 cloudCount = 0;
	int32 nRows = (int32)m_cloudDataRowList.size();
	for (int32 i = 0; i < nRows; ++i)
	{
		auto const& row = m_cloudDataRowList[i];
		auto rowIt = row.begin();
		if (rowIt != row.end())
		{
			CCLOG("time: %f currRowIndex: %d", (*rowIt)->time, m_initialSafeZoneRadius - (m_startSafeZoneRadius - i));
			for (; rowIt != row.end(); ++rowIt)
			{
				CloudData* data = *rowIt;
				cloudCount++;
				CCLOG("tile: %d,%d delay: %f spawnable: %d loops: %d duration: %f", data->tileCoord.x, data->tileCoord.y, data->delay, data->isSpawnable, data->loops, CloudCluster::ACTIVE_CLOUD_DURATION);
			}
		}
	}
	CCLOG("Total number of spawned clouds: %d", cloudCount);
#endif

	return m_cloudDataRowList;
}

NS_END
