#include "AppConfig.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)

NS_BEGIN

int32 getBuildNumber()
{
    return 0;
}


NS_END

#endif // CC_TARGET_PLATFORM == CC_PLATFORM_WIN32