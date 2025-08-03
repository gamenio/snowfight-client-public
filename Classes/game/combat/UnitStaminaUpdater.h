#ifndef __UNIT_STAMINA_UPDATER_H__
#define __UNIT_STAMINA_UPDATER_H__

#include "common/Common.h"
#include "common/utils/Timer.h"
#include "game/entities/StaminaInfo.h"
#include "StaminaUpdater.h"

NS_BEGIN

class Unit;

class UnitStaminaUpdater : public StaminaUpdater
{
public:
	UnitStaminaUpdater(Unit* owner);
	~UnitStaminaUpdater();

	void update(float delta) override;
	void stop() override;
	void setRestoreProjectileScale(bool isRestoreScale) { m_isRestoreProjectileScale = isRestoreScale; }

	void chargeBy(StaminaInfo const& info);

private:
	void stopChargeProgress();
	float calcProjectileScale();

	Unit* m_owner;

	bool m_isRestoreProjectileScale;
	int32 m_startStamina;
	int32 m_diffStamina;
	DelayTimer m_staminaTimer;
};

NS_END

#endif // __UNIT_STAMINA_UPDATER_H__