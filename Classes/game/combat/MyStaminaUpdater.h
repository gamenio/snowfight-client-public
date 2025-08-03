#ifndef __MY_STAMINA_UPDATER_H__
#define __MY_STAMINA_UPDATER_H__

#include "common/Common.h"
#include "common/utils/Timer.h"
#include "game/entities/StaminaInfo.h"
#include "StaminaUpdater.h"

NS_BEGIN

enum ChargeState
{
	CHARGE_STATE_NONE,
	CHARGE_STATE_PENDING,
	CHARGE_STATE_CHARGING,
	CHARGE_STATE_FULLY,
};

class MyCharacter;

class MyStaminaUpdater : public StaminaUpdater
{
public:
	MyStaminaUpdater(MyCharacter* owner);
	~MyStaminaUpdater();

	void update(float delta) override;
	void stop() override;

	void charge();
	void chargeStart(StaminaInfo const& info);
	void chargeUpdate(StaminaInfo const& info);
	void chargeStop();
	bool isInCharge() const { return m_chargeState != CHARGE_STATE_NONE; }
	ChargeState getChargeState() const { return m_chargeState; }
	NSTime getChargeStartTime() const { return m_chargeStartTime; }

	bool canAttack() const;
	void prepareForChargedAttack();
	void consumeStamina(int32 points, uint32 attackCounter);
	void ackStamina(uint16 opcode, StaminaInfo const& info);
	void startRegenStamina();

	void resetStaminaSyncState();
private:
	void stopRegenStamina();

	void updateStamina(float delta);
	void sendSyncStamina(uint32 flags = 0);
	int32 adjustStaminaForSync(int32 newStamina);

	void stopChargeProgress();
	void sendChargeStart();
	void sendChargeStop();

	MyCharacter* m_owner;

	int32 m_startStamina;
	int32 m_diffStamina;
	DelayTimer m_staminaTimer;

	ChargeState m_chargeState;
	NSTime m_chargeStartTime;
	uint32 m_chargeCounter;

	IntervalTimer m_staminaSyncTimer;
	int32 m_staminaSyncCounter;
	bool m_isStaminaSyncPaused;
	int32 m_syncedStamina;
};

NS_END

#endif // __MY_MOVEMENT_GENERATOR_H__