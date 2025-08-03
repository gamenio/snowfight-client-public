#ifndef __UNIT_HELPER_H__
#define __UNIT_HELPER_H__

#include "common/Common.h"

USING_NS_CC;

NS_BEGIN


class MapData;
class DataUnit;

class UnitHelper
{
public:
	static bool testSightLine(MapData const* mapData, int32 x1, int32 y1, int32 x2, int32 y2);

	static cocos2d::Point computeLandingPosition(DataUnit const* launcher, float direction);
	static cocos2d::Point computeLandingPosition(cocos2d::Point const& launcherPos, float attackRange, float direction);

	static cocos2d::Point computeLaunchPosition(DataUnit const* launcher, cocos2d::Point const& targetPos);
	static cocos2d::Point computeLaunchPosition(MapData const* mapData, cocos2d::Point const& launcherPos, cocos2d::Point const& launchCenter, float launchRadiusInMap, cocos2d::Point const& targetPos);

};


NS_END


#endif // __UNIT_HELPER_H__