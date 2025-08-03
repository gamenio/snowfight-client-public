#include "PlayerMovementGenerator.h"

#include "game/behaviors/Unit.h"

NS_BEGIN

PlayerMovementGenerator::PlayerMovementGenerator(Unit* owner):
	m_owner(owner)
{
}

PlayerMovementGenerator::~PlayerMovementGenerator()
{
	m_owner = nullptr;
}

void PlayerMovementGenerator::update(float delta)
{

}

void PlayerMovementGenerator::finish()
{

}

void PlayerMovementGenerator::moveTo(cocos2d::Point const& dest)
{
	//m_owner->getData()->addUnitState(UNIT_STATE_MOVING);
	//m_owner->getMoveSpline()->moveByPoint(dest);
}

NS_END