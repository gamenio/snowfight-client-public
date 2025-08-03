#ifndef __MOVE_SPLINE_H__
#define __MOVE_SPLINE_H__

#include "common/Common.h"

NS_BEGIN

class MoveSpline
{
public:
	MoveSpline() {}
	virtual ~MoveSpline() {}

	virtual void update(float delta) = 0;

	virtual bool isFinished() const = 0;
	virtual bool stop() = 0;

	virtual void slowSpeed(float duration) { }
	virtual void normalizeSpeed() { }

	virtual void setMoveTurnEnabled(bool isEnabled) { }
};

NS_END

#endif //__MOVE_SPLINE_H__