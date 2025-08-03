#ifndef __STAMINA_UPDATER_H__
#define __STAMINA_UPDATER_H__

#include "common/Common.h"

NS_BEGIN

class StaminaUpdater
{
public:
	StaminaUpdater() { }
	virtual ~StaminaUpdater() { }

	virtual void update(float delta) = 0;
	virtual void stop() = 0;
};

NS_END

#endif // __STAMINA_UPDATER_H__
