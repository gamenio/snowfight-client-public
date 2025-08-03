#ifndef __MOVE_SEGMENT_H__
#define __MOVE_SEGMENT_H__

#include "cocos2d.h"
#include "common/Common.h"

USING_NS_CC;

NS_BEGIN

template<typename T>
class MoveSegment
{
public:
	MoveSegment(T* target, float duration, cocos2d::Point const& endPosition);
	~MoveSegment();

	void advance(float duration, cocos2d::Point const& endPosition);
	// 设置移动速度比例，值必须大于零
	void setSpeedScale(float scale);
	float getDuration() const { return m_duration; }
	cocos2d::Point const& getEndPosition() const { return m_endPosition; }
	cocos2d::Point const& getStartPosition() const { return m_startPosition; }

	void step(float dt);
	bool isDone() const;

private:
	void update(float time);

	T* m_target;

	float m_elapsed;
	float m_realDuration;
	float m_duration;
	float m_speedScale;

	cocos2d::Point m_positionDelta;
	cocos2d::Point m_endPosition;
	cocos2d::Point m_startPosition;
};

NS_END

#endif //__MOVE_SEGMENT_H__
