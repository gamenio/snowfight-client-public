#ifndef __COMBAT_TEST_H__
#define __COMBAT_TEST_H__

#include "TestBase.h"
#include "game/World.h"


class CombatTest: public TestBase
{
	enum AttackMode
	{
		ATTACK_NODE_NORMAL,
		ATTACK_MODE_CONTINUOUS,
		ATTACK_MODE_CHARGED,
	};
public:
	CombatTest(std::shared_ptr<World> world);
	~CombatTest();

	bool runTest() override;
	void update(float dt) override;

private:
	void doAttack();
	void updateAttackDelay();

	std::shared_ptr<World> m_world;
	std::unordered_map<ObjectGuid, DataBasic*> m_objectDataSet;
	DelayTimer m_attackTimer;

	AttackMode m_attackMode;
	bool m_isInCharge;
};



#endif // __COMBAT_TEST_H__
