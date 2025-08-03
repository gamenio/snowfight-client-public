#include "MathTools.h"

NS_BEGIN

#define Y0						0.0f

float MathTools::computeAngleInRadians(Point const& p1, Point const& p2)
{
	float dx = p2.x - p1.x;
	float dy = p2.y - p1.y;
	float rad = atan2(dy, dx);
	return rad;
}

bool MathTools::isDegreesInRange(float deg, float center, float swingDeg)
{
	bool boundary = false;

	float left = center - swingDeg;
	if (left < 0)
	{
		left = 360 + left;
		boundary = true;
	}

	float right = center + swingDeg;
	if (right > 360)
	{
		right = right - 360;
		boundary = true;
	}

	if (boundary)
		return deg > left || deg <= right;
	else
		return deg > left && deg <= right;
}

// https://www.geeksforgeeks.org/minimum-distance-from-a-point-to-the-line-segment-using-vectors/
float MathTools::minDistanceFromPointToSegment(Point const& p1, Point const& p2, Point const& p)
{
	Point const& A = p1;
	Point const& B = p2;
	Point const& E = p;

	// vector AB
	float ABx = B.x - A.x;
	float ABy = B.y - A.y;

	// vector BP
	float BEx = E.x - B.x;
	float BEy = E.y - B.y;

	// vector AP
	float AEx = E.x - A.x;
	float AEy = E.y - A.y;

	// Variables to store dot product
	float AB_BE, AB_AE;

	// Calculating the dot product
	AB_BE = (ABx * BEx + ABy * BEy);
	AB_AE = (ABx * AEx + ABy * AEy);

	// Minimum distance from
	// point E to the line segment
	float reqAns = 0;

	// Case 1
	if (AB_BE > 0) {

		// Finding the magnitude
		float y = E.y - B.y;
		float x = E.x - B.x;
		reqAns = std::sqrt(x * x + y * y);
	}

	// Case 2
	else if (AB_AE < 0) {
		float y = E.y - A.y;
		float x = E.x - A.x;
		reqAns = std::sqrt(x * x + y * y);
	}

	// Case 3
	else {

		// Finding the perpendicular distance
		float x1 = ABx;
		float y1 = ABy;
		float x2 = AEx;
		float y2 = AEy;
		float mod = std::sqrt(x1 * x1 + y1 * y1);
		if (mod > 0)
			reqAns = std::abs(x1 * y2 - y1 * x2) / mod;
	}
	return reqAns;
}

// https://www.geeksforgeeks.org/program-for-point-of-intersection-of-two-lines/
bool MathTools::findIntersectionTwoLines(Point const& l1p1, Point const& l1p2, Point const& l2p1, Point const& l2p2, Point& result)
{
	// Line 1 represented as a1x + b1y = c1
	float a1 = l1p2.y - l1p1.y;
	float b1 = l1p1.x - l1p2.x;
	float c1 = a1*(l1p1.x) + b1*(l1p1.y);

	// Line 2 represented as a2x + b2y = c2
	float a2 = l2p2.y - l2p1.y;
	float b2 = l2p1.x - l2p2.x;
	float c2 = a2*(l2p1.x) + b2*(l2p1.y);

	float determinant = a1*b2 - a2*b1;

	if (determinant == 0)
	{
		// The lines are parallel. 
		return false;
	}
	else
	{
		result.x = (b2*c1 - b1*c2) / determinant;
		result.y = (a1*c2 - a2*c1) / determinant;
		return true;
	}
}

Point MathTools::findPointAlongLine(Point const& p1, Point const& p2, float distance)
{
	float dx = p2.x - p1.x;
	float dy = p2.y - p1.y;
	float angle = std::atan2(dy, dx);
	float offX = std::cos(angle) * distance;
	float offY = std::sin(angle) * distance;

	return p1 + Point(offX, offY);
}

bool MathTools::isPointInsideCircle(Point const& center, float radius, Point const& p)
{
	float dx = p.x - center.x;
	float dy = p.y - center.y;
	if ((dx * dx + dy * dy) <= radius * radius)
		return true;
	else
		return false;
}

NS_END


