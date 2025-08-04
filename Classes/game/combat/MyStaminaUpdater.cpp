#include "MyStaminaUpdater.h"

#include "game/behaviors/MyCharacter.h"
#include "game/behaviors/Projectile.h"
#include "game/client/protocol/WorldOpcodeHandler.h"


NS_BEGIN

#define STAMINA_SYNC_INTERVAL			0.2f	// Stamina synchronization interval. Unit: seconds

MyStaminaUpdater::MyStaminaUpdater(MyCharacter* owner):
	m_owner(owner),
	m_startStamina(0),
	m_diffStamina(0),
	m_chargeState(CHARGE_STATE_NONE),
	m_chargeStartTime(0),
	m_chargeCounter(0),
	m_staminaSyncCounter(0),
	m_isStaminaSyncPaused(false),
	m_syncedStamina(0)
{
	m_staminaSyncTimer.setInterval(STAMINA_SYNC_INTERVAL);
}

MyStaminaUpdater::~MyStaminaUpdater()
{
	m_owner = nullptr;
}

void MyStaminaUpdater::update(float delta)
{
	if (m_diffStamina == 0)
		return;

	if (!m_isStaminaSyncPaused)
	{
		float diff = m_staminaSyncTimer.getInterval() - m_staminaSyncTimer.getCurrent();
		float updateDt = std::min(std::max(0.f, diff), delta);
		this->updateStamina(updateDt);

		// Scheduled synchronization stamina
		m_staminaSyncTimer.update(delta);
		if (m_staminaSyncTimer.passed())
		{
			//CCLOG("staminaSyncCounter:%d", m_staminaSyncCounter);
			// If the stamina synchronization package is not acknowledged by the server, charging or stamina regeneration will be paused.
			if (m_staminaSyncCounter > 0)
			{
				m_isStaminaSyncPaused = true;
				//CCLOG("MyStaminaUpdater::syncStamina paused: %d", m_staminaSyncPaused);
			}
			else
				this->sendSyncStamina();

			m_staminaSyncTimer.reset();
		}
	}
}

void MyStaminaUpdater::stop()
{
	m_owner->getData()->setProjectileScale(1.0f);
	m_chargeCounter = 0;

	this->stopRegenStamina();
	this->stopChargeProgress();
	this->resetStaminaSyncState();
}

void MyStaminaUpdater::charge()
{
	if (m_chargeState != CHARGE_STATE_NONE)
		return;

	//CCLOG("MyStaminaUpdater::charge chargeState: %d", m_chargeState);

	this->stopRegenStamina();
	++m_chargeCounter;
	m_chargeState = CHARGE_STATE_PENDING;

	m_staminaSyncTimer.reset();
	this->sendChargeStart();
}

void MyStaminaUpdater::chargeStart(StaminaInfo const& info)
{
	if (m_chargeState != CHARGE_STATE_PENDING)
		return;

	if (m_chargeCounter != info.chargeCounter)
		return;

	NS_ASSERT(m_diffStamina == 0);

	m_startStamina = m_owner->getData()->getStamina();
	NS_ASSERT(m_startStamina == info.chargeStartStamina);
	m_diffStamina = -m_startStamina;
	float duration = m_startStamina / (float)m_owner->getData()->getChargeConsumesStamina();
	duration = std::max(0.f, duration);
	m_staminaTimer.setDuration(duration);

	m_chargeStartTime = time_util::getUptimeMillis();
	m_chargeState = CHARGE_STATE_CHARGING;
	m_owner->getData()->addStaminaFlag(STAMINA_FLAG_CHARGING);

	//CCLOG("%s MyStaminaUpdater::chargeStart startStamina: %d diffStamina: %d duration: %f", time_util::getNowTimeStr().c_str(), m_startStamina, m_diffStamina, m_staminaTimer.getDuration());
}

void MyStaminaUpdater::chargeUpdate(StaminaInfo const& info)
{
	if (m_chargeState != CHARGE_STATE_CHARGING)
		return;

	m_startStamina = info.chargeStartStamina;
	m_diffStamina = -m_startStamina;
	float duration = m_startStamina / (float)m_owner->getData()->getChargeConsumesStamina();
	duration = std::max(0.f, duration);
	m_staminaTimer.setDuration(duration);
	if (info.chargedStamina > 0)
	{
		float elapsed = info.chargedStamina / (float)m_owner->getData()->getChargeConsumesStamina();
		m_staminaTimer.update(elapsed);
	}

	//CCLOG("%s MyStaminaUpdater::chargeUpdate startStamina: %d diffStamina: %d duration: %f extraTime: %f", time_util::getNowTimeStr().c_str(), m_startStamina, m_diffStamina, m_staminaTimer.getDuration());
}

void MyStaminaUpdater::chargeStop()
{
	if (m_chargeState == CHARGE_STATE_NONE)
		return;

	m_owner->getData()->setProjectileScale(1.0f);

	m_staminaSyncTimer.reset();
	m_owner->getData()->clearStaminaFlag(STAMINA_FLAG_CHARGING);
	this->sendChargeStop();

	this->startRegenStamina();
}

bool MyStaminaUpdater::canAttack() const
{
	if (m_chargeState == CHARGE_STATE_NONE)
	{
		if (m_owner->getData()->getStamina() < m_owner->getData()->getAttackTakesStamina())
			return false;
	}

	return true;
}

void MyStaminaUpdater::prepareForChargedAttack()
{
	if (m_chargeState == CHARGE_STATE_NONE)
		return;

	m_owner->getData()->clearStaminaFlag(STAMINA_FLAG_CHARGING);

	this->sendSyncStamina(STAMINA_FLAG_ATTACK);
	m_staminaSyncTimer.reset();

	this->startRegenStamina();
}

void MyStaminaUpdater::consumeStamina(int32 points, uint32 attackCounter)
{
	NS_ASSERT(attackCounter > m_owner->getData()->getAttackCounter());

	m_owner->getData()->setAttackCounter(attackCounter);
	m_owner->getData()->increaseConsumedStaminaTotal(points);

	int32 stamina = m_owner->getData()->getStamina();
	int32 newStamina = stamina - points;
	NS_ASSERT(newStamina >= 0);
	newStamina = std::max(newStamina, 0);
	m_owner->getData()->setStamina(newStamina);
	m_syncedStamina = newStamina;
	//CCLOG("MyStaminaUpdater::consumeStamina guid: 0x%08X stamina: %d consumedPoints: %d attackCounter: %d", m_owner->getData()->getGuid().getRawValue(), stamina, points, attackCounter);

	if (m_chargeState == CHARGE_STATE_NONE)
		this->startRegenStamina();
}

void MyStaminaUpdater::ackStamina(uint16 opcode, StaminaInfo const& info)
{
	if (m_staminaSyncCounter > 0)
		--m_staminaSyncCounter;

	//CCLOG("%s STAMINA ACK guid: 0x%08X opcode: %s syncCounter: %d paused: %d stamina: %d attackCounter: %d/%d consumedStaminaTotal: %d flags: 0x%08X chrageState: %d", time_util::getNowTimeStr().c_str(), m_owner->getData()->getGuid().getRawValue(), world::getOpcodeNameForLogging(opcode).c_str(), m_staminaSyncCounter, m_isStaminaSyncPaused, info.stamina, info.attackCounter, m_owner->getData()->getAttackCounter(), info.consumedStaminaTotal, info.flags, m_chargeState);
	NS_ASSERT(m_owner->getData()->getAttackCounter() == info.attackCounter);
	m_owner->getData()->setConsumedStaminaTotal(info.consumedStaminaTotal);
	m_owner->getData()->setStaminaFlags(info.flags);

	if(m_owner->getData()->hasStaminaFlag(STAMINA_FLAG_ATTACK))
		m_owner->getData()->setProjectileScale(1.0f);
	m_owner->getData()->clearStaminaFlag(STAMINA_FLAG_ATTACK);

	if (m_isStaminaSyncPaused)
	{
		if (m_staminaSyncCounter <= 0)
		{
			m_isStaminaSyncPaused = false;
			this->sendSyncStamina();
		}
	}
}

void MyStaminaUpdater::startRegenStamina()
{
	this->stopChargeProgress();
	this->stopRegenStamina();

	int32 stamina = m_owner->getData()->getStamina();
	int32 diff = m_owner->getData()->getMaxStamina() - stamina;
	if (diff > 0)
	{
		float duration = diff / (m_owner->getData()->getStaminaRegenRate() * m_owner->getData()->getMaxStamina());
		duration = std::max(0.f, duration);

		m_staminaTimer.setDuration(duration);
		m_diffStamina = diff;
		m_startStamina = stamina;

		//CCLOG("%s MyStaminaUpdater::startRegenStamina currStamina: %d startStamina: %d diffStamina: %d duration: %f", time_util::getNowTimeStr().c_str(), stamina, m_startStamina, m_diffStamina, m_staminaTimer.getDuration());
	}
}

void MyStaminaUpdater::resetStaminaSyncState()
{
	m_staminaSyncTimer.reset();
	m_staminaSyncCounter = 0;
	m_syncedStamina = m_owner->getData()->getStamina();
	m_isStaminaSyncPaused = false;
}

void MyStaminaUpdater::stopRegenStamina()
{
	if (m_diffStamina <= 0)
		return;

	NS_ASSERT(m_chargeState == CHARGE_STATE_NONE);

	m_diffStamina = 0;
	m_startStamina = 0;
	m_staminaTimer.reset();

	//CCLOG("MyStaminaUpdater::stopRegenStamina()");
}

void MyStaminaUpdater::updateStamina(float delta)
{
	m_staminaTimer.update(delta);
	float scale = std::min(1.0f, m_staminaTimer.getRemainder() / m_staminaTimer.getDuration());

	int32 newStamina = (int32)(m_startStamina + m_diffStamina * (1.0f - scale));
	newStamina = this->adjustStaminaForSync(newStamina);
	m_owner->getData()->setStamina(newStamina);

	if (m_chargeState == CHARGE_STATE_CHARGING)
	{
		int32 chargedStamina = m_startStamina - newStamina;
		if (chargedStamina > m_owner->getData()->getAttackTakesStamina())
		{
			float projScale = 1.0f + (chargedStamina - m_owner->getData()->getAttackTakesStamina()) * PROJECTILE_SCALE_TO_STAMINA_RATIO;
			// Take the maximum scale to solve the problem of the scale decreasing after modifying stamina
			projScale = std::max(m_owner->getData()->getProjectileScale(), projScale); 
			m_owner->getData()->setProjectileScale(projScale);
		}
		//CCLOG("MyStaminaUpdater::updateStamina chargedStamina: %d projectileScale: %f", chargedStamina, m_owner->getData()->getProjectileScale());
	}

	if (m_staminaTimer.passed() && (newStamina == 0 || newStamina == m_owner->getData()->getMaxStamina()))
	{
		if (m_chargeState == CHARGE_STATE_CHARGING)
		{
			m_chargeState = CHARGE_STATE_FULLY;
			//CCLOG("%s stamina: %d fully charged!", time_util::getNowTimeStr().c_str(), newStamina);
		}
		//else
		//	CCLOG("%s stamina: %d is full!", time_util::getNowTimeStr().c_str(), newStamina);

		m_startStamina = 0;
		m_diffStamina = 0;
		m_staminaTimer.reset();
		m_staminaSyncTimer.setPassed();
	}
	//else
	//	CCLOG("%s stamina: %d", time_util::getNowTimeStr().c_str(), newStamina);
}

void MyStaminaUpdater::sendSyncStamina(uint32 flags)
{
	if (!m_owner->getSession())
		return;

	StaminaInfo info;
	m_owner->buildStaminaInfo(info);
	info.flags |= flags;

	WorldPacket packet(world::MSG_STAMINA_SYNC, std::move(info));
	m_owner->getSession()->sendPacket(std::move(packet));

	++m_staminaSyncCounter;

	//CCLOG("%s MSG_STAMINA_SYNC paused: %d syncCounter: %d %s", time_util::getNowTimeStr().c_str(), m_isStaminaSyncPaused, m_staminaSyncCounter, info.description().c_str());

	//int32 syncedPoints = std::abs(m_owner->getData()->getStamina() - m_syncedStamina);
	//CCLOG("%s MSG_STAMINA_SYNC syncCounter: %d stamina: %d->%d syncedPoints: %d", time_util::getNowTimeStr().c_str(), m_staminaSyncCounter, m_syncedStamina, m_owner->getData()->getStamina(), syncedPoints);
	m_syncedStamina = m_owner->getData()->getStamina();
}

int32 MyStaminaUpdater::adjustStaminaForSync(int32 newStamina)
{
	if (m_chargeState == CHARGE_STATE_CHARGING)
	{
		int32 maxPoints = (int32)std::ceil(STAMINA_SYNC_INTERVAL * m_owner->getData()->getChargeConsumesStamina());
		int32 diff = std::abs(m_syncedStamina - newStamina);
		if (diff > maxPoints)
		{
			newStamina += (diff - maxPoints);
			//CCLOG("MyStaminaUpdater: Stamina points consumed is out of synchronization range. (%d > %d)", diff, maxPoints);
		}
	}
	else
	{
		int32 maxPoints = (int32)std::ceil(STAMINA_SYNC_INTERVAL * (m_owner->getData()->getStaminaRegenRate() * m_owner->getData()->getMaxStamina()));
		int32 diff = std::abs(m_syncedStamina - newStamina);
		if (diff > maxPoints)
		{
			newStamina -= (diff - maxPoints);
			//CCLOG("MyStaminaUpdater: Stamina points regenerated is out of synchronization range. (%d > %d)", diff, maxPoints);
		}
	}

	return newStamina;
}

void MyStaminaUpdater::stopChargeProgress()
{
	if (m_chargeState == CHARGE_STATE_NONE)
		return;

	NS_ASSERT(m_diffStamina <= 0);

	m_chargeStartTime = 0;
	m_chargeState = CHARGE_STATE_NONE;

	m_diffStamina = 0;
	m_startStamina = 0;
	m_staminaTimer.reset();
	m_owner->getData()->clearStaminaFlag(STAMINA_FLAG_CHARGING);

	//CCLOG("MyStaminaUpdater::stopChargeProgress()");
}

void MyStaminaUpdater::sendChargeStart()
{
	if (!m_owner->getSession())
		return;

	StaminaInfo info;
	info.chargeCounter = m_chargeCounter;
	m_owner->buildStaminaInfo(info);

	WorldPacket packet(world::MSG_CHARGE_START, std::move(info));
	m_owner->getSession()->sendPacket(std::move(packet));

	++m_staminaSyncCounter;
	m_syncedStamina = m_owner->getData()->getStamina();
	//CCLOG("%s MSG_CHARGE_START paused: %d syncCounter: %d %s", time_util::getNowTimeStr().c_str(), m_isStaminaSyncPaused, m_staminaSyncCounter, info.description().c_str());
}

void MyStaminaUpdater::sendChargeStop()
{
	if (!m_owner->getSession())
		return;

	StaminaInfo info;
	m_owner->buildStaminaInfo(info);

	WorldPacket packet(world::MSG_CHARGE_STOP, std::move(info));
	m_owner->getSession()->sendPacket(std::move(packet));

	++m_staminaSyncCounter;
	m_syncedStamina = m_owner->getData()->getStamina();
	//CCLOG("%s MSG_CHARGE_STOP paused: %d syncCounter: %d %s", time_util::getNowTimeStr().c_str(), m_isStaminaSyncPaused, m_staminaSyncCounter, info.description().c_str());
}

NS_END