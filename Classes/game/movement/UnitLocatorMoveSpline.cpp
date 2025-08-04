#include "UnitLocatorMoveSpline.h"

#include "common/utils/MathTools.h"
#include "common/utils/TimeUtil.h"
#include "game/behaviors/UnitLocator.h"
#include "game/movement/MoveSegmentImpl.h"


NS_BEGIN

UnitLocatorMoveSpline::UnitLocatorMoveSpline(UnitLocator* owner) :
	m_owner(owner),
	m_isFinished(true),
	m_currSegment(nullptr)
{
}

UnitLocatorMoveSpline::~UnitLocatorMoveSpline()
{
	m_owner = nullptr;
	CC_SAFE_DELETE(m_currSegment);
}

void UnitLocatorMoveSpline::update(float delta)
{
	if (!m_currSegment)
		return;

	m_currSegment->step(delta);
	if (m_currSegment->isDone())
	{
		CC_SAFE_DELETE(m_currSegment);
		m_isFinished = true;
	}
}


void UnitLocatorMoveSpline::moveBy(LocationInfo const& location)
{
	m_isFinished = false;
	this->process(location);
}

bool UnitLocatorMoveSpline::stop(bool isCorrectPosition)
{
	if (m_isFinished)
		return true;

	if (isCorrectPosition)
	{
		if (m_currSegment)
			m_owner->updatePosition(m_currSegment->getEndPosition());
	}

	this->stopSegment();

	m_isFinished = true;

	return true;
}

void UnitLocatorMoveSpline::stopSegment()
{
	if (!m_currSegment)
		return;

	CC_SAFE_DELETE(m_currSegment);

	//CCLOG("Stop current segment");
}

void UnitLocatorMoveSpline::process(LocationInfo const& location)
{
	Point endPos = location.position;
	Point currPos = m_owner->getData()->getPosition();

	// Calculate the delay and moving distance
	float delay = time_util::toGameTimeSeconds(std::max(0, time_util::getUptimeMillis() - location.time));
	float length = endPos.getDistance(currPos);

	// Calculate the remaining moving time
	float movingTime = MathTools::computeMovingTimeSec(length, m_owner->getData()->getMoveSpeed());
	float remaining = movingTime - delay;
	//CCLOG("LOCATOR UNIT MOVESPLINE guid: 0x%08X delay: %f movingtime: %f/%f length: %f moveto: [%.1f,%.1f]->[%.1f,%.1f] orient: %.1f",
	//	m_owner->getData()->getGuid(),
	//	delay,
	//	remaining, movingTime,
	//	length,
	//	currPos.x, currPos.y, endPos.x, endPos.y);
	if (remaining > 0)
	{
		if (m_currSegment)
			m_currSegment->advance(remaining, endPos);
		else
			m_currSegment = new MoveSegment<UnitLocator>(m_owner, remaining, endPos);
	}
	else
	{
		this->stopSegment();
		// Move directly to the target position
		m_owner->updatePosition(endPos);
		m_isFinished = true;
	}

}


NS_END
