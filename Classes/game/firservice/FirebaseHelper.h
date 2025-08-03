//
//  FirebaseHelper.h
//  snowfight
//
// Created by Luthier on 2020/8/14.
//
#ifndef __FIREBASE_HELPER_H__
#define __FIREBASE_HELPER_H__

#include "firebase/gma/types.h"

#include "common/Common.h"

NS_BEGIN

class FirebaseHelper
{
public:
    static std::string getAdMobAppID();
    static firebase::gma::AdParent getAdParent();

    static void pauseGame();
    static void resumeGame();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    static bool isNeedTrackingAuthorization();
    static void requestIDFA(std::function<void(bool)> const& callback);
#endif
private:
    FirebaseHelper() {}
    ~FirebaseHelper() {}
};

NS_END

#endif // __FIREBASE_HELPER_H__
