#include "DataUnit.h"

#include "common/utils/MathTools.h"
#include "updates/ObjectUpdateFields.h"


NS_BEGIN

DataUnit::DataUnit():
	m_direction(RIGHT),
	m_name(""),
	m_displayId(0),
	m_health(0),
	m_maxHealth(0),
	m_isAlive(false),
	m_moveSpeed(0),
	m_attackRange(0),
	m_attackTakesStamina(0),
	m_projectileScale(1.0f),
	m_level(0),
	m_experience(0),
	m_nextLevelXP(0),
	m_launchCenter(Point::ZERO),
	m_launchRadiusInMap(0),
	m_unitFlags(0),
	m_smiley(0),
	m_concealmentState(CONCEALMENT_STATE_EXPOSED),
	m_pickupDuration(0),
	m_magicBeanCount(0),
	m_killCount(0)
{
	m_type |= DataTypeMask::DATA_TYPEMASK_UNIT;
	m_typeId = DataTypeID::DATA_TYPEID_UNIT;
}


DataUnit::~DataUnit()
{
}

void DataUnit::clearFields()
{
	m_direction = RIGHT;
	m_movementInfo.Clear();
	m_staminaInfo.Clear();
	m_displayId = 0;
	m_health = 0;
	m_maxHealth = 0;
	m_isAlive = false;
	m_moveSpeed = 0;
	m_attackRange = 0;
	m_attackTakesStamina = 0;
	m_level = 0;
	m_experience = 0;
	m_nextLevelXP = 0;
	m_unitFlags = 0;
	m_smiley = 0;
	m_concealmentState = CONCEALMENT_STATE_EXPOSED;
	m_pickupDuration = 0;
	m_magicBeanCount = 0;
	m_killCount = 0;

	DataWorldObject::clearFields();
}

bool DataUnit::readFields(UpdateType updateType, uint32 updateFlags, FieldUpdateMask const& updateMask, DataInputStream* input)
{
	CHECK_READ(DataWorldObject::readFields(updateType, updateFlags, updateMask, input));

	if (Updatable::isNeedReadField(updateMask, SUNIT_FIELD_HEALTH))
	{
		CHECK_READ((Parcel::readInt32(input, &m_health)));
		this->setUpdatedField(CUNIT_FIELD_HEALTH);

		this->setAlive(m_health > 0);
		this->clearUnitFlag(UNIT_FLAG_DAMAGED);
	}

	if (Updatable::isNeedReadField(updateMask, SUNIT_FIELD_MAX_HEALTH))
	{
		CHECK_READ((Parcel::readInt32(input, &m_maxHealth)));
		this->setUpdatedField(CUNIT_FIELD_MAX_HEALTH);
	}

	if (Updatable::isNeedReadField(updateMask, SUNIT_FIELD_MOVEMENT_INFO)) 
	{
		m_movementInfo.Clear();
		CHECK_READ(m_movementInfo.readFromStream(input));
		this->setUpdatedField(CUNIT_FIELD_POSITION);
		this->setUpdatedField(CUNIT_FIELD_MOVEMENT_FLAG);

		this->expectFacingToAngle(m_movementInfo.orientation);

		m_moveSegment.Clear();
		if ((m_movementInfo.flags & MOVEMENT_FLAG_WALKING) != 0)
			m_moveSegment.readFromStream(input);
	}

	if (Updatable::isNeedReadField(updateMask, SUNIT_FIELD_STAMINA_INFO))
	{
		m_staminaInfo.Clear();
		CHECK_READ(m_staminaInfo.readFromStream(input));
		this->setUpdatedField(CUNIT_FIELD_STAMINA);
	}

	if (Updatable::isNeedReadField(updateMask, SUNIT_FIELD_MOVE_SPEED))
	{
		CHECK_READ((Parcel::readInt32(input, &m_moveSpeed)));
		this->setUpdatedField(CUNIT_FIELD_MOVE_SPEED);
	}

	if (Updatable::isNeedReadField(updateMask, SUNIT_FIELD_ATTACK_RANGE))
	{
		CHECK_READ((Parcel::readFloat(input, &m_attackRange)));
		this->setUpdatedField(CUNIT_FIELD_ATTACK_RANGE);
	}

	if (Updatable::isNeedReadField(updateMask, SUNIT_FIELD_ATTACK_TAKES_STAMINA))
	{
		CHECK_READ((Parcel::readInt32(input, &m_attackTakesStamina)));
	}

	if (Updatable::isNeedReadField(updateMask, SUNIT_FIELD_DISPLAYID))
	{
		CHECK_READ((Parcel::readUInt32(input, &m_displayId)));
	}

	if (Updatable::isNeedReadField(updateMask, SUNIT_FIELD_LEVEL))
	{
		int32 level;
		CHECK_READ((Parcel::readInt32(input, &level)));
		m_level = static_cast<uint8>(level);
		this->setUpdatedField(CUNIT_FIELD_LEVEL);
	}

	if (Updatable::isNeedReadField(updateMask, SUNIT_FIELD_UNIT_FLAGS))
	{
		CHECK_READ((Parcel::readUInt32(input, &m_unitFlags)));
	}

	if (Updatable::isNeedReadField(updateMask, SUNIT_FIELD_SMILEY))
	{
		int32 smiley;
		CHECK_READ(Parcel::readInt32(input, &smiley));
		m_smiley = static_cast<uint16>(smiley);
		this->setUpdatedField(CUNIT_FIELD_SMILEY);
	}

	if (Updatable::isNeedReadField(updateMask, SUNIT_FIELD_MAGIC_BEAN_COUNT))
	{
		CHECK_READ(Parcel::readInt32(input, &m_magicBeanCount));
		this->setUpdatedField(CUNIT_FIELD_MAGIC_BEAN_COUNT);
	}

	return true;
}

void DataUnit::setPosition(Point const& position)
{
	if(m_movementInfo.position != position)
	{ 
		m_movementInfo.position = position; 
		this->setUpdatedField(CUNIT_FIELD_POSITION);
	}
}

void DataUnit::setAlive(bool isAlive)
{
	if (m_isAlive != isAlive)
	{
		m_isAlive = isAlive;
		this->setUpdatedField(CUNIT_FIELD_IS_ALIVE);
	}
}

void DataUnit::setStamina(int32 stamina)
{
	if (m_staminaInfo.stamina != stamina)
	{
		m_staminaInfo.stamina = stamina;
		this->setUpdatedField(CUNIT_FIELD_STAMINA);
	}
}

void DataUnit::setStaminaInfo(StaminaInfo const& stamina)
{
	NS_ASSERT(this->getGuid() == stamina.guid);

	this->setStamina(stamina.stamina);
	this->setMaxStamina(stamina.maxStamina);
	this->setStaminaRegenRate(stamina.staminaRegenRate);
	m_staminaInfo = stamina;
}

void DataUnit::setProjectileScale(float scale)
{
	if (m_projectileScale != scale)
	{
		m_projectileScale = scale;
		this->setUpdatedField(CUNIT_FIELD_PROJECTILE_SCALE);
	}
}

void DataUnit::setMovementInfo(MovementInfo const& movement)
{
	NS_ASSERT(this->getGuid() == movement.guid);

	this->setPosition(movement.position);
	this->expectFacingToAngle(movement.orientation);
	this->setMovementFlags(movement.flags);
	m_movementInfo = movement;
}

uint8 DataUnit::getDirectionWithAngle(float rad)
{
	float deg = MathTools::radians2Degrees(rad);
	if (deg < 0.f)
		deg += 360.f;

	const static float SWING_DEG = 22.5f;
	uint8 dir;

	if (MathTools::isDegreesInRange(deg, 360, SWING_DEG))
		dir = RIGHT;
	else if (MathTools::isDegreesInRange(deg, 90, SWING_DEG))
		dir = UP;
	else if (MathTools::isDegreesInRange(deg, 180, SWING_DEG))
		dir = LEFT;
	else if (MathTools::isDegreesInRange(deg, 270, SWING_DEG))
		dir = DOWN;
	else if (MathTools::isDegreesInRange(deg, 45, SWING_DEG))
		dir = RIGHT_UP;
	else if (MathTools::isDegreesInRange(deg, 135, SWING_DEG))
		dir = LEFT_UP;
	else if (MathTools::isDegreesInRange(deg, 225, SWING_DEG))
		dir = LEFT_DOWN;
	else // center=315
		dir = RIGHT_DOWN;

	return dir;
}

bool DataUnit::expectFacingToAngle(float rad)
{
	m_movementInfo.orientation = rad;
	uint8 dir = DataUnit::getDirectionWithAngle(rad);

	if (m_direction != dir)
	{
		m_direction = dir;
		this->setUpdatedField(CUNIT_FIELD_DIRECTION);
		return true;
	}

	return false;
}

void DataUnit::setName(std::string const& name)
{
	if (m_name != name)
	{
		m_name = name;
		this->setUpdatedField(CUNIT_FIELD_NAME);
	}
}

void DataUnit::addMovementFlag(uint32 flag)
{
	if (!this->hasMovementFlag(flag))
	{
		m_movementInfo.flags |= flag;
		this->setUpdatedField(CUNIT_FIELD_MOVEMENT_FLAG);
	}

}

void DataUnit::clearMovementFlag(uint32 flag)
{
	if (hasMovementFlag(flag))
	{
		m_movementInfo.flags &= ~flag;
		this->setUpdatedField(CUNIT_FIELD_MOVEMENT_FLAG);
	}

}

void DataUnit::setMovementFlags(uint32 flags)
{
	if (m_movementInfo.flags != flags)
	{
		m_movementInfo.flags = flags;
		this->setUpdatedField(CUNIT_FIELD_MOVEMENT_FLAG);
	}
}

void DataUnit::clearUnitFlag(uint32 flag)
{
	if (this->hasUnitFlag(flag))
	{
		m_unitFlags &= ~flag;
	}
}

NS_END
