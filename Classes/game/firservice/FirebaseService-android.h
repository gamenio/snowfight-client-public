//
//  FirebaseService-android.h
//  snowfight
//
// Created by Luthier on 2021/11/6.
//

#ifndef __FIREBASE_SERVICE_ANDROID_H__
#define __FIREBASE_SERVICE_ANDROID_H__

#include "firebase/app.h"

#include "common/Common.h"
#include "FirebaseServiceProtocol.h"
#include "AdManager-mobile.h"
#include "Analytics-mobile.h"

NS_BEGIN

class FirebaseServiceImpl: public FirebaseServiceProtocol
{
public:
    FirebaseServiceImpl();
    ~FirebaseServiceImpl();

    void init() override;

    AdManagerImpl* getAdManager() const override { return m_adManager; }
    AnalyticsImpl* getAnalytics() const override { return  m_analytics; }

private:
    firebase::App* m_app;
    AdManagerImpl* m_adManager;
    AnalyticsImpl* m_analytics;
};

NS_END

#endif // __FIREBASE_SERVICE_ANDROID_H__
