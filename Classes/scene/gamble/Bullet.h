#ifndef __BULLET_H__
#define __BULLET_H__

#include "common/Common.h"
#include "game/entities/DataProjectile.h"
#include "game/utils/TrajectoryGenerator.h"
#include "game/WorldListeners.h"
#include "scene/actions/BezierCurve.h"
#include "GameObject.h"


USING_NS_CC;


NS_BEGIN


class Role;

class Bullet : public GameObject
{
public:
	bool init() override;
	virtual bool initWithData(DataProjectile* data);

	Bullet();
	virtual ~Bullet();

	virtual void onActivated() override;
	virtual void onInactivated() override;

	DataProjectile* getData() const override { return m_data; }

	void update(float delta) override;

	void onLaunchResult(LaunchStatus status, ObjectGuid const& target, cocos2d::Point const& position);

protected:
	virtual void reset();
	virtual void launch();

	// Called when the shadow position changes
	// The position is the relative position of the shadow to the bullet
	virtual void didShadowRelocation(cocos2d::Point const& position, float vertexZ){ }

	virtual void didHit(ObjectGuid const& target) { }
	virtual void didFallToGround() { }
	virtual void didLaunch() { }

private:
	void stopParabolaAction();
	void updateShadow();

	void finished();

	DataProjectile* m_data;
	BezierCurve* m_parabola;

};

NS_END

#endif // __BULLET_H__
