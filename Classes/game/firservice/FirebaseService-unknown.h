//
//  FirebaseService-unknown.h
//  snowfight
//
// Created by Luthier on 2021/11/6.
//

#ifndef __FIREBASE_SERVICE_UNKNOWN_H__
#define __FIREBASE_SERVICE_UNKNOWN_H__

#include "common/Common.h"
#include "FirebaseServiceProtocol.h"
#include "AdManager-unknown.h"
#include "Analytics-unknown.h"

NS_BEGIN

class FirebaseServiceImpl: public FirebaseServiceProtocol
{
public:
    FirebaseServiceImpl() :
            m_adManager(nullptr)
    {
        m_adManager = new AdManagerImpl();
		m_analytics = new AnalyticsImpl();
    }

   ~FirebaseServiceImpl()
    {
        CC_SAFE_DELETE(m_adManager);
        CC_SAFE_DELETE(m_analytics);
    }

    void init() override {}

    AdManagerImpl* getAdManager() const override { return m_adManager; }
    AnalyticsImpl* getAnalytics() const override { return m_analytics; }

private:
    AdManagerImpl* m_adManager;
    AnalyticsImpl* m_analytics;
};


NS_END

#endif // __FIREBASE_SERVICE_UNKNOWN_H__
