#ifndef __DATA_PROJECTILE_H__
#define __DATA_PROJECTILE_H__

#include "common/Common.h"
#include "updates/ObjectUpdateFields.h"
#include "game/utils/TrajectoryGenerator.h"
#include "DataWorldObject.h"

NS_BEGIN

#define PROJECTILE_SPEED			355.f			// 抛射体的速度。单位：points/second 

enum LaunchStatus
{
	LAUNCHSTATUS_NONE				= 0,
	LAUNCHSTATUS_FINISHED,					// 落到预定目的地
	LAUNCHSTATUS_HIT_TARGET,				// 击中目标
	LAUNCHSTATUS_HIT_BUILDING,				// 与建筑物发生碰撞
	LAUNCHSTATUS_FAILED,					// 发射失败
};


class DataProjectile: public DataWorldObject
{
public:
	DataProjectile();
	virtual ~DataProjectile();
    
	void clearFields() override;
	uint32 getReadFieldCount() const override { return SPROJECTILE_END; }
    bool readFields(UpdateType updateType, uint32 updateFlags, FieldUpdateMask const& updateMask, DataInputStream* input) override;

	ObjectGuid const&  getLauncher() const { return m_launcher; }

	cocos2d::Point const& getLauncherOrigin() const { return m_launcherOrigin; }
	void setLauncherOrigin(cocos2d::Point origin) { m_launcherOrigin = origin; }

	float getAttackRange() const { return m_attackRange; }

	cocos2d::Point const& getLaunchCenter() const { return m_launchCenter; }
	void setLaunchCenter(cocos2d::Point const& center) { m_launchCenter = center; }
	float getLaunchRadiusInMap() const { return m_launchRadiusInMap; };
	void setLaunchRadiusInMap(float radius) { m_launchRadiusInMap = radius; }

	cocos2d::Point const& getPosition() const override { return m_position; }
	float getOrientation() const { return m_orientation; }

	int32 getElapsed() const { return m_elapsed; }
	int32 getDuration() const { return m_duration; }

	uint32 getAttackCounter() const { return m_attackCounter; }
	int32 getConsumedStamina() const { return m_consumedStamina; }
	float getScale() const { return m_scale; }
	uint32 getAttackInfoCounter() const { return m_attackInfoCounter; }

	void setTrajectory(BezierCurveConfig const& trajectory) { m_trajectory = trajectory; }
	BezierCurveConfig const& getTrajectory() const { return m_trajectory; }
 
	LaunchStatus getStatus() const { return m_status; }

private:
	ObjectGuid m_launcher;
	cocos2d::Point m_launcherOrigin;
	float m_attackRange;

	cocos2d::Point m_launchCenter;
	float m_launchRadiusInMap;
	cocos2d::Point m_position;
	float m_orientation;

	int32 m_elapsed;
	int32 m_duration;
	uint32 m_attackCounter;
	int32 m_consumedStamina;
	float m_scale;
	uint32 m_attackInfoCounter;

	BezierCurveConfig m_trajectory;
	LaunchStatus m_status;
};


NS_END

#endif // __DATA_PROJECTILE_H__
