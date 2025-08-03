#ifndef __HIDINGSPOT_SEARCHER_H__
#define __HIDINGSPOT_SEARCHER_H__

#include "common/Common.h"
#include "game/maps/MapData.h"

USING_NS_CC;

NS_BEGIN

class HidingSpotSearcher
{
	class HidingSpotInfoRef;

public:
	enum VisibleRange
	{
		VIEWPORT,
		POINT,
	};

	HidingSpotSearcher(MapData* mapData);
	~HidingSpotSearcher();

	void setDebugDraw(DrawNode* node);

	void update(TileCoord const& center, std::vector<HidingSpotInfo>& visibleList, std::vector<HidingSpotInfo>& outOfRangeList);
	void setVisibleRange(VisibleRange range);
	VisibleRange getVisibleRange() const { return m_visibleRange; }

private:
	void drawRhombus(DrawNode* drawNode, Vec2 const& origin, cocos2d::Size const& size, Color4F const& color);
	void drawTileBorder(DrawNode* drawNode, TileCoord const& coord, Color4F const& color);

	void updateHidingSpotOnVisibleRange(TileCoord const& center, std::vector<HidingSpotInfo>& visibleList);
	void cleanupHidingSpotsOutsideVisibleRange(std::vector<HidingSpotInfo>& outOfRangeList);

	DrawNode* m_debugDraw;

	MapData* m_mapData;

	VisibleRange m_visibleRange;
	cocos2d::Size m_viewport;
	int32 m_extendedRangeInTiles;
	bool* m_visibleHidingSpotFlags;
	std::unordered_map<int32/* RefIndex */, HidingSpotInfoRef*> m_hidingSpotInfoRefs;
	std::unordered_map<int32 /* TileIndex */, int32/* RefIndex */> m_hidingSpots;
};

NS_END

#endif // __HIDINGSPOT_SEARCHER_H__
