//
//  FirebaseService.h
//  snowfight
//
// Created by Luthier on 2021/11/6.
//

#ifndef __FIREBASE_SERVICE_H__
#define __FIREBASE_SERVICE_H__

#include "common/Common.h"
#include "FirebaseServiceProtocol.h"
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
#include "FirebaseService-android.h"
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
#include "FirebaseService-ios.h"
#else
#include "FirebaseService-unknown.h"
#endif

NS_BEGIN

class FirebaseService: public FirebaseServiceProtocol
{
public:
    static FirebaseService* instance();

    void init() override;

    AdManagerImpl* getAdManager() const override;
    AnalyticsImpl* getAnalytics() const override;

private:
    FirebaseService();
    ~FirebaseService();

    FirebaseServiceImpl* m_impl;
};

#define sFirebaseService FirebaseService::instance()
#define sAdManager FirebaseService::instance()->getAdManager()
#define sAnalytics FirebaseService::instance()->getAnalytics()

NS_END

#endif // __FIREBASE_SERVICE_H__
