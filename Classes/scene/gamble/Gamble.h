#ifndef __GAMBLE_H__
#define __GAMBLE_H__

#include "common/Common.h"
#include "game/entities/DataBasic.h"

NS_BEGIN

// An interface class for an object that can do data updates and state synchronization in a scene.
class Gamble
{
public:
	Gamble() {}
	virtual ~Gamble() { }

	// The active state of the object
	virtual void onActivated() = 0;
	virtual void onInactivated() = 0;
	virtual bool isActive() const = 0;

	// Clears the update mask for data, typically called after an object is updated
	virtual void cleanUpdateMask() = 0;

	// Object data
	virtual DataBasic* getData() const = 0;
};

NS_END

#endif // __GAMBLE_H__

