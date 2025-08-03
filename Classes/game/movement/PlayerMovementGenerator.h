#ifndef __PLAYER_MOVEMENT_GENERATOR_H__
#define __PLAYER_MOVEMENT_GENERATOR_H__

#include "common/Common.h"
#include "game/movement/MovementGenerator.h"

NS_BEGIN

class Unit;

class PlayerMovementGenerator: public MovementGenerator
{
public:
	PlayerMovementGenerator(Unit* owner);
	~PlayerMovementGenerator();

	virtual void update(float delta) override;
	virtual void finish() override;

	void moveTo(cocos2d::Point const& dest);

private:
	Unit* m_owner;
};

NS_END

#endif // __PLAYER_MOVEMENT_GENERATOR_H__
