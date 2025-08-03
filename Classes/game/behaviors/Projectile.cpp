#include "Projectile.h"

#include "common/utils/TimeUtil.h"
#include "game/utils/UnitHelper.h"
#include "game/behaviors/ObjectShapes.h"
#include "MyCharacter.h"
#include "game/World.h"

NS_BEGIN

Projectile::Projectile()
{
	m_type |= TypeMask::TYPEMASK_PROJECTILE;
	m_typeId = TypeID::TYPEID_PROJECTILE;
}

Projectile::~Projectile()
{
}

void Projectile::update(float delta)
{
}

DataBasic* Projectile::loadData(ObjectGuid const& guid)
{
	DataProjectile* data = new DataProjectile();
	data->autorelease();
	data->setGuid(guid);

	data->setObjectSize(PROJECTILE_OBJECT_SIZE);
	data->setAnchorPoint(PROJECTILE_ANCHOR_POINT);
	data->setObjectRadiusInMap(PROJECTILE_OBJECT_RADIUS_IN_MAP);

	this->setData(data);

	return data;
}


void Projectile::updateObject(UpdateType updateType, uint32 updateFlags, DataBasic* data)
{
	NS_ASSERT(updateType == UPDATE_TYPE_CREATE);

	DataProjectile* dProj = data->asDataProjectile();
	//NSTime delay = (NSTime)(time_util::getUptimeMillis() - dProj->getTime());
	//CCLOG("Projectile::updateObject delay: %dms", delay);
	NS_ASSERT(dProj->getElapsed() <= dProj->getDuration());

	MyCharacter* myChar = this->getMap()->getMyChar();
	NS_ASSERT(myChar != nullptr);

	// 我的抛射体
	if (dProj->getLauncher() == myChar->getData()->getGuid())
	{
		// 抛射体未开始运动并且未设置发射结果
		if (dProj->getElapsed() <= 0 && dProj->getStatus() == LAUNCHSTATUS_NONE)
		{
			myChar->getData()->expectFacingToAngle(dProj->getOrientation());
			myChar->lockFacingDirection();
			if (dProj->getAttackInfoCounter() == myChar->getAttackInfoCounter())
			{
				NS_ASSERT(myChar->getData()->hasMovementFlag(MOVEMENT_FLAG_HANDUP));
				if (myChar->canHandDown())
					myChar->startHandDownTimer();
			}
			myChar->getStaminaUpdater()->consumeStamina(dProj->getConsumedStamina(), dProj->getAttackCounter());
			dProj->setLauncherOrigin(myChar->getData()->getPosition());
		}
	}
	// 其他玩家或机器人的抛射体
	else
	{
		WorldObject* object = this->getMap()->findObject(dProj->getLauncher());
		Unit* launcher = object ? object->asUnit() : nullptr;

		// 本地能查找到发射者
		if (launcher
			&& dProj->getElapsed() <= 0 && dProj->getStatus() == LAUNCHSTATUS_NONE)
		{
			NS_ASSERT(!launcher->isAlive() || launcher->getData()->hasMovementFlag(MOVEMENT_FLAG_HANDUP));

			launcher->getData()->expectFacingToAngle(dProj->getOrientation());
			launcher->addUnitState(UNIT_STATE_ATTACKING);
			dProj->setLauncherOrigin(launcher->getData()->getPosition());
		}
	}

	Vec2 landingPos = UnitHelper::computeLandingPosition(dProj->getLauncherOrigin(), dProj->getAttackRange(), dProj->getOrientation());
	Vec2 launchPos = UnitHelper::computeLaunchPosition(dProj->getMapData(), dProj->getLauncherOrigin(), dProj->getLaunchCenter(), dProj->getLaunchRadiusInMap(), landingPos);
	TrajectoryGenerator trajGenerator(TRAJECTORY_TYPE_PROJECTILE, launchPos, landingPos);
	trajGenerator.compute();
	dProj->setTrajectory(trajGenerator.getBezierCurveConfig());

	this->getMap()->activateObject(this);
}


NS_END
