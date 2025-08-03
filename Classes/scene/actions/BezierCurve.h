#ifndef __BEZIER_CURVE_H__
#define __BEZIER_CURVE_H__

#include "cocos2d.h"

#include "common/Common.h"
#include "game/utils/TrajectoryGenerator.h"

USING_NS_CC;

NS_BEGIN

class BezierCurve : public ActionInterval
{
public:
	static BezierCurve* create(float t, BezierCurveConfig const& c);

	virtual void startWithTarget(Node *target) override;
	virtual void update(float time) override;
	virtual void step(float dt) override;

CC_CONSTRUCTOR_ACCESS:
	BezierCurve();
	~BezierCurve();

	bool initWithDuration(float t, BezierCurveConfig const& c);

protected:
	BezierCurveConfig m_config;
	Vec2 m_startPosition;
	Vec2 m_previousPosition;

private:
	CC_DISALLOW_COPY_AND_ASSIGN(BezierCurve)
};

NS_END

#endif // __BEZIER_CURVE_H__
