#ifndef __SAFE_ZONE_H__
#define __SAFE_ZONE_H__

#include "common/Common.h"
#include "game/maps/MapData.h"
#include "CloudGenerator.h"

USING_NS_CC;

NS_BEGIN

class SafeZone: public Ref
{
public:
	SafeZone(MapData* mapData, TileCoord const& center, int32 radius, NSTime duration);
	~SafeZone();

	void createClouds();

	std::vector<CloudData*> const& getCloudDataSet() const { return m_cloudDataSet; }
	TileCoord getCenter() const { return m_center; }
	int32 getRadius() const { return m_radius; }
	NSTime getDuration() const { return m_duration; }

private:
	void createCloudData(CloudGenerator::Position pos, TileCoord const& safeZoneCenter, int32 initialSafeZoneRadius, NSTime duration);
	void clearCloudDataSet();

	MapData* m_mapData;
	std::vector<CloudData*> m_cloudDataSet;
	TileCoord m_center;
	int32 m_radius;
	NSTime m_duration;
};


NS_END

#endif // __SAFE_ZONE_H__
