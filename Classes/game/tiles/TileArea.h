#ifndef __TILE_AREA_H__
#define __TILE_AREA_H__

#include "common/Common.h"
#include "TileCoord.h"


NS_BEGIN


struct TileArea
{
	TileArea() {}
	TileArea(TileCoord const& low, TileCoord const& high) :
		lowBound(low),
		highBound(high) {}

	TileCoord lowBound;
	TileCoord highBound;

	inline bool containsTileCoord(TileCoord const& coord) const
	{
		return (coord.x >= lowBound.x && coord.y >= lowBound.y && coord.x <= highBound.x && coord.y <= highBound.y);
	}
};


inline TileArea computeTileAreaInViewport(cocos2d::Point const& center, cocos2d::Size const& viewport, Size const& mapSize)
{
	float halfWidth = viewport.width * 0.5f;
	float halfHeight = viewport.height * 0.5f;

	TileCoord lefttop(mapSize, cocos2d::Point(center.x - halfWidth, center.y + halfHeight));
	TileCoord righttop(mapSize, cocos2d::Point(center.x + halfWidth, center.y + halfHeight));
	TileCoord leftbottom(mapSize, cocos2d::Point(center.x - halfWidth, center.y - halfHeight));
	TileCoord rightbottom(mapSize, cocos2d::Point(center.x + halfWidth, center.y - halfHeight));

	TileCoord low(std::max(lefttop.x, 0), std::max(righttop.y, 0));
	TileCoord high(std::min(rightbottom.x, (int32)(mapSize.width - 1)), std::min(leftbottom.y, (int32)(mapSize.height - 1)));

	return TileArea(low, high);
}

NS_END


#endif //__TILE_AREA_H__