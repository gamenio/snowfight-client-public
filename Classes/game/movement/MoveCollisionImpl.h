#ifndef __MOVE_COLLISION_IMPL_H__
#define __MOVE_COLLISION_IMPL_H__

#include "MoveCollision.h"

#include "common/utils/MathTools.h"
#include "game/maps/MapData.h"

NS_BEGIN

template<typename T>
MoveCollision<T>::MoveCollision(T* target):
	m_target(target),
	m_direction(0)
{
}

template<typename T>
MoveCollision<T>::~MoveCollision()
{
	m_target = nullptr;
}

template<typename T>
bool MoveCollision<T>::isWalkable(TileCoord const& tileCoord) const
{
	MapData const* mapData = m_target->getMap()->getMapData();
	if (!mapData->isValidTileCoord(tileCoord) || mapData->isWall(tileCoord) || m_target->getMap()->isTileClosed(tileCoord))
		return false;

	return true;
}

template<typename T>
bool MoveCollision<T>::isCirclesIntersect(cocos2d::Point const& p1, float r1, cocos2d::Point const& p2, float r2)
{
	float dx = p1.x - p2.x;
	float dy = p1.y - p2.y;
	float r = r1 + r2;

	return dx * dx + dy * dy < r * r;
}

template<typename T>
Point MoveCollision<T>::calculatePointCloseToBlock(cocos2d::Point const& blockPos, float blockSize, cocos2d::Point const& startPos, cocos2d::Point const& endPos, float collisionSize)
{
	float collisionSizeHalf = collisionSize / 2;
	float blockSizeHalf = blockSize / 2;

	cocos2d::Point origin = startPos;
	float d = collisionSizeHalf + blockSizeHalf;
	NS_ASSERT(d > 0);
	float dx = blockPos.x - startPos.x;
	float dy = blockPos.y - startPos.y;
	float rx = endPos.x - startPos.x;
	float ry = endPos.y - startPos.y;
	float r = std::sqrt(rx * rx + ry * ry);
	float c = std::sqrt(dx * dx + dy * dy);
	if (r == 0)
		return origin;

	float A = std::atan2(dy, dx);
	// 避免无法构成三角形(r+c<d)，以及c为0的情况出现
	if (c < d)
	{
		c = d;
		origin.x = blockPos.x - std::cos(A) * c;
		origin.y = blockPos.y - std::sin(A) * c;
	}
	float xx = (r * r + c * c - d * d) / (2 * r * c);
	float B = std::acos(xx);

	if (A < 0)
		A = 2 * M_PI + A;

	float rad;
	// https://math.stackexchange.com/questions/274712/calculate-on-which-side-of-a-straight-line-is-a-given-point-located/274728#274728
	float s = rx * dy - ry * dx;
	if (s > 0)
		rad = A - B;
	else
		rad = A + B;

	cocos2d::Point p;
	p.x = origin.x + std::cos(rad) * r;
	p.y = origin.y + std::sin(rad) * r;

	return p;
}

template<typename T>
cocos2d::Point MoveCollision<T>::moveInMapPos(cocos2d::Point const& from, cocos2d::Point const& to)
{
	MapData const* mapData = m_target->getMap()->getMapData();
	int32 mapWidth = mapData->getMapSize().width;
	int32 mapHeight = mapData->getMapSize().height;
	float maxWidth = mapData->getMapSize().width * TILE_HEIGHT;
	float maxHeight = mapData->getMapSize().height * TILE_HEIGHT;
	
	float collSize = m_target->getData()->getObjectSize().width;
	float collSizeHalf = collSize / 2;

	cocos2d::Point start = from;
	cocos2d::Point end = to;

	if (end.x - collSizeHalf < -1)
		end.x = collSizeHalf - 1;
	if (end.y - collSizeHalf < -1)
		end.y = collSizeHalf - 1;
	if (end.x + collSizeHalf > maxWidth - 1)
		end.x = maxWidth - collSizeHalf - 1;
	if (end.y + collSizeHalf > maxHeight - 1)
		end.y = maxHeight - collSizeHalf - 1;

	cocos2d::Point endTL(end.x - collSizeHalf, end.y - collSizeHalf);
	cocos2d::Point endBL(end.x - collSizeHalf, end.y + collSizeHalf);
	cocos2d::Point endTR(end.x + collSizeHalf, end.y - collSizeHalf);
	cocos2d::Point endBR(end.x + collSizeHalf, end.y + collSizeHalf);

	TileCoord startTile(std::min(mapWidth - 1, (int32)((start.x + 1) / TILE_HEIGHT)), std::min(mapHeight - 1, (int32)((start.y + 1) / TILE_HEIGHT)));
	TileCoord endTLTile(std::min(mapWidth - 1, (int32)((endTL.x + 1) / TILE_HEIGHT)), std::min(mapHeight - 1, (int32)((endTL.y + 1) / TILE_HEIGHT)));
	TileCoord endBLTile(std::min(mapWidth - 1, (int32)((endBL.x + 1) / TILE_HEIGHT)), std::min(mapHeight - 1, (int32)((endBL.y + 1) / TILE_HEIGHT)));
	TileCoord endTRTile(std::min(mapWidth - 1, (int32)((endTR.x + 1) / TILE_HEIGHT)), std::min(mapHeight - 1, (int32)((endTR.y + 1) / TILE_HEIGHT)));
	TileCoord endBRTile(std::min(mapWidth - 1, (int32)((endBR.x + 1) / TILE_HEIGHT)), std::min(mapHeight - 1, (int32)((endBR.y + 1) / TILE_HEIGHT)));

	if (!isWalkable(endTLTile) && !isWalkable(endBRTile))
	{
		float tlBlockMidX = endTLTile.x * TILE_HEIGHT + TILE_HEIGHT_HALF - 1;
		float tlBlockMidY = endTLTile.y * TILE_HEIGHT + TILE_HEIGHT_HALF - 1;
		float brBlockMidX = endBRTile.x * TILE_HEIGHT + TILE_HEIGHT_HALF - 1;
		float brBlockMidY = endBRTile.y * TILE_HEIGHT + TILE_HEIGHT_HALF - 1;

		float dx = brBlockMidX - tlBlockMidX;
		float dy = brBlockMidY - tlBlockMidY;
		float sx = start.x - tlBlockMidX;
		float sy = start.y - tlBlockMidY;
		float s = sx * dy - sy * dx;
		if (s < 0)
		{
			end.x = tlBlockMidX + TILE_HEIGHT_HALF - collSizeHalf;
			end.y = tlBlockMidY + TILE_HEIGHT_HALF + collSizeHalf;
		}
		else
		{
			end.x = tlBlockMidX + TILE_HEIGHT_HALF + collSizeHalf;
			end.y = tlBlockMidY + TILE_HEIGHT_HALF - collSizeHalf;
		}
	}
	else if (!isWalkable(endBLTile) && !isWalkable(endTRTile))
	{
		float blBlockMidX = endBLTile.x * TILE_HEIGHT + TILE_HEIGHT_HALF - 1;
		float blBlockMidY = endBLTile.y * TILE_HEIGHT + TILE_HEIGHT_HALF - 1;
		float trBlockMidX = endTRTile.x * TILE_HEIGHT + TILE_HEIGHT_HALF - 1;
		float trBlockMidY = endTRTile.y * TILE_HEIGHT + TILE_HEIGHT_HALF - 1;

		float dx = trBlockMidX - blBlockMidX;
		float dy = trBlockMidY - blBlockMidY;
		float sx = start.x - blBlockMidX;
		float sy = start.y - blBlockMidY;
		float s = sx * dy - sy * dx;
		if (s > 0)
		{
			end.x = blBlockMidX + TILE_HEIGHT_HALF - collSizeHalf;
			end.y = blBlockMidY - TILE_HEIGHT_HALF - collSizeHalf;
		}
		else
		{
			end.x = blBlockMidX + TILE_HEIGHT_HALF + collSizeHalf;
			end.y = blBlockMidY - TILE_HEIGHT_HALF + collSizeHalf;
		}
	}
	else if (!isWalkable(endTLTile) && (!isWalkable(endBLTile) || !isWalkable(endTRTile)))
	{
		float blockMaxX = endTLTile.x * TILE_HEIGHT + TILE_HEIGHT - 1;
		float blockMaxY = endTLTile.y * TILE_HEIGHT + TILE_HEIGHT - 1;

		if (!isWalkable(endBLTile))
			end.x = blockMaxX + collSizeHalf;
		if (!isWalkable(endTRTile))
			end.y = blockMaxY + collSizeHalf;
	}

	else if (!isWalkable(endBRTile) && (!isWalkable(endTRTile) || !isWalkable(endBLTile)))
	{
		float blockMinX = endBRTile.x * TILE_HEIGHT - 1;
		float blockMinY = endBRTile.y * TILE_HEIGHT - 1;

		if (!isWalkable(endTRTile))
			end.x = blockMinX - collSizeHalf;
		if (!isWalkable(endBLTile))
			end.y = blockMinY - collSizeHalf;
	}
	else
	{
		if (!isWalkable(endTLTile))
		{
			float blockMidX = endTLTile.x * TILE_HEIGHT + TILE_HEIGHT_HALF - 1;
			float blockMidY = endTLTile.y * TILE_HEIGHT + TILE_HEIGHT_HALF - 1;
			if (this->isCirclesIntersect(end, collSizeHalf, Vec2(blockMidX, blockMidY), TILE_HEIGHT_HALF))
			{
				end = this->calculatePointCloseToBlock(Vec2(blockMidX, blockMidY), TILE_HEIGHT, start, end, collSize);
			}

		}
		else if (!isWalkable(endBLTile))
		{
			float blockMidX = endBLTile.x * TILE_HEIGHT + TILE_HEIGHT_HALF - 1;
			float blockMidY = endBLTile.y * TILE_HEIGHT + TILE_HEIGHT_HALF - 1;
			if (this->isCirclesIntersect(end, collSizeHalf, Vec2(blockMidX, blockMidY), TILE_HEIGHT_HALF))
			{
				end = this->calculatePointCloseToBlock(Vec2(blockMidX, blockMidY), TILE_HEIGHT, start, end, collSize);
			}
		}
		else if (!isWalkable(endTRTile))
		{
			float blockMidX = endTRTile.x * TILE_HEIGHT + TILE_HEIGHT_HALF - 1;
			float blockMidY = endTRTile.y * TILE_HEIGHT + TILE_HEIGHT_HALF - 1;
			if (this->isCirclesIntersect(end, collSizeHalf, Vec2(blockMidX, blockMidY), TILE_HEIGHT_HALF))
			{
				end = this->calculatePointCloseToBlock(Vec2(blockMidX, blockMidY), TILE_HEIGHT, start, end, collSize);
			}
		}
		else if (!isWalkable(endBRTile))
		{
			float blockMidX = endBRTile.x * TILE_HEIGHT + TILE_HEIGHT_HALF - 1;
			float blockMidY = endBRTile.y * TILE_HEIGHT + TILE_HEIGHT_HALF - 1;
			if (this->isCirclesIntersect(end, collSizeHalf, Vec2(blockMidX, blockMidY), TILE_HEIGHT_HALF))
			{
				end = this->calculatePointCloseToBlock(Vec2(blockMidX, blockMidY), TILE_HEIGHT, start, end, collSize);
			}

		}
	}

	return end;
}

template<typename T>
void MoveCollision<T>::step(float dt)
{
	if (!m_target)
		return;

	MapData const* mapData = m_target->getMap()->getMapData();

	float moveDist = m_target->getData()->getMoveSpeed() * dt;
	float dx = std::cos(m_direction) * moveDist;
	float dy = std::sin(m_direction) * moveDist;
	cocos2d::Point currPos = m_target->getData()->getPosition();
	cocos2d::Point nextPos(currPos.x + dx, currPos.y + dy);

	cocos2d::Point currMapPos = mapData->openGLToMapPos(currPos);
	cocos2d::Point nextMapPos = mapData->openGLToMapPos(nextPos);
	cocos2d::Point newMapPos = this->moveInMapPos(currMapPos, nextMapPos);
	// 发生碰撞
	if (newMapPos != nextMapPos)
	{
		nextPos = mapData->mapToOpenGLPos(newMapPos);
		float newDist = currPos.getDistance(nextPos);
		// 新的距离不能超出移动距离
		if (newDist > moveDist)
		{
			float rad = MathTools::computeAngleInRadians(currPos, nextPos);
			dx = std::cos(rad) * moveDist;
			dy = std::sin(rad) * moveDist;
			nextPos.setPoint(currPos.x + dx, currPos.y + dy);
		}
	}
	m_target->updatePosition(nextPos);
}

NS_END

#endif // __MOVE_COLLISION_IMPL_H__