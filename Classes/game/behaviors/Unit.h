#ifndef __UNIT_H__
#define __UNIT_H__

#include "common/utils/Timer.h"
#include "game/entities/DataUnit.h"
#include "game/movement/MoveSpline.h"
#include "game/movement/MovementGenerator.h"
#include "game/combat/StaminaUpdater.h"
#include "WorldObject.h"

NS_BEGIN

enum DeathState
{
	DEATH_STATE_ALIVE,
	DEATH_STATE_DEAD,
};


enum UnitState
{
	UNIT_STATE_NONE			= 0,
	UNIT_STATE_MOVING		= 1 << 1,
	/*UNIT_STATE_IN_COMBAT	= 1 << 2,*/
	UNIT_STATE_ATTACKING	= 1 << 3,
};

class Unit : public WorldObject
{
public:
	Unit();
	virtual ~Unit();

	void clearUnitState(uint32 state) { if (hasUnitState(state)) m_unitState &= ~state; }
	bool hasUnitState(uint32 state) const { return (m_unitState & state) != 0; }
	void addUnitState(uint32 state) { if (!hasUnitState(state)) m_unitState |= state; }
	uint32 getUnitState() const { return m_unitState; }

	bool isAlive() const { return m_deathState == DEATH_STATE_ALIVE; }
	virtual void setDeathState(DeathState state);

	void update(float delta) override;

	void cleanupBeforeDelete() override;
	void activate() override;
	void inactivate() override;

	MoveSpline* getMoveSpline() const { return m_moveSpline; }
	void setMoveSpline(MoveSpline* moveSpline) { m_moveSpline = moveSpline; }
	MovementGenerator* getMovementGenerator() const { return m_movementGenerator; }
	void setMovementGenerator(MovementGenerator* movementGenerator) { m_movementGenerator = movementGenerator; }
	virtual void stopMoving() { }

	StaminaUpdater* getStaminaUpdater() const { return m_staminaUpdater; }
	void setStaminaUpdater(StaminaUpdater* staminaUpdater) { m_staminaUpdater = staminaUpdater; }
	virtual void stopStaminaUpdate();

	virtual bool attackStop();
	virtual void combatStop();
	virtual bool canCombatWith(Unit* victim);

	virtual bool updatePosition(cocos2d::Point const& newPosition);

	void updateObject(UpdateType updateType, uint32 updateFlags, DataBasic* data) override;
	DataUnit* getData() override { return static_cast<DataUnit*>(m_data); }
    DataUnit const* getData() const override { return static_cast<DataUnit*>(m_data); }

protected:
	uint32 m_unitState;
	DeathState m_deathState;

	MovementGenerator* m_movementGenerator;
	MoveSpline* m_moveSpline;
	StaminaUpdater* m_staminaUpdater;
};

NS_END

#endif // __UNIT_H__
