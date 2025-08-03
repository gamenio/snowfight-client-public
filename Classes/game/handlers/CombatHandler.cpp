#include "game/client/WorldSession.h"

#include "game/client/protocol/pb/LaunchResult.pb.h"

#include "game/utils/UnitHelper.h"
#include "game/client/protocol/WorldOpcodeHandler.h"
#include "game/behaviors/MyCharacter.h"
#include "game/behaviors/Projectile.h"
#include "game/combat/UnitStaminaUpdater.h"
#include "game/World.h"

NS_BEGIN

void WorldSession::handleLaunchResult(WorldPacket& recvPacket)
{
	LaunchResult result;
	recvPacket.unpack(result);

	MyCharacter* myChar = this->getMyCharacter();
	NS_ASSERT(myChar != nullptr);
	//CCLOG("handleLaunchResult launcher: 0x%08X projectile: 0x%08X, target: 0x%08X, status: %d", result.launcher(), result.projectile(), result.target(), result.status());
	LaunchStatus status = static_cast<LaunchStatus>(result.status());

	if (myChar->getData()->getGuid().getRawValue() == result.launcher())
	{
		if (status == LAUNCHSTATUS_FAILED)
		{
			if (result.attack_info_counter() == myChar->getAttackInfoCounter())
			{
				NS_ASSERT(myChar->getData()->hasMovementFlag(MOVEMENT_FLAG_HANDUP));
				if (myChar->canHandDown())
					myChar->startHandDownTimer();
			}
			return;
		}

	}

	WorldObject* obj = m_world->getMap()->findObject(ObjectGuid(result.projectile()));
	if (!obj)
		return;

	m_world->notifyLaunchResult(result);
}

void WorldSession::handleStaminaInfo(WorldPacket& recvPacket)
{
	StaminaInfo info;
	recvPacket.unpack(info);

	uint16 opcode = recvPacket.getOpcode();
	WorldObject* obj = m_world->getMap()->findObject(ObjectGuid(info.guid));
	NS_ASSERT_LOG(obj != nullptr, "WorldObject is not in world");
	if (!obj)
		return;

	//CCLOG("WorldSession::handleStaminaInfo opcode: %s %s", world::getOpcodeNameForLogging(recvPacket.getOpcode()).c_str(), info.description().c_str());

	if (obj->getTypeID() == TYPEID_MYCHARACTER)
	{
		MyCharacter* myChar = obj->asMyCharacter();
		MyStaminaUpdater* staminaUpdater = myChar->getStaminaUpdater();
		staminaUpdater->ackStamina(opcode, info);
		switch (opcode)
		{
		case world::MSG_CHARGE_START:
			staminaUpdater->chargeStart(info);
			break;
		case world::MSG_CHARGE_STOP:
			if (info.attackInfoCounter == myChar->getAttackInfoCounter()
				&& myChar->canHandDown()
				&& !myChar->isHandDownTimerEnabled())
			{
				myChar->getData()->clearMovementFlag(MOVEMENT_FLAG_HANDUP);
				myChar->sendMoveSync();
			}
			break;
		default:
			break;
		}
	}
	else // Player|Robot
	{
		Unit* unit = obj->asUnit();
		switch (opcode)
		{
		case world::MSG_CHARGE_START:
			unit->getData()->setStamina(info.stamina);
			unit->getData()->setChargeStartStamina(info.chargeStartStamina);
			unit->getData()->setChargedStamina(info.chargedStamina);
			break;
		default: // MSG_CHARGE_STOP|MSG_STAMINA_SYNC
		{
			UnitStaminaUpdater* staminaUpdater = static_cast<UnitStaminaUpdater*>(unit->getStaminaUpdater());
			bool isRestoreScale = opcode == world::MSG_CHARGE_STOP;
			staminaUpdater->setRestoreProjectileScale(isRestoreScale);
			staminaUpdater->chargeBy(info);
			break;
		}
		}
	}
}

NS_END
