#ifndef __TILE_COORD_H__
#define __TILE_COORD_H__

#include "cocos2d.h"

#include "common/Common.h"
#include "TileDefines.h"

USING_NS_CC;

NS_BEGIN

class MapData;


//单位在地图上的瓦片坐标，需要与用点作为单位的Vec2类区分开。
//瓦片位置为屏幕坐标系
struct TileCoord
{
	TileCoord() :x(0), y(0) 
	{
	}

	TileCoord(int32 xx, int32 yy) :
		x(xx), y(yy) 
	{
	}

	TileCoord(cocos2d::Size const& mapSize, cocos2d::Point const& position)
	{
		float ix = position.x - mapSize.width * TILE_WIDTH_HALF;
		float iy = mapSize.height * TILE_HEIGHT - position.y;
		this->x = (int32)floor((ix / TILE_WIDTH_HALF + iy / TILE_HEIGHT_HALF) / 2);
		this->y = (int32)floor((iy / TILE_HEIGHT_HALF - ix / TILE_WIDTH_HALF) / 2);
	}

	void setTileCoord(int32 x, int32 y)
	{
		this->x = x;
		this->y = y;
	}

	cocos2d::Point computePosition(cocos2d::Size const& mapSize) const
	{
		float x = mapSize.width * TILE_WIDTH_HALF + (this->x - this->y) * TILE_WIDTH_HALF;
		float y = mapSize.height * TILE_HEIGHT - (this->x + this->y) * TILE_HEIGHT_HALF - TILE_HEIGHT_HALF;

		return cocos2d::Point(x, y);
	}

	static const TileCoord ZERO;
	static const TileCoord INVALID;

	int32 x;
	int32 y;
};

inline bool operator==( TileCoord const& lhs,  TileCoord const& rhs) {
	return (lhs.x == rhs.x && lhs.y == rhs.y);
}


inline bool operator!=( TileCoord const& lhs, TileCoord const& rhs) {
	return !(lhs == rhs);
}


inline TileCoord operator-(TileCoord const& lhs, TileCoord const& rhs) {
	return TileCoord(lhs.x - rhs.x, lhs.y - rhs.y);
}

inline TileCoord operator+(TileCoord const& lhs, TileCoord const& rhs) {
	return TileCoord(lhs.x + rhs.x, lhs.y + rhs.y);
}


NS_END


#endif //__TILE_COORD_H__
