#ifndef __MOVEMENT_TEST_H__
#define __MOVEMENT_TEST_H__

#include "TestBase.h"
#include "game/World.h"


class MovementTest : public TestBase
{
public:
	MovementTest(std::shared_ptr<World> world);
	~MovementTest();

	bool runTest() override;
	void update(float dt) override;

	void finish() override;

private:
	void doMove();

	std::shared_ptr<World> m_world;
	IntervalTimer m_moveTimer;
};



#endif // __MOVEMENT_TEST_H__
