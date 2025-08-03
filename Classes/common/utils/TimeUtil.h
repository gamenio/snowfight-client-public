#ifndef __TIME_UTIL_H__
#define __TIME_UTIL_H__

#include <chrono>
#include <iomanip>

#include "common/Common.h"

NS_BEGIN

namespace time_util {


static std::chrono::system_clock::time_point sApplicationStartTime = std::chrono::system_clock::now();

inline NSTime getUptimeMillis()
{
	using namespace std::chrono;

	NSTime time = NSTime(duration_cast<milliseconds>(system_clock::now() - sApplicationStartTime).count());
	return time;
}

inline int64 getSystemTimeMillis()
{
	using namespace std::chrono;

	int64 time = int64(duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
	return time;
}

inline double getHighResolutionTimeMillis()
{
	using namespace std::chrono;

	double msDouble = duration_cast<duration<double, std::milli>>(high_resolution_clock::now().time_since_epoch()).count();
	return msDouble;
}

inline float toGameTimeSeconds(NSTime millis)
{
	return (float)millis / 1000.0f;
}

inline NSTime toServerTimeMillis(float seconds)
{
	return static_cast<NSTime>(seconds * 1000.0f);
}

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) || (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT)
struct tm* localtime_r(const time_t* time, struct tm *result);
#endif

inline std::string getDateTimeStr(std::chrono::system_clock::time_point const& time)
{
	auto seconds = std::chrono::time_point_cast<std::chrono::seconds>(time);
	auto fraction = time - seconds;
	auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(fraction);
	auto tt = std::chrono::system_clock::to_time_t(time);

	tm aTm;
	localtime_r(&tt, &aTm);
	char buff[24];
	snprintf(buff, sizeof(buff),
		"%04d/%02d/%02d %02d:%02d:%02d.%03d",
		aTm.tm_year + 1900,
		aTm.tm_mon + 1,
		aTm.tm_mday,
		aTm.tm_hour,
		aTm.tm_min,
		aTm.tm_sec,
		(int)milliseconds.count());

	return std::string(buff);
}


inline std::string getNowTimeStr()
{
	return getDateTimeStr(std::chrono::system_clock::now());
}



} //namespace time_util


NS_END


#endif //__TIME_UTIL_H__



