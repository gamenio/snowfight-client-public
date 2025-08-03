//
//  FirebaseHelper-android.cpp
//  snowfight
//
// Created by Luthier on 2020/8/14.
//

#include "FirebaseHelper.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)

#include "platform/android/jni/JniHelper.h"

USING_NS_CC;

NS_BEGIN

static const std::string firebaseHelperHelperClassName = "io/gamen/snowfight/FirebaseHelper";

std::string FirebaseHelper::getAdMobAppID()
{
    std::string appID = JniHelper::callStaticStringMethod(firebaseHelperHelperClassName, "getAdMobAppID");
    return appID;
}

firebase::gma::AdParent FirebaseHelper::getAdParent()
{
    return JniHelper::getActivity();
}

void FirebaseHelper::pauseGame()
{
}

void FirebaseHelper::resumeGame()
{
}

NS_END

#endif // CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
