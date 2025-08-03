#include "Player.h"

#include "game/behaviors/ObjectShapes.h"
#include "game/combat/UnitStaminaUpdater.h"

NS_BEGIN

Player::Player()
{
	m_type |= TYPEMASK_PLAYER;
	m_typeId = TypeID::TYPEID_PLAYER;
}


Player::~Player()
{
}


void Player::stopMoving()
{
	if (this->getMoveSpline())
		this->getMoveSpline()->stop(!this->isAlive());

	this->clearUnitState(UNIT_STATE_MOVING);
}

DataBasic* Player::loadData(ObjectGuid const& guid)
{
	DataPlayer* data = new DataPlayer();
	data->autorelease();
	data->setGuid(guid);

    data->setObjectSize(UNIT_OBJECT_SIZE);
    data->setAnchorPoint(UNIT_ANCHOR_POINT);
	data->setObjectRadiusInMap(UNIT_OBJECT_RADIUS_IN_MAP);
    data->setLaunchCenter(UNIT_LAUNCH_CENTER);
    data->setLaunchRadiusInMap(UNIT_LAUNCH_RADIUS_IN_MAP);
    
    this->setData(data);

	this->setMoveSpline(new UnitMoveSpline(this));
	this->setStaminaUpdater(new UnitStaminaUpdater(this));

	return data;
}

void Player::updateObject(UpdateType updateType, uint32 updateFlags, DataBasic* data)
{
	Unit::updateObject(updateType, updateFlags, data);

	DataPlayer* dPlayer = data->asDataPlayer();

	if (updateType == UPDATE_TYPE_CREATE)
	{
		if (dPlayer->hasMovementFlag(MOVEMENT_FLAG_WALKING))
		{
			MovementInfo movement = dPlayer->getMovementInfo();
			MovementInfo const& segment = dPlayer->getMoveSegment();
			this->updatePosition(segment.position);

			this->addUnitState(UNIT_STATE_MOVING);
			NS_ASSERT(!this->getMoveSpline()->isCleanupMoveFlags());
			this->getMoveSpline()->moveBy(movement);
		}

		if (dPlayer->hasStaminaFlag(STAMINA_FLAG_CHARGING))
		{
			StaminaInfo stamina = dPlayer->getStaminaInfo();
			this->getStaminaUpdater()->chargeBy(stamina);
		}

		this->getMap()->activateObject(this);
	}
	else if (updateType == UPDATE_TYPE_VALUES)
	{
	}

}

NS_END
