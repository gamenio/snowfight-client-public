//
// Created by Luthier on 2019/9/19.
//

#include "GoogleServicesUtils.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)

#include "platform/android/jni/JniHelper.h"

NS_BEGIN

static const std::string googleServicesHelperClassName = "io/gamen/snowfight/GoogleServicesHelper";

int32 GoogleServicesUtils::checkGooglePlayServices()
{
    return JniHelper::callStaticIntMethod(googleServicesHelperClassName, "checkGooglePlayServices");
}

void GoogleServicesUtils::showErrorDialog(int32 errorCode)
{
    JniHelper::callStaticVoidMethod(googleServicesHelperClassName, "showErrorDialog", errorCode);
}

NS_END

#endif // CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID