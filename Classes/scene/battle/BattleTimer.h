#ifndef __BATTLE_TIMER_H__
#define __BATTLE_TIMER_H__

#include "common/Common.h"
#include "common/utils/Timer.h"

USING_NS_CC;

NS_BEGIN

class BattleTimer : public Node
{
public:
	BattleTimer();
	~BattleTimer();

	static BattleTimer* create();
	bool init() override;

	void setDuration(NSTime milliseconds);
	NSTime getDuration() const { return m_duration; }

	void start();
	void stop();

	void onEnter() override;
	void update(float delta) override;

private:
	void updateTimeLeft(int32 seconds);

	NSTime m_duration;
	NSTime m_startTime;
	int32 m_currTimeLeft;
	bool m_isCountdownStopped;

	Label* m_timeLabel;
};


NS_END

#endif // __BATTLE_TIMER_H__
