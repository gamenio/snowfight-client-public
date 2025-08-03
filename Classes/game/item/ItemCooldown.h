#ifndef __ITEM_COOLDOWN_H__
#define __ITEM_COOLDOWN_H__

#include "common/Common.h"
#include "common/utils/Timer.h"
#include "game/entities/DataCarriedItem.h"

NS_BEGIN

class Unit;

class ItemCooldown
{
public:
	ItemCooldown(NSTime duration, NSTime remainingTime);
	~ItemCooldown();

	float getDuration() const { return m_finishTimer.getDuration(); }
	float getElapsed() const { return m_finishTimer.getElapsed(); }

	bool update(float delta);

private:
	void start(NSTime duration, NSTime remainingTime);

	DelayTimer m_finishTimer;
};

NS_END
 
#endif // __ITEM_COOLDOWN_H__