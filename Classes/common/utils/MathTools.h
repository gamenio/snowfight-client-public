#ifndef __MATH_TOOLS_H__
#define __MATH_TOOLS_H__

#include "common/Common.h"

USING_NS_CC;

NS_BEGIN

#ifndef M_PI
#define M_PI            3.14159265358979323846
#endif

#ifndef M_PI_2
#define M_PI_2			1.57079632679489661923   // pi/2
#endif

class MathTools
{
public:
	// 计算两点之间的弧度
	static float computeAngleInRadians(cocos2d::Point const& p1, cocos2d::Point const& p2);

	static bool isDegreesInRange(float deg, float center, float swingDeg);
	static float degrees2Radians(float deg);
	static float radians2Degrees(float rad);

	// 计算从p1到p2的一条线段与点p之间的最小距离
	static float minDistanceFromPointToSegment(cocos2d::Point const& p1, cocos2d::Point const& p2, cocos2d::Point const& p);
	// 求直线1和直线2相交的点
	static bool findIntersectionTwoLines(cocos2d::Point const& l1p1, cocos2d::Point const& l1p2, cocos2d::Point const& l2p1, cocos2d::Point const& l2p2, cocos2d::Point& result);

	// 沿着p1到p2的一条直线找到距离p1一定距离的点
	static cocos2d::Point findPointAlongLine(cocos2d::Point const& p1, cocos2d::Point const& p2, float distance);
	static bool isPointInsideCircle(cocos2d::Point const& center, float radius, cocos2d::Point const& p);

	static float computeMovingTimeSec(float distance, int32 speed);
	static float computeMovingDist(float time, int32 speed);

	static float roundFloatWithPrecision(float value, int32 precision);
};

inline float MathTools::computeMovingTimeSec(float distance, int32 speed)
{
	if(speed != 0)
		return distance / speed;

	return 0.f;
}

inline float MathTools::computeMovingDist(float time, int32 speed)
{
	return time * speed;
}

inline float MathTools::degrees2Radians(float deg)
{
	return deg * (float)M_PI / 180.0f;
}

inline float MathTools::radians2Degrees(float rad)
{
	float deg = rad * 180.0f / (float)M_PI;
	return deg;
}

inline float MathTools::roundFloatWithPrecision(float value, int32 precision)
{
	float ret = std::floor(value * std::powf(10, precision) + 0.5f) / std::powf(10, precision);
	return ret;
}

NS_END

#endif // __MATH_TOOLS_H__