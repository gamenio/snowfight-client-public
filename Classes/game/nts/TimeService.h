#ifndef __TIME_SERVICE_H__
#define __TIME_SERVICE_H__

#include "protocol/pb/TimeResult.pb.h"

#include "common/Common.h"
#include "common/utils/MessageDispatcher.h"
#include "NTSSocketMgr.h"

USING_NS_CC;

NS_BEGIN

class TimeSyncListener
{
public:
	virtual void onTimeSyncSuccess(int64 millis) {}
	virtual void onTimeSyncFail() { }
};

class TimeService
{
public:
	enum TimeState
	{
		TIME_NONE,
		TIME_SYNCING,
		TIME_SYNCED,
	};

	static TimeService* instance();
	void init();

	MessageDispatcher* getDispatcher() const { return m_dispatcher; }

	void syncTime();
	TimeState getTimeState() const { return m_timeState; }

	// Get the current time. If the return value is 0 the time is invalid
	int64 getCurrentTimeMillis();

	void addListener(TimeSyncListener* listener);
	void removeListener(TimeSyncListener* listener);

	void update(float delta);

private:
	TimeService();
	~TimeService();

	void updateTime(TimeInfo const& info);
	void resetTime();
	std::chrono::time_point<std::chrono::nanoseconds> getBootTime();

	void onTimeSyncSuccess(TimeInfo const& info);
	void onTimeSyncFail(NetworkError const& error);

	void handleMessage(Message const& message);

	MessageDispatcher* m_dispatcher;
	Scheduler* m_scheduler;

	NTSSocketMgr* m_socketMgr;
	int32 m_attempts;

	int64 m_remoteLastTime;
	std::chrono::system_clock::time_point m_hostLastTime;
	std::chrono::time_point<std::chrono::nanoseconds> m_lastUpdateTime;
	TimeState m_timeState;
	std::vector<TimeSyncListener*> m_listeners;
};

#define sTimeService TimeService::instance()


NS_END

#endif // __TIME_SERVICE_H__
