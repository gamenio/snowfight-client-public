#ifndef __INTERSECTION_H__
#define __INTERSECTION_H__

#include "common/Common.h"


USING_NS_CC;

NS_BEGIN

class Intersection
{
public:
	/**
	* est rect and polygon
	* @param a - The rect
	* @param b - The polygon, a set of points
	* @return
	*/
	static bool rectPolygon(cocos2d::Rect const& a, std::vector<Vec2> const& b);

	/**
	* Test line and polygon
	* @param a1 - The start point of the line
	* @param a2 - The end point of the line
	* @param b - The polygon, a set of points
	* @return
	*/
	static bool linePolygon(Vec2 const& a1, Vec2 const& a2, std::vector<Vec2> const& b);

	/**
	* Test whether the point is in the polygon
	* @param point - The point
	* @param polygon - The polygon, a set of points
	* @return
	*/
	static bool pointInPolygon(Vec2 const& point, std::vector<Vec2> const& polygon);


	/**
	* Test line and line
	* @param a1 - The start point of the first line
	* @param a2 - The end point of the first line
	* @param b1 - The start point of the second line
	* @param b2 - The end point of the second line
	* @return
	*/
	static bool lineLine(Vec2 const& a1, Vec2 const& a2, Vec2 const& b1, Vec2 const& b2);


};

NS_END

#endif // __INTERSECTION_H__