#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "game/entities/DataPlayer.h"
#include "game/client/WorldSession.h"
#include "game/movement/UnitMoveSpline.h"
#include "game/combat/UnitStaminaUpdater.h"
#include "Unit.h"

NS_BEGIN

class Player: public Unit
{
public:
	Player();
	virtual ~Player();

	UnitMoveSpline* getMoveSpline() const { return static_cast<UnitMoveSpline*>(Unit::getMoveSpline()); }
	virtual void stopMoving() override;

	UnitStaminaUpdater* getStaminaUpdater() const { return static_cast<UnitStaminaUpdater*>(Unit::getStaminaUpdater()); }

	DataBasic* loadData(ObjectGuid const& guid);
	DataPlayer* getData() override { return static_cast<DataPlayer*>(m_data); }
	DataPlayer const* getData() const override { return static_cast<DataPlayer*>(m_data); }
	void updateObject(UpdateType updateType, uint32 updateFlags, DataBasic* data) override;    

};

NS_END

#endif // __PLAYER_H__