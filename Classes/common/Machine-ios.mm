#include "Machine.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)

#import <sys/utsname.h>
#include "platform/ios/CCEAGLView-ios.h"

NS_BEGIN

std::string Machine::getModel() const
{
    struct utsname systemInfo;
    uname(&systemInfo);
    
    return systemInfo.machine;

}

std::string Machine::getOS() const
{
    NSString* sysName = [[UIDevice currentDevice] systemName];
    NSString* sysVer = [[UIDevice currentDevice] systemVersion];
    NSString* os = [NSString stringWithFormat:@"%@ %@", sysName, sysVer];
    return [os cStringUsingEncoding: NSASCIIStringEncoding];
}

std::string Machine::getDeviceGUID() const
{
    UIDevice *device = [UIDevice currentDevice];
    NSUUID *identifier = [device identifierForVendor];
    NSString* uuidString = identifier.UUIDString;
    return uuidString.UTF8String;
}

cocos2d::Rect Machine::getSafeAreaRect() const
{
    return Director::getInstance()->getSafeAreaRect();
}

bool Machine::isTablet() const
{
	if (Application::getInstance()->getTargetPlatform() == ApplicationProtocol::Platform::OS_IPAD)
		return true;
	else
		return false;
}

void Machine::setMultipleTouchEnabled(bool enabled)
{
    GLView* view = Director::getInstance()->getOpenGLView();
    if(view)
    {
        CCEAGLView *eaglView = (CCEAGLView *)view->getEAGLView();
        [eaglView setMultipleTouchEnabled: enabled];
    }
}

bool Machine::isAppInstalled(std::string const& packageName) const
{
    return false;
}

void Machine::vibrate(VibrationType type)
{
    if (@available(iOS 10.0, *)) {
        UIImpactFeedbackStyle feedbackStyle;
        switch(type)
        {
            case VIBRATION_TYPE_TOUCH_HOLD:
                feedbackStyle = UIImpactFeedbackStyleMedium;
                break;
            default:
                feedbackStyle = UIImpactFeedbackStyleLight;
                break;
        }
        UIImpactFeedbackGenerator *generator = [[UIImpactFeedbackGenerator alloc] initWithStyle: feedbackStyle];
        [generator impactOccurred];
        [generator release];
    }
}

NS_END

#endif // CC_TARGET_PLATFORM == CC_PLATFORM_IOS

