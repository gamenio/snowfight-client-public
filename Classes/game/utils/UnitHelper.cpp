#include "UnitHelper.h"

#include "common/utils/MathTools.h"
#include "game/maps/MapData.h"
#include "game/entities/DataUnit.h"
#include "game/behaviors/SharedUnitDefines.h"

NS_BEGIN


bool UnitHelper::testSightLine(MapData const* mapData, int32 x1, int32 y1, int32 x2, int32 y2)
{
	// Bresenham's line algorithm
	//https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
	//http://rosettacode.org/wiki/Bitmap/Bresenham%27s_line_algorithm

	bool steep = (abs(y2 - y1) > abs(x2 - x1));
	if (steep)
	{
		std::swap(x1, y1);
		std::swap(x2, y2);
	}

	if (x1 > x2)
	{
		std::swap(x1, x2);
		std::swap(y1, y2);
	}

	float dx = float(x2 - x1);
	float dy = float(std::abs(y2 - y1));
	float error = dx / 2.0f;

	int32 ystep = (y1 < y2) ? 1 : -1;
	int32 y = y1;
	bool gap = false; // When Y changes, fill in the gap to prevent the line of sight from crossing the edge of the tile.

	int32 maxX = x2;
	TileCoord current;
	TileCoord left;
	TileCoord upright;

	for (int32 x = x1; x <= maxX; x++)
	{
		if (steep)
		{
			current.setTileCoord(y, x);
			if (gap)
			{
				left.setTileCoord(y, x - 1);
				upright.setTileCoord(y - ystep, x);
			}

		}
		else
		{
			current.setTileCoord(x, y);
			if (gap)
			{
				left.setTileCoord(x - 1, y);
				upright.setTileCoord(x, y - ystep);
			}
		}

		//if (gap)
		//	CCLOG("SightLine %d,%d %d,%d %d,%d", current.x, current.y, left.x, left.y, upright.x, upright.y);
		//else
		//	CCLOG("SightLine %d,%d", current.x, current.y);

		if (mapData->isCollidable(current) || (gap && (mapData->isCollidable(left) || mapData->isCollidable(upright))))
			return false;

		gap = false;
		error -= dy;
		if (error < 0)
		{
			y += ystep;
			error += dx;

			gap = true;
		}
	}

	return true;
}

Point UnitHelper::computeLandingPosition(DataUnit const* launcher, float direction)
{
	return computeLandingPosition(launcher->getPosition(), launcher->getAttackRange(), direction);
}

Point UnitHelper::computeLandingPosition(Point const& launcherPos, float attackRange, float direction)
{
	Point landingPos;
	Point startPos = launcherPos;
	float dist = attackRange;
	float dx = cos(direction) * dist;
	float dy = sin(direction) * dist;
	landingPos.setPoint(startPos.x + dx, startPos.y + dy);

	return landingPos;
}

Point UnitHelper::computeLaunchPosition(DataUnit const* launcher, Point const& targetPos)
{
	return computeLaunchPosition(launcher->getMapData(), launcher->getPosition(), launcher->getLaunchCenter(), launcher->getLaunchRadiusInMap(), targetPos);
}

Point UnitHelper::computeLaunchPosition(MapData const* mapData, Point const& launcherPos, Point const& launchCenter, float launchRadiusInMap,  Point const& targetPos)
{
	Point launcherMapPos = mapData->openGLToMapPos(launcherPos);
	Point targetMapPos = mapData->openGLToMapPos(targetPos);
	Point point = MathTools::findPointAlongLine(launcherMapPos, targetMapPos, launchRadiusInMap);
	point = mapData->mapToOpenGLPos(point);
	Point result;
	result.x = point.x;
	result.y = point.y + launchCenter.y;
	return result;
}

NS_END