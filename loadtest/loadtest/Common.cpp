#include "Common.h"

#include "common/utils/TimeUtil.h"

USING_NS;

void testlog(const char* format, ...)
{
	va_list args;
	char buffer[MAX_LOG_LENGTH];

	va_start(args, format);
	int nret = vsnprintf(buffer, MAX_LOG_LENGTH, format, args);
	va_end(args);

	NS_ASSERT(nret >= 0 && nret < MAX_LOG_LENGTH);
	cocos2d::log("%s  %s", time_util::getNowTimeStr().c_str(), buffer);
}
