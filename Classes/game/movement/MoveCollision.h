#ifndef __MOVE_COLLISION_H__
#define __MOVE_COLLISION_H__

#include "common/Common.h"
#include "game/tiles/TileCoord.h"

USING_NS_CC;

NS_BEGIN

template<typename T>
class MoveCollision
{
public:
	MoveCollision(T* target);
	~MoveCollision();

	void setDirection(float rad) { m_direction = rad; }
	float getDirection() const {  return m_direction; }

	void step(float dt);

private:
	bool isWalkable(TileCoord const& tileCoord) const;
	bool isCirclesIntersect(cocos2d::Point const& p1, float r1, cocos2d::Point const& p2, float r2);
	cocos2d::Point calculatePointCloseToBlock(cocos2d::Point const& blockPos, float blockSize, cocos2d::Point const& startPos, cocos2d::Point const& endPos, float collisionSize);
	cocos2d::Point moveInMapPos(cocos2d::Point const& from, cocos2d::Point const& to);

	T* m_target;

	float m_direction;
};

NS_END

#endif // __MOVE_COLLISION_H__
