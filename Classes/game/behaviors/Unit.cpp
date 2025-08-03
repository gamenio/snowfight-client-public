#include "Unit.h"

#include "game/utils/UnitHelper.h"

NS_BEGIN

Unit::Unit() :
	m_unitState(UNIT_STATE_NONE),
	m_deathState(DEATH_STATE_ALIVE),
	m_movementGenerator(nullptr),
	m_moveSpline(nullptr),
	m_staminaUpdater(nullptr)
{
	m_type |= TypeMask::TYPEMASK_UNIT;
	m_typeId = TypeID::TYPEID_UNIT;

}

Unit::~Unit()
{
	CC_SAFE_DELETE(m_moveSpline);
	CC_SAFE_DELETE(m_movementGenerator);
	CC_SAFE_DELETE(m_staminaUpdater);
}

void Unit::update(float delta)
{
	WorldObject::update(delta);

	if (!this->isAlive() || !this->isInWorld())
		return;

	if (m_moveSpline)
		m_moveSpline->update(delta);

	if (m_movementGenerator)
		m_movementGenerator->update(delta);

	if (m_staminaUpdater)
		m_staminaUpdater->update(delta);
}

void Unit::setDeathState(DeathState state)
{
	m_deathState = state;

	switch (state)
	{
	case DEATH_STATE_DEAD:
		this->combatStop();
		this->stopMoving();
		this->stopStaminaUpdate();
		break;
	case DEATH_STATE_ALIVE:
		break;
	}
}

void Unit::cleanupBeforeDelete()
{
	WorldObject::cleanupBeforeDelete();
}

void Unit::activate()
{
	if (this->isActivated())
		return;

	WorldObject::activate();
}

void Unit::inactivate()
{
	if (!this->isActivated())
		return;

	this->combatStop();
	this->stopMoving();
	this->stopStaminaUpdate();

	WorldObject::inactivate();
}


bool Unit::canCombatWith(Unit* victim)
{
	if (!this->isAlive() || !victim->isInWorld() || !victim->isAlive())
		return false;

	if (victim == this)
		return false;


	return true;
}

void Unit::updateObject(UpdateType updateType, uint32 updateFlags, DataBasic* data)
{
	WorldObject::updateObject(updateType, updateFlags, data);
	DataUnit* dUnit = data->asDataUnit();
	if (!dUnit->isAlive())
	{
		if (this->isAlive())
		{
			this->setDeathState(DEATH_STATE_DEAD);
		}
	}
	else
	{
		if (!this->isAlive())
		{
			this->setDeathState(DEATH_STATE_ALIVE);
		}
	}
}

void Unit::stopStaminaUpdate()
{
	if (m_staminaUpdater)
		m_staminaUpdater->stop();
}

bool Unit::attackStop()
{
	this->clearUnitState(UNIT_STATE_ATTACKING);
	return true;
}

void Unit::combatStop()
{
	this->attackStop();
	//this->clearUnitState(UNIT_STATE_IN_COMBAT);

}

bool Unit::updatePosition(Point const& newPosition)
{
	this->getData()->setPosition(newPosition);
	return true;
}

NS_END
