#include "UnitStaminaUpdater.h"

#include "game/behaviors/Unit.h"
#include "game/behaviors/Projectile.h"
#include "game/client/protocol/WorldOpcodeHandler.h"


NS_BEGIN

UnitStaminaUpdater::UnitStaminaUpdater(Unit* owner):
	m_owner(owner),
	m_isRestoreProjectileScale(false),
	m_startStamina(0),
	m_diffStamina(0)
{
}

UnitStaminaUpdater::~UnitStaminaUpdater()
{
	m_owner = nullptr;
}

void UnitStaminaUpdater::update(float delta)
{
	if (m_diffStamina == 0)
		return;

	m_staminaTimer.update(delta);
	float scale = std::min(1.0f, m_staminaTimer.getRemainder() / m_staminaTimer.getDuration());

	int32 newStamina = m_startStamina + (int32)std::ceil(m_diffStamina * (1.0f - scale));
	m_owner->getData()->setChargedStamina(newStamina);
	m_owner->getData()->setStamina(m_owner->getData()->getChargeStartStamina() - newStamina);

	float projScale = this->calcProjectileScale();
	m_owner->getData()->setProjectileScale(projScale);

	if (m_staminaTimer.passed())
	{
		m_startStamina = 0;
		m_diffStamina = 0;
		m_staminaTimer.setDuration(0);

		if (m_isRestoreProjectileScale)
			m_owner->getData()->setProjectileScale(1.0f);
	}	

	//CCLOG("UnitStaminaUpdater::update stamina: %d chargedStamina: %d projectileScale: %f", m_owner->getData()->getStamina(), newStamina, m_owner->getData()->getProjectileScale());
}

void UnitStaminaUpdater::stop()
{
	this->stopChargeProgress();
	m_isRestoreProjectileScale = false;
	m_owner->getData()->setProjectileScale(1.0f);
}

void UnitStaminaUpdater::chargeBy(StaminaInfo const& info)
{
	float delay = time_util::toGameTimeSeconds(std::max(0, time_util::getUptimeMillis() - info.time));
	int32 diff = info.chargedStamina - m_owner->getData()->getChargedStamina();
	float duration = diff / (float)info.chargeConsumesStamina - delay;
	if (duration > 0 && (info.flags & STAMINA_FLAG_ATTACK) == 0)
	{
		//float remainingTime = m_staminaTimer.getRemainder();
		m_startStamina = m_owner->getData()->getChargedStamina();
		m_diffStamina = diff;
		m_staminaTimer.setDuration(duration);

		//CCLOG("%s UnitStaminaUpdater::chargeBy startStamina: %d diffStamina: %d duration: %f delay: %f lastRemainingTime: %f", time_util::getNowTimeStr().c_str(), m_startStamina, m_diffStamina, m_staminaTimer.getDuration(), delay, remainingTime);
	}
	else
	{
		this->stopChargeProgress();

		m_owner->getData()->setStamina(info.stamina);
		m_owner->getData()->setChargedStamina(info.chargedStamina);
		float projScale = 1.0f;
		if (!m_isRestoreProjectileScale && (info.flags & STAMINA_FLAG_ATTACK) == 0)
			projScale = this->calcProjectileScale();
		m_owner->getData()->setProjectileScale(projScale);

		//CCLOG("UnitStaminaUpdater::chargeBy stamina: %d chargedStamina: %d projectileScale: %f", info.stamina, info.chargedStamina, projScale);
	}
}

void UnitStaminaUpdater::stopChargeProgress()
{
	m_diffStamina = 0;
	m_startStamina = 0;
	m_staminaTimer.reset();

	//CCLOG("%s UnitStaminaUpdater::stopChargeProgress()", time_util::getNowTimeStr().c_str());
}

float UnitStaminaUpdater::calcProjectileScale()
{
	float scale = 1.0f;
	int32 chargedStamina = m_owner->getData()->getChargedStamina();
	if (chargedStamina > m_owner->getData()->getAttackTakesStamina())
	{
		scale = 1.0f + (chargedStamina - m_owner->getData()->getAttackTakesStamina()) * PROJECTILE_SCALE_TO_STAMINA_RATIO;
	}

	return scale;
}

NS_END
