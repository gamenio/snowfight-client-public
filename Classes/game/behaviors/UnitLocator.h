#ifndef __UNIT_LOCATOR_H__
#define __UNIT_LOCATOR_H__

#include "common/Common.h"
#include "game/entities/DataUnitLocator.h"
#include "game/movement/UnitLocatorMoveSpline.h"
#include "game/maps/BattleMap.h"
#include "LocatorObject.h"

NS_BEGIN

class UnitLocator: public LocatorObject
{
public:
	UnitLocator();
	virtual ~UnitLocator();

	void update(float delta) override;
	virtual bool updatePosition(cocos2d::Point const& newPosition);

	bool isAlive() const;
	void stopMoving();

	UnitLocatorMoveSpline* getMoveSpline() const { return m_moveSpline; }

	void activate() override;
	void inactivate() override;

	void updateObject(UpdateType updateType, uint32 updateFlags, DataBasic* data) override;
	DataUnitLocator* getData() override { return static_cast<DataUnitLocator*>(m_data); }
	DataUnitLocator const* getData() const override { return static_cast<DataUnitLocator*>(m_data); }
	DataBasic* loadData(ObjectGuid const& guid);
    
protected:
	UnitLocatorMoveSpline* m_moveSpline;
};

NS_END

#endif // __UNIT_LOCATOR_H__