#include "Intersection.h"

NS_BEGIN

bool Intersection::rectPolygon(Rect const& a, std::vector<Vec2> const& b)
{
	Vec2 r0(a.getMinX(), a.getMinY());
	Vec2 r1(a.getMinX(), a.getMaxY());
	Vec2 r2(a.getMaxX(), a.getMaxY());
	Vec2 r3(a.getMaxX(), a.getMinY());

	// intersection check
	if (linePolygon(r0, r1, b))
		return true;

	if (linePolygon(r1, r2, b))
		return true;

	if (linePolygon(r2, r3, b))
		return true;

	if (linePolygon(r3, r0, b))
		return true;

	// check if a contains b
	for (size_t i = 0, l = b.size(); i < l; ++i) 
	{
		//if (pointInPolygon(b[i], a))
		//	return true;
		if (a.containsPoint(b[i]))
			return true;
	}

	// check if b contains a
	if (pointInPolygon(r0, b))
		return true;

	if (pointInPolygon(r1, b))
		return true;

	if (pointInPolygon(r2, b))
		return true;

	if (pointInPolygon(r3, b))
		return true;

	return false;
}

bool Intersection::linePolygon(Vec2 const& a1, Vec2 const& a2, std::vector<Vec2> const& b)
{
	size_t size = b.size();

	for (size_t i = 0; i < size; ++i) 
	{
		Vec2 const& b1 = b[i];
		Vec2 const& b2 = b[(i + 1) % size];

		if (lineLine(a1, a2, b1, b2))
			return true;
	}

	return false;
}

bool Intersection::pointInPolygon(Vec2 const& point, std::vector<Vec2> const& polygon)
{
	bool inside = false;
	float x = point.x;
	float y = point.y;

	// use some raycasting to test hits
	// https://github.com/substack/point-in-polygon/blob/master/index.js
	size_t size = polygon.size();

	for (size_t i = 0, j = size - 1; i < size; j = i++) 
	{
		float xi = polygon[i].x, yi = polygon[i].y,
			xj = polygon[j].x, yj = polygon[j].y,
			intersect = ((yi > y) != (yj > y)) && (x < (xj - xi) * (y - yi) / (yj - yi) + xi);

		if (intersect) inside = !inside;
	}

	return inside;
}

bool Intersection::lineLine(Vec2 const& a1, Vec2 const& a2, Vec2 const& b1, Vec2 const& b2)
{
	// jshint camelcase:false

	float ua_t = (b2.x - b1.x) * (a1.y - b1.y) - (b2.y - b1.y) * (a1.x - b1.x);
	float ub_t = (a2.x - a1.x) * (a1.y - b1.y) - (a2.y - a1.y) * (a1.x - b1.x);
	float u_b = (b2.y - b1.y) * (a2.x - a1.x) - (b2.x - b1.x) * (a2.y - a1.y);

	if (u_b != 0) 
	{
		float ua = ua_t / u_b;
		float ub = ub_t / u_b;

		if (0 <= ua && ua <= 1 && 0 <= ub && ub <= 1) 
		{
			return true;
		}
	}

	return false;
}


NS_END