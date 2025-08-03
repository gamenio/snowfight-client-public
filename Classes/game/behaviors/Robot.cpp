#include "Robot.h"

#include "common/utils/TimeUtil.h"
#include "common/utils/MathTools.h"
#include "game/entities/updates/ObjectUpdateFields.h"
#include "game/behaviors/ObjectShapes.h"
#include "game/combat/UnitStaminaUpdater.h"
#include "game/World.h"
#include "game/movement/UnitMoveSpline.h"

NS_BEGIN

Robot::Robot()
{
	m_type |= TYPEMASK_ROBOT;
	m_typeId = TypeID::TYPEID_ROBOT;
}


Robot::~Robot()
{
}

DataBasic* Robot::loadData(ObjectGuid const& guid)
{
	DataRobot* data = new DataRobot();
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

void Robot::updateObject(UpdateType updateType, uint32 updateFlags, DataBasic* data)
{
	Unit::updateObject(updateType, updateFlags, data);

	DataRobot* dRobot = data->asDataRobot();

	if (updateType == UPDATE_TYPE_CREATE)
	{
		if (dRobot->hasMovementFlag(MOVEMENT_FLAG_WALKING))
		{
			this->addUnitState(UNIT_STATE_MOVING);

			MovementInfo movement = dRobot->getMoveSegment();
			this->getMoveSpline()->moveBy(movement);
		}

		if (dRobot->hasStaminaFlag(STAMINA_FLAG_CHARGING))
		{
			StaminaInfo stamina = dRobot->getStaminaInfo();
			this->getStaminaUpdater()->chargeBy(stamina);
		}

		this->getMap()->activateObject(this);
	}
	else if(updateType == UPDATE_TYPE_VALUES)
	{
	}

}

void Robot::stopMoving()
{
	if (this->getMoveSpline())
		this->getMoveSpline()->stop(!this->isAlive());

	this->clearUnitState(UNIT_STATE_MOVING);
}

void Robot::update(float delta)
{
	Unit::update(delta);

	if (!this->isAlive() || !this->isInWorld())
		return;
}


NS_END