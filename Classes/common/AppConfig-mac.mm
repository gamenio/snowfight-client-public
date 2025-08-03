#include "AppConfig.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_MAC)

NS_BEGIN

int32 getBuildNumber()
{
    NSDictionary *info = [[NSBundle mainBundle] infoDictionary];
    NSString *version = [info objectForKey:@"CFBundleVersion"];
    if(version) {
        int32 value = [version intValue];
        return value;
    }
    return 0;
}



NS_END

#endif // CC_TARGET_PLATFORM == CC_PLATFORM_MAC
