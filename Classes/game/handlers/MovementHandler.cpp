#include "game/client/WorldSession.h"

#include "common/utils/MathTools.h"
#include "common/utils/Timer.h"
#include "game/entities/MovementInfo.h"
#include "game/client/protocol/WorldOpcode.h"
#include "game/client/protocol/WorldOpcodeHandler.h"
#include "game/behaviors/MyCharacter.h"
#include "game/behaviors/Player.h"
#include "game/World.h"

NS_BEGIN


void WorldSession::handleMovementInfo(WorldPacket& recvPacket)
{
	MovementInfo movement;
	recvPacket.unpack(movement);

	uint16 opcode = recvPacket.getOpcode();
	WorldObject* obj = m_world->getMap()->findObject(movement.guid);
	NS_ASSERT_LOG(obj != nullptr, "WorldObject is not in world");
	if (!obj)
		return;

	//CCLOG("WorldSession::handleMovementInfo opcode: %s guid: 0x%08X movementinfo: %s", world::getOpcodeNameForLogging(recvPacket.getOpcode()).c_str(), movement.guid, movement.description().c_str());

	if (obj->getTypeID() == TYPEID_MYCHARACTER)
	{
		if(opcode == world::MSG_MOVE_HEARTBEAT)
			obj->asMyCharacter()->getMoveSpline()->ackHeartbeat();
	}
	else // Player|Robot
	{
		Unit* unit = obj->asUnit();
		unit->getData()->setMovementFlags(movement.flags);

		if (unit->isAlive())
		{
			UnitMoveSpline* moveSpline = static_cast<UnitMoveSpline*>(unit->getMoveSpline());
			switch (opcode)
			{
			case world::MSG_MOVE_START:
				unit->addUnitState(UNIT_STATE_MOVING);
				moveSpline->setCleanupMoveFlags(false);
				break;
			case world::MSG_MOVE_STOP:
				unit->clearUnitState(UNIT_STATE_MOVING);
				moveSpline->setCleanupMoveFlags(true);
				break;
			case world::MSG_MOVE_TURN: // Player
			case world::MSG_MOVE_SYNC: // Robot|Player
				break;
			default:
				break;
			}

			if (unit->getData()->hasMovementFlag(MOVEMENT_FLAG_WALKING) || opcode == world::MSG_MOVE_STOP)
			{
				moveSpline->moveBy(movement);
			}
			else if(moveSpline->isFinished())
			{
				NS_ASSERT(unit->getData()->getPosition() == movement.position);
			}
		}
		else
		{
			unit->updatePosition(movement.position);
		}
	}
}

NS_END