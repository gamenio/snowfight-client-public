#include "MovementTest.h"

#include "Common.h"
#include "game/behaviors/MyCharacter.h"
#include "game/tiles/TileArea.h"
#include "TestService.h"

#define MOVE_MIN_INTERVAL				1.0f
#define MOVE_MAX_INTERVAL				3.0f

MovementTest::MovementTest(std::shared_ptr<World> world) :
	m_world(world)
{
	m_moveTimer.setInterval(MOVE_MIN_INTERVAL);
	m_moveTimer.setPassed();
}

MovementTest::~MovementTest()
{

}

bool MovementTest::runTest()
{
	this->doMove();

	return true;
}

void MovementTest::update(float dt)
{
	TestBase::update(dt);

	m_moveTimer.update(dt);
	if (m_moveTimer.passed())
	{
		this->doMove();
		float inr = MOVE_MIN_INTERVAL + CCRANDOM_0_1() * (MOVE_MAX_INTERVAL - MOVE_MIN_INTERVAL);
		m_moveTimer.setInterval(inr);
	}
}

void MovementTest::finish()
{
	if (m_isFinished)
		return;

	TestBase::finish();
}

void MovementTest::doMove()
{
	MyCharacter* myChar = m_world->getMyCharacter();
	if (myChar)
	{
		float dir = random(-M_PI, M_PI);
		myChar->moveBy(dir);
	}
}