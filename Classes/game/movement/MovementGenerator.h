#ifndef __MOVEMENT_GENERATOR_H__
#define __MOVEMENT_GENERATOR_H__


#include "common/Common.h"

NS_BEGIN

class MovementGenerator
{
public:
	MovementGenerator() { }
	virtual ~MovementGenerator() { }

	virtual void update(float delta) = 0;
	virtual void finish() = 0;
};

NS_END

#endif // __MOVEMENT_GENERATOR_H__
