#ifndef __DANGER_ZONE_H__
#define __DANGER_ZONE_H__

#include "common/Common.h"
#include "game/maps/MapData.h"
#include "scene/ObjectPools.h"
#include "CloudGenerator.h"
#include "CloudCluster.h"
#include "SafeZone.h"

USING_NS_CC;

NS_BEGIN

class SafeZoneListener
{
public:
	virtual void onSafeZoneStarted(TileCoord const& center, int32 initialRadius) {}
	virtual void onSafeZoneUpdated(int32 currRadius) {}
	virtual void onSafeZoneStopped() {}
};

class DangerZone : public Node, CloudClusterPoolListener
{
public:
	static DangerZone* create(MapData* mapData, cocos2d::Rect const& viewport);
	bool init(MapData* mapData, cocos2d::Rect const& viewport);

	DangerZone();
	~DangerZone();

	void setSafeZone(SafeZone* safeZone);
	SafeZone* getSafeZone() const { return m_safeZone; }
	bool isSafeZoneReady() const;

	void addSafeZoneListener(SafeZoneListener* listener);
	void removeSafeZoneListener(SafeZoneListener* listener);

	void start(NSTime sartTime = -1);
	void stop();
	bool isStopped() const { return m_isStopped; }

	void update(float delta) override;
	void onEnter() override;

private:
	void initCloudsOnViewport();

	void drawRhombus(Brush* drawNode, Vec2 const& origin, cocos2d::Size const& size, Color4F const& color);
	void drawSafeZoneBorder(Brush* drawNode, TileCoord const& center, int32 radius, Color4F const& color);
	void drawTileBorder(Brush* drawNode, TileCoord const& coord, Color4F const& color);

	void updateSafeZone(float delta);

	void updateCloudOnViewport(float delta);
	void updateCloud(TileCoord const& coord, float delta);
	void cleanupCloudsOutsideViewport();

	int32 getTileIndex(TileCoord const& coord);
	int32 getVertexZForTile(TileCoord const& coord);

	// CloudClusterPoolListener
	void onObjectPoolIncreased(std::unordered_set<CloudCluster*> const& objects) override;

	Brush* m_debugDraw;
	MapData* m_mapData;
	cocos2d::Rect m_viewport;
	std::vector<SafeZoneListener*> m_safeZoneListeners;

	SafeZone* m_safeZone;
	Map<int32, CloudCluster*> m_cloudClusters;
	bool* m_visibleCloudFlags;
	int32 m_currSafeZoneRadius;

	bool m_isStopped;
	NSTime m_startTime;
	NSTime m_elapsed;
	NSTime m_lastUpdateTime;
};


NS_END

#endif // __DANGER_ZONE_H__
