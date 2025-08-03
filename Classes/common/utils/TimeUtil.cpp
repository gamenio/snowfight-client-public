#include "TimeUtil.h"

NS_BEGIN

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) || (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT)
struct tm* time_util::localtime_r(const time_t* time, struct tm *result)
{
	localtime_s(result, time);
	return result;
}
#endif



NS_END