#ifndef __TRAJECTORY_GENERATOR_H__
#define __TRAJECTORY_GENERATOR_H__

#include "common/Common.h"

USING_NS_CC;

NS_BEGIN


struct BezierCurveConfig
{
	enum CurveOrder
	{
		QUADRATIC,
		CUBIC,
	};

	BezierCurveConfig() :
		order(QUADRATIC),
		length(0)
	{
	}

	CurveOrder order;
	cocos2d::Point startPosition;
	cocos2d::Point endPosition;
	std::vector<cocos2d::Point> controlPoints;
	float length;

	void draw(DrawNode* drawNode) const;
};

enum TrajectoryType
{
	TRAJECTORY_TYPE_PROJECTILE,
	TRAJECTORY_TYPE_ITEM,
};

class TrajectoryGenerator
{
public:
	TrajectoryGenerator(TrajectoryType type, cocos2d::Point const& origin, cocos2d::Point const& destination);
	~TrajectoryGenerator();

	TrajectoryType getType() const { return m_type; }
	void compute();
	BezierCurveConfig const& getBezierCurveConfig() const { return m_config; }

private:
	void computeProjectileTrajectory();
	void computeItemTrajectory();

	TrajectoryType m_type;
	cocos2d::Point m_origin;
	cocos2d::Point m_destination;

	BezierCurveConfig m_config;
};


NS_END

#endif // __TRAJECTORY_GENERATOR_H__
