#ifndef __CLOUD_GENERATOR_H__
#define __CLOUD_GENERATOR_H__

#include "common/Common.h"
#include "game/maps/MapData.h"

USING_NS_CC;

NS_BEGIN

class CloudData: public Ref
{
public:
	enum Action
	{
		ACTION_NONE,
		ACTION_ENABLE_RANDOM_POSITION,
		ACTION_STOP,
	};
	CloudData():
		isSpawnable(false),
		afterLoopsAction(ACTION_NONE),
		time(0),
		delay(0),
		loops(0)
	{
	}
	~CloudData()
	{
	}

	bool isSpawnable;
	Action afterLoopsAction; // Valid when loops is greater than 0
	TileCoord tileCoord;
	float time;
	float delay;
	int32 loops; // Infinite loop when the value is 0
};
typedef Vector<CloudData*> CloudDataRow;

class CloudGenerator
{
public:
	enum Position
	{
		POSITION_TOP,
		POSITION_LEFT,
		POSITION_BOTTOM,
		POSITION_RIGHT,
	};

	CloudGenerator();
	~CloudGenerator();

	void init(Position position, MapData* mapData, TileCoord const& safeZoneCenter, int32 initialSafeZoneRadius, NSTime duration);
	std::vector<CloudDataRow> const& spawnClouds();

private:
	void createParentClouds(float time);
	void spawnRowClouds(int32 rowIndex, float interval, float time);

	Position m_position;
	MapData* m_mapData;
	TileCoord m_safeZoneCenter;
	int32 m_startSafeZoneRadius;
	int32 m_initialSafeZoneRadius;

	std::vector<CloudDataRow> m_cloudDataRowList;

	float m_startTime;
	int32 m_firstRowIndex;
	float m_interval;
};

NS_END

#endif // __CLOUD_GENERATOR_H__
