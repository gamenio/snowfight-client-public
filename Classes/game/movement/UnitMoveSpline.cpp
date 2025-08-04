#include "UnitMoveSpline.h"

#include "common/utils/MathTools.h"
#include "common/utils/Timer.h"
#include "common/utils/TimeUtil.h"
#include "common/debugging/DebugDrawer.h"
#include "game/behaviors/Unit.h"
#include "game/movement/MoveSegmentImpl.h"
#include "game/entities/updates/ObjectUpdateFields.h"



NS_BEGIN

UnitMoveSpline::UnitMoveSpline(Unit* owner) :
	m_owner(owner),
	m_isFinished(true),
	m_isCleanupMoveFlags(false),
	m_currSegment(nullptr),
	m_realMoveSpeed(0)
{
}

UnitMoveSpline::~UnitMoveSpline()
{
	m_owner = nullptr;
	CC_SAFE_DELETE(m_currSegment);
}

void UnitMoveSpline::update(float delta)
{
	if (!m_currSegment)
		return;

	// Check if the moving speed has changed
	if (m_owner->getData()->hasUpdatedField(CUNIT_FIELD_MOVE_SPEED) && m_realMoveSpeed > 0)
	{
		int32 moveSpeed = m_owner->getData()->getMoveSpeed();
		float speedScale = moveSpeed / (float)m_realMoveSpeed;
		m_currSegment->setSpeedScale(speedScale);
	}

	m_currSegment->step(delta);
	if (m_currSegment->isDone())
	{
		CC_SAFE_DELETE(m_currSegment);
		if (m_isCleanupMoveFlags)
			m_owner->getData()->clearMovementFlag(MOVEMENT_FLAG_WALKING);

		m_isFinished = true;
	}
}


void UnitMoveSpline::moveBy(MovementInfo const& movement)
{
	m_isFinished = false;
	this->process(movement);
}

bool UnitMoveSpline::stop(bool isCorrectPosition)
{
	this->finish(isCorrectPosition);

	m_isCleanupMoveFlags = false;

	return true;
}

void UnitMoveSpline::stopSegment()
{
	if (!m_currSegment)
		return;

	CC_SAFE_DELETE(m_currSegment);
	m_realMoveSpeed = 0;

	//CCLOG("Stop current segment");
}

void UnitMoveSpline::finish(bool isCorrectPosition)
{
	if (m_isFinished)
		return;

	if (isCorrectPosition)
	{
		if (m_currSegment)
			m_owner->updatePosition(m_currSegment->getEndPosition());
	}

	this->stopSegment();
	m_isFinished = true;
}

void UnitMoveSpline::process(MovementInfo const& movement)
{
	m_owner->getData()->expectFacingToAngle(movement.orientation);
	m_owner->getData()->addMovementFlag(MOVEMENT_FLAG_WALKING);

	Point endPos = movement.position;
	Point currPos = m_owner->getData()->getPosition();

	// Calculate the delay and moving distance
	float delay = time_util::toGameTimeSeconds(std::max(0, time_util::getUptimeMillis() - movement.time));
	float length = endPos.getDistance(currPos);

	// Calculate the remaining moving time
	m_realMoveSpeed = m_owner->getData()->getMoveSpeed();
	float movingTime = MathTools::computeMovingTimeSec(length, m_realMoveSpeed);
	float remaining = movingTime - delay;
	//CCLOG("UNIT MOVESPLINE guid: 0x%08X delay: %f movingtime: %f/%f length: %f moveto: [%.1f,%.1f]->[%.1f,%.1f] orient: %.1f",
	//	m_owner->getData()->getGuid(),
	//	delay,
	//	remaining, movingTime,
	//	length,
	//	currPos.x, currPos.y, endPos.x, endPos.y, 
	//	movement.orientation);
	if (remaining > 0)
	{
		if (m_currSegment)
			m_currSegment->advance(remaining, endPos);
		else
			m_currSegment = new MoveSegment<Unit>(m_owner, remaining, endPos);
	}
	else
	{
		this->stopSegment();

		// Move directly to the target position
		m_owner->updatePosition(endPos);

		if (m_isCleanupMoveFlags)
			m_owner->getData()->clearMovementFlag(MOVEMENT_FLAG_WALKING);

		m_isFinished = true;
	}

}


NS_END
