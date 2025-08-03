#ifndef __ROBOT_H__
#define __ROBOT_H__

#include "game/entities/DataRobot.h"
#include "game/movement/UnitMoveSpline.h"
#include "game/combat/UnitStaminaUpdater.h"
#include "Unit.h"


NS_BEGIN

class Robot : public Unit
{
public:
	Robot();
	virtual ~Robot();

	DataBasic* loadData(ObjectGuid const& guid);
	DataRobot* getData() const override { return static_cast<DataRobot*>(m_data); }
	virtual void updateObject(UpdateType updateType, uint32 updateFlags, DataBasic* data) override;

	UnitMoveSpline* getMoveSpline() const { return static_cast<UnitMoveSpline*>(Unit::getMoveSpline()); }
	virtual void stopMoving() override;

	UnitStaminaUpdater* getStaminaUpdater() const { return static_cast<UnitStaminaUpdater*>(Unit::getStaminaUpdater()); }

	virtual void update(float delta) override;
};

NS_END

#endif // __ROBOT_H__