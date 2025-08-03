#include "TimeService.h"

#include "common/utils/TimeUtil.h"
#include "game/ClientConfig.h"
#include "game/firservice/FirebaseService.h"
#include "MessageEnum.h"

#include <time.h>

#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS
#include <sys/sysctl.h>

#endif

NS_BEGIN

#define MAX_ATTEMPTS				2			// 时间同步失败后最大尝试次数

using SystemClock = std::chrono::system_clock;
using SteadyClock = std::chrono::steady_clock;

TimeService* TimeService::instance()
{
	static TimeService instance;
	return &instance;
}

void TimeService::init()
{
	if (m_socketMgr)
		return;

	m_socketMgr = new NTSSocketMgr(this);

	this->getDispatcher()->registerHandler(nts::SOCKMSG_NETWORK_ERROR, this, &TimeService::handleMessage);
	this->getDispatcher()->registerHandler(nts::SOCKMSG_TIME_INFO, this, &TimeService::handleMessage);

	m_scheduler = Director::getInstance()->getScheduler();
	CC_SAFE_RETAIN(m_scheduler);
	m_scheduler->scheduleUpdate(this, UPDATE_PRIORITY_NTS, false);

	this->syncTime();
}

int64 TimeService::getCurrentTimeMillis()
{
	int64 currTime = 0;
	if (m_timeState == TIME_SYNCED)
	{
		auto diff = getBootTime() - m_lastUpdateTime;
		// CCLOG("TimeService: elapsedTime: " SI64FMTD, (int64)std::chrono::duration_cast<std::chrono::milliseconds>(diff).count());
		currTime = std::chrono::duration_cast<std::chrono::milliseconds>(m_hostLastTime.time_since_epoch() + diff).count();
	}

	return currTime;
}

void TimeService::addListener(TimeSyncListener* listener)
{
	auto it = std::find(m_listeners.begin(), m_listeners.end(), listener);
	if (it == std::end(m_listeners))
		m_listeners.push_back(listener);
}

void TimeService::removeListener(TimeSyncListener* listener)
{
	m_listeners.erase(std::remove(m_listeners.begin(), m_listeners.end(), listener), m_listeners.end());
}

void TimeService::update(float delta)
{
	m_dispatcher->dispatch();
}

TimeService::TimeService() :
	m_dispatcher(new MessageDispatcher()),
	m_scheduler(nullptr),
	m_socketMgr(nullptr),
	m_attempts(0),
	m_remoteLastTime(0),
	m_timeState(TIME_NONE)
{
}

TimeService::~TimeService()
{
	if (m_scheduler)
	{
		m_scheduler->unscheduleUpdate(this);
		CC_SAFE_RELEASE_NULL(m_scheduler);
	}

	CC_SAFE_DELETE(m_socketMgr);
	CC_SAFE_DELETE(m_dispatcher);
}

void TimeService::updateTime(TimeInfo const& info)
{
	m_remoteLastTime = time_util::getSystemTimeMillis() + info.offset;
	m_lastUpdateTime = getBootTime();
	auto now = SystemClock::now();
	auto remoteNow = now + std::chrono::milliseconds(info.offset);
	m_hostLastTime = remoteNow;
	CCLOG("TimeService: Local time is: %s", time_util::getDateTimeStr(now).c_str());
	CCLOG("TimeService: Remote time is: %s", time_util::getDateTimeStr(remoteNow).c_str());
	CCLOG("TimeService: Remote timestamp: " SI64FMTD "/" SI64FMTD "ms offset: " SI64FMTD "ms delay: %dms",
		m_remoteLastTime,
		(int64)(std::chrono::duration_cast<std::chrono::milliseconds>(remoteNow.time_since_epoch()).count()),
		info.offset, info.delay);

	m_timeState = TIME_SYNCED;
}


void TimeService::resetTime()
{
	m_timeState = TIME_NONE;
	m_hostLastTime = SystemClock::time_point(SystemClock::duration::zero());
	m_lastUpdateTime = std::chrono::time_point<std::chrono::nanoseconds>(std::chrono::nanoseconds::zero());
}

void TimeService::syncTime()
{
	if (m_timeState == TIME_SYNCING)
	{
		CCLOG("TimeService: Time synchronization is already in progress.");
		return;
	}

	CCLOG("TimeService: Start synchronizing time. maxAttempts: %d", MAX_ATTEMPTS);

	m_timeState = TIME_SYNCING;
	m_attempts = MAX_ATTEMPTS;
	m_socketMgr->start(NTS_SERVER_ADDR, NTS_SERVER_PORT);
}

void TimeService::onTimeSyncSuccess(TimeInfo const& info)
{
	m_socketMgr->stop();

	this->updateTime(info);
	int64 currTime = this->getCurrentTimeMillis();
	for (TimeSyncListener* listen : m_listeners)
		listen->onTimeSyncSuccess(currTime);
}

void TimeService::onTimeSyncFail(NetworkError const& error)
{
	m_socketMgr->stop();

	std::string description = error.getErrorString();
	sAnalytics->logException("nts", description.c_str());

	this->resetTime();
	for (TimeSyncListener* listen : m_listeners)
		listen->onTimeSyncFail();
}

std::chrono::time_point<std::chrono::nanoseconds> TimeService::getBootTime()
{
    using TimePoint = std::chrono::time_point<std::chrono::nanoseconds>;
    
#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
	timespec ts;
	if(clock_gettime(CLOCK_BOOTTIME, &ts) == 0)
    {
        return TimePoint(std::chrono::seconds(ts.tv_sec) + std::chrono::nanoseconds(ts.tv_nsec));
    }

#elif CC_TARGET_PLATFORM == CC_PLATFORM_IOS
    if (__builtin_available(iOS 10.0, *))
    {
        timespec ts;
        if(clock_gettime(CLOCK_MONOTONIC_RAW, &ts) == 0)
        {
            return TimePoint(std::chrono::seconds(ts.tv_sec) + std::chrono::nanoseconds(ts.tv_nsec));
        }
    }
    else
    {
        timeval tv;
        int32 mib[2]  = {CTL_KERN, KERN_BOOTTIME};
        size_t size = sizeof(tv);
        if(sysctl(mib, 2, &tv, &size, nullptr, 0) == 0)
        {
            auto diff = SystemClock::now().time_since_epoch() - (std::chrono::seconds(tv.tv_sec) + std::chrono::nanoseconds(tv.tv_usec));
            return TimePoint(diff);
        }
    }
    
#endif
	return TimePoint(SteadyClock::now().time_since_epoch());
}

void TimeService::handleMessage(Message const& message)
{
	switch (message.what())
	{
	case nts::SOCKMSG_NETWORK_ERROR:
	{
		NetworkError error;
		message.objectAs(error);
		CCLOG("TimeService: Time synchronization failed. error(%d): %s", error.getErrorCode(), error.getMessage().c_str());
		if (m_attempts > 0)
		{
			--m_attempts;
			CCLOG("TimeService: Try synchronizing time again (attempt %d of %d).", MAX_ATTEMPTS - m_attempts, MAX_ATTEMPTS);
			m_socketMgr->restart();
		}
		else
			this->onTimeSyncFail(error);
		break;
	}
	case nts::SOCKMSG_TIME_INFO:
	{
		TimeInfo info;
		message.objectAs(info);
		this->onTimeSyncSuccess(info);
		break;
	}
	default:
		break;
	}
}


NS_END
