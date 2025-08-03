#include "MyMovementGenerator.h"

#include "game/behaviors/MyCharacter.h"
#include "MyMoveSpline.h"


NS_BEGIN

MyMovementGenerator::MyMovementGenerator(MyCharacter* owner):
	m_owner(owner)
{

}

MyMovementGenerator::~MyMovementGenerator()
{
	m_owner = nullptr;
}

void MyMovementGenerator::update(float delta)
{
}

void MyMovementGenerator::moveBy(float direction)
{
	if (!m_owner->hasUnitState(UNIT_STATE_MOVING))
	{
		this->sendMoveStart();
		m_owner->addUnitState(UNIT_STATE_MOVING);
	}
	m_owner->getMoveSpline()->moveByDirection(direction);
}

void MyMovementGenerator::finish()
{
	if (m_owner->hasUnitState(UNIT_STATE_MOVING))
	{
		m_owner->getMoveSpline()->stop();
		this->sendMoveStop();

		m_owner->clearUnitState(UNIT_STATE_MOVING);
	}
}

void MyMovementGenerator::sendMoveStart()
{
	if (!m_owner->getSession())
		return;

	MovementInfo movement;
	m_owner->buildMovementInfo(movement);
	WorldPacket packet(world::MSG_MOVE_START, std::move(movement));
	m_owner->getSession()->sendPacket(std::move(packet));
	//CCLOG("MSG_MOVE_START");
}

void MyMovementGenerator::sendMoveStop()
{
	if (!m_owner->getSession())
		return;

	MovementInfo movement;
	m_owner->buildMovementInfo(movement);
	WorldPacket packet(world::MSG_MOVE_STOP, std::move(movement));
	m_owner->getSession()->sendPacket(std::move(packet));
	//CCLOG("MSG_MOVE_STOP");
}

NS_END