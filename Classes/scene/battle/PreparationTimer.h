#ifndef __PREPARATION_TIMER_H__
#define __PREPARATION_TIMER_H__

#include "common/Common.h"

USING_NS_CC;

NS_BEGIN

class PreparationTimer : public Node
{
public:
	PreparationTimer();
	~PreparationTimer();

	static PreparationTimer* create();
	bool init() override;

	void start(NSTime milliseconds);
	void stop();

	void onEnter() override;
	void update(float delta) override;

private:
	void updateTimeLeft(int32 seconds);

	NSTime m_duration;
	NSTime m_startTime;
	int32 m_currTimeLeft;
	bool m_isStopped;

	Sprite* m_numberSp;
};


NS_END

#endif // __PREPARATION_TIMER_H__
