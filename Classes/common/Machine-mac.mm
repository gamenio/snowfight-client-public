#include "Machine.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_MAC)

#include <sys/sysctl.h>

NS_BEGIN

std::string Machine::getModel() const
{
    std::string model;
    size_t len = 0;
    sysctlbyname("hw.model", NULL, &len, NULL, 0);
    if (len) {
        char* buf = (char*)malloc(len * sizeof(char));
        sysctlbyname("hw.model", buf, &len, NULL, 0);
        model = buf;
        free(buf);
    }
    return model;
    
}

std::string Machine::getOS() const
{
    NSDictionary *version = [NSDictionary dictionaryWithContentsOfFile:@"/System/Library/CoreServices/SystemVersion.plist"];
    NSString *productVersion = [version objectForKey:@"ProductVersion"];
    NSString* os = [NSString stringWithFormat:@"macOS %@", productVersion];
    return [os cStringUsingEncoding: NSASCIIStringEncoding];
}

std::string Machine::getDeviceGUID() const
{
    NSUUID* uuid = nil;
    uuid_t hostuuid = {};
    struct timespec timeout = {1, 0}; /* 1 sec timeout */
    if(gethostuuid(hostuuid, &timeout) == 0)
        uuid = [[NSUUID alloc] initWithUUIDBytes:hostuuid];
    else
        uuid = [[NSUUID alloc] init];
    
    NSString* uuidString = [uuid UUIDString];
    [uuid release];
    
    return uuidString.UTF8String;
}

Rect Machine::getSafeAreaRect() const
{
    return Director::getInstance()->getSafeAreaRect();
}

bool Machine::isTablet() const
{
    return false;
}

void Machine::setMultipleTouchEnabled(bool enabled)
{
}

bool Machine::isAppInstalled(std::string const& packageName) const
{
    return false;
}

void Machine::vibrate(VibrationType type)
{
}

NS_END

#endif // CC_TARGET_PLATFORM == CC_PLATFORM_IOS
