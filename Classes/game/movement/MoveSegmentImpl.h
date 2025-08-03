#ifndef __MOVE_SEGMENT_IMPL_H__
#define __MOVE_SEGMENT_IMPL_H__

#include "MoveSegment.h"

#include "common/utils/MathTools.h"

NS_BEGIN

template<typename T>
MoveSegment<T>::MoveSegment(T* target, float duration, cocos2d::Point const& endPosition) :
	m_target(target),
	m_elapsed(0.f),
	m_realDuration(duration),
	m_duration(duration),
	m_speedScale(1.0f),
	m_startPosition(target->getData()->getPosition()),
	m_endPosition(endPosition)
{
	m_positionDelta = m_endPosition - m_startPosition;
}

template<typename T>
MoveSegment<T>::~MoveSegment()
{
	m_target = nullptr;
}

template<typename T>
void MoveSegment<T>::advance(float duration, cocos2d::Point const& endPosition)
{
	m_realDuration = m_duration = duration;
	m_elapsed = 0;

	m_endPosition = endPosition;
	m_startPosition = m_target->getData()->getPosition();
	m_positionDelta = m_endPosition - m_startPosition;
}


template<typename T>
void MoveSegment<T>::setSpeedScale(float scale)
{
	if (m_speedScale == scale ||  scale <= 0.f)
		return;

	float duration = m_realDuration *  (1.f / scale);
	float ratio = MAX(0, MIN(1, m_elapsed / m_duration));
	m_elapsed = ratio * duration;
	m_duration = duration;
	m_speedScale = scale;
}

template<typename T>
void MoveSegment<T>::update(float t)
{
	if (!m_target)
		return;

	cocos2d::Point newPos = m_startPosition + (m_positionDelta * t);
    m_target->updatePosition(newPos);
}

template<typename T>
void MoveSegment<T>::step(float dt)
{
	m_elapsed += dt;


	float updateDt = MAX(0,                                  // needed for rewind. elapsed could be negative
		MIN(1, m_elapsed / m_duration)
	);

	this->update(updateDt);
}

template<typename T>
bool MoveSegment<T>::isDone() const
{
	return m_elapsed >= m_duration;
}

NS_END

#endif // __MOVE_SEGMENT_IMPL_H__