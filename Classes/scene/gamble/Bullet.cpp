#include "Bullet.h"

#include "common/debugging/DebugDrawer.h"
#include "common/utils/TimeUtil.h"
#include "game/entities/updates/ObjectUpdateFields.h"
#include "game/behaviors/Projectile.h"
#include "scene/actions/BezierCurve.h"
#include "Role.h"

NS_BEGIN


#if NS_DEBUG

#define DEBUG_BULLET_SHADOW			0
#define DEBUG_COLLISION_BOX			0
#define DEBUG_TRAJECTORY			0

#endif // NS_DEBUG


Bullet::Bullet():
	m_data(nullptr),
	m_parabola(nullptr)
{

}

Bullet::~Bullet()
{
	this->stopParabolaAction();

	CC_SAFE_RELEASE_NULL(m_data);
}

bool Bullet::init()
{
	if (!GameObject::init())
		return false;

	return true;
}

bool Bullet::initWithData(DataProjectile* data)
{
	if (!GameObject::init())
		return false;

	CC_SAFE_RETAIN(data);
	m_data = data;

	return true;
}

void Bullet::onActivated()
{
	if (this->isActive())
		return;

	GameObject::onActivated();

	if (m_data->getStatus() == LAUNCHSTATUS_NONE)
		this->launch();
}

void Bullet::onInactivated()
{
	if (!this->isActive())
		return;

	this->reset();

	GameObject::onInactivated();
}

void Bullet::update(float delta)
{
	GameObject::update(delta);

	if (!this->isActive())
		return;

	this->updateShadow();

#if DEBUG_COLLISION_BOX
	auto debugDraw = sDebugDrawer->getDrawByTag("Bullet.CollisionBox");
	debugDraw->setClearing(true);
	Size projSize = this->getData()->getObjectSize() * this->getData()->getScale();
	Rect collbox = Rect(this->getPosition().x - (projSize.width * this->getData()->getAnchorPoint().x),
		this->getPosition().y - (projSize.height * this->getData()->getAnchorPoint().y),
		projSize.width,
		projSize.height);
	debugDraw->drawRect(collbox.origin, collbox.size, Color4F::RED);
#endif

	if (m_parabola && m_parabola->isDone())
	{
		this->finished();
		CC_SAFE_RELEASE_NULL(m_parabola);
	}
}

void Bullet::onLaunchResult(LaunchStatus status, ObjectGuid const& target, cocos2d::Point const& position)
{
	this->setPosition(position);
	this->stopParabolaAction();
	this->didHit(target);
}

void Bullet::reset()
{
	this->stopParabolaAction();
	this->setVisible(false);
	this->setPosition(Point::ZERO);
}

void Bullet::launch()
{
	NS_ASSERT(m_data != nullptr);

#if DEBUG_BULLET_SHADOW
	sDebugDrawer->getDrawByTag("Bullet.Shadow")->clear();
#endif // DEBUG_BULLET_SHADOW

	BezierCurveConfig const& config = m_data->getTrajectory();
#if DEBUG_TRAJECTORY
	Brush* debugDraw = sDebugDrawer->getDrawByTag("Bullet.Trajectory");
	debugDraw->clear();
	config.draw(debugDraw->getDrawNode());
#endif // DEBUG_TRAJECTORY
	CC_SAFE_RELEASE_NULL(m_parabola);

	this->setPosition(config.startPosition);
	this->setVisible(true);

	// Parabolic motion animation
	float duration = time_util::toGameTimeSeconds(m_data->getDuration());
	float elapsed = time_util::toGameTimeSeconds(m_data->getElapsed());
	elapsed = std::min(elapsed, duration);
	m_parabola = new BezierCurve();
	m_parabola->initWithDuration(duration, config);
	this->runAction(m_parabola);
	m_parabola->step(0);
	m_parabola->step(elapsed);

	this->didLaunch();

	if (elapsed <= 0)
	{
		auto launcher = m_gameMapLayer->getGameObject<Role>(m_data->getLauncher());
		if (launcher)
			launcher->hurl();
	}
}

void Bullet::finished()
{
	this->didFallToGround();
}

void Bullet::updateShadow()
{
	if (!m_parabola)
		return;

	BezierCurveConfig const& config = m_data->getTrajectory();
	Point startPos = config.startPosition;
	Point endPos = config.startPosition + config.endPosition;
	Point currPos = this->getPosition();

	// Calculate the starting Y coordinate of the shadow
	Point shadowStartPos;
	shadowStartPos.x = startPos.x;
	shadowStartPos.y = startPos.y - m_data->getLaunchCenter().y;

	// Calculate the current position of the shadow
	float scale = std::max(0.f, std::min(1.f, m_parabola->getElapsed() / m_parabola->getDuration()));
	float dx = (endPos.x - shadowStartPos.x) * scale;
	float dy = (endPos.y - shadowStartPos.y) * scale;
	Point shadowCurrPos(shadowStartPos.x + dx, shadowStartPos.y + dy);

#if NS_DEBUG

	Brush* debugDraw;
	debugDraw = nullptr;

#if DEBUG_BULLET_SHADOW
	debugDraw = sDebugDrawer->getDrawByTag("Bullet.Shadow");
	debugDraw->drawLine(shadowStartPos, endPos, Color4F::GRAY);
	debugDraw->drawLine(shadowStartPos, shadowCurrPos, Color4F::ORANGE);
	debugDraw->drawLine(startPos, shadowStartPos, Color4F::BLUE);

#endif // DEBUG_BULLET_SHADOW

#if DEBUG_COLLISION_BOX
	debugDraw = sDebugDrawer->getDrawByTag("Bullet.CollisionBox");
	debugDraw->setClearing(true);
	debugDraw->drawCircle(shadowCurrPos, this->getData()->getObjectRadiusInMap() * this->getData()->getScale(), std::sqrt(2), std::sqrt(2) / 2, Color4F::RED);
#endif // DEBUG_COLLISION_BOX

#endif // NS_DEBUG

	// Calculate the Z coordinate
	float newZ;
	MapData const* mapData = m_data->getMapData();
	Size mapSize = mapData->getMapSize();
	TileCoord coord(mapSize, shadowCurrPos);
	if (mapData->isPenetrable(coord))
	{
		// Ensure that the snowball is displayed above the penetrable building
		newZ = mapData->getMaxTileZ();
	}
	else
	{
		float offsetZ;
		newZ = mapData->getTileZForPos(shadowCurrPos, &offsetZ);
		newZ += offsetZ;
	}

	Point relPos(0, shadowCurrPos.y - currPos.y);
	this->didShadowRelocation(relPos, newZ);
}

void Bullet::stopParabolaAction()
{
	if (!m_parabola)
		return;

	this->stopAction(m_parabola);
	CC_SAFE_RELEASE_NULL(m_parabola);
}

NS_END
