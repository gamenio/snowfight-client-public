#include "ConnectivityHelper.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)

#include "platform/android/jni/JniHelper.h"

USING_NS_CC;

NS_BEGIN

static const std::string helperClassName = "io/gamen/snowfight/ConnectivityHelper";

NetworkType ConnectivityHelper::getNetworkType()
{
    NetworkType networkType = (NetworkType)JniHelper::callStaticIntMethod(helperClassName, "getNetworkClass");
	return networkType;
}

NS_END

#endif // CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID