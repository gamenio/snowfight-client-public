#include "AppConfig.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)

#include "platform/android/jni/JniHelper.h"

NS_BEGIN

static const std::string helperClassName = "io/gamen/snowfight/AppHelper";

int32 getBuildNumber()
{
    int32 versionCode =  cocos2d::JniHelper::callStaticIntMethod(helperClassName, "getVersionCode");
    return versionCode;
}

NS_END

#endif // CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID