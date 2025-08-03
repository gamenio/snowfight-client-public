#include "CombatTest.h"

#include "Common.h"
#include "common/utils/TimeUtil.h"
#include "common/utils/MathTools.h"
#include "game/behaviors/MyCharacter.h"
#include "game/tiles/TileArea.h"
#include "TestService.h"


#define CONTINUOUS_ATTACK_DELAY_MAX					3.f
#define CHARGED_ATTACK_DELAY_MAX					6.f

CombatTest::CombatTest(std::shared_ptr<World> world) :
	m_world(world),
	m_isInCharge(false),
	m_attackMode(ATTACK_NODE_NORMAL)
{
}

CombatTest::~CombatTest()
{
}

bool CombatTest::runTest()
{
	DataPlayer* myChar = m_world->getMyself();
	if (!myChar)
		return false;

	m_attackTimer.passed();

	m_world->sendGMCommand("add carrieditem 8"); // Add Snowball Bucket

	return true;
}

void CombatTest::update(float dt)
{
	TestBase::update(dt);

	m_attackTimer.update(dt);
	if (m_attackTimer.passed())
	{
		this->doAttack();
		this->updateAttackDelay();
	}
}

void CombatTest::doAttack()
{
	MyCharacter* myChar = m_world->getMyCharacter();
	if (!myChar)
		return;

	m_attackMode = (AttackMode)random((int32)ATTACK_NODE_NORMAL, (int32)ATTACK_MODE_CHARGED);
	if (m_attackMode == ATTACK_MODE_CHARGED && !m_isInCharge)
	{
		myChar->charge();
		m_isInCharge = true;
	}
	else
	{
		float direction;
		if (m_attackMode == ATTACK_MODE_CONTINUOUS)
		{
			Point targetPos;
			if (myChar->getClosestTargetPosition(targetPos))
				direction = MathTools::computeAngleInRadians(myChar->getData()->getPosition(), targetPos);
			else
				direction = random(-M_PI, M_PI);
		}
		else
			direction = FLT_MAX;


		myChar->attack(direction);
		m_isInCharge = false;
	}
}

void CombatTest::updateAttackDelay()
{
	float delay = 0.f;
	switch (m_attackMode)
	{
	case ATTACK_MODE_CHARGED:
		delay = random(0.f, CHARGED_ATTACK_DELAY_MAX);
		break;
	case ATTACK_MODE_CONTINUOUS:
		delay = random(0.f, CONTINUOUS_ATTACK_DELAY_MAX);
		break;
	default: // ATTACK_NODE_NORMAL
		break;
	}
	m_attackTimer.setDuration(delay);
}
