#include "ItemCooldown.h"

#include "common/utils/TimeUtil.h"

NS_BEGIN

ItemCooldown::ItemCooldown(NSTime duration, NSTime remainingTime)
{
	this->start(duration, remainingTime);
}

ItemCooldown::~ItemCooldown()
{
}

bool ItemCooldown::update(float delta)
{
	m_finishTimer.update(delta);
	if (m_finishTimer.passed())
	{
		m_finishTimer.reset();
		return false;
	}

	return true;
}

void ItemCooldown::start(NSTime duration, NSTime remainingTime)
{
	float durInSec = time_util::toGameTimeSeconds(duration);
	m_finishTimer.setDuration(durInSec);

	if (remainingTime > 0)
	{
		float elapsed = time_util::toGameTimeSeconds(duration - remainingTime);
		m_finishTimer.update(elapsed);
	}
}

NS_END