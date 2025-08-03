//
//  FirebaseService-ios.cpp
//  snowfight
//
// Created by Luthier on 2021/11/11.
//

#include "FirebaseService-ios.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)

USING_NS_CC;

NS_BEGIN

void FirebaseServiceImpl::init()
{
    NS_ASSERT_LOG(m_app == nullptr, "Firebase app has been created.");
    m_app = firebase::App::Create(firebase::AppOptions());

    m_adManager = new AdManagerImpl();
    m_adManager->init(m_app);

    m_analytics = new AnalyticsImpl();
    m_analytics->init(m_app);
}

FirebaseServiceImpl::FirebaseServiceImpl() :
        m_app(nullptr),
        m_adManager(nullptr),
        m_analytics(nullptr)
{
}

FirebaseServiceImpl::~FirebaseServiceImpl()
{
    CC_SAFE_DELETE(m_adManager);
    CC_SAFE_DELETE(m_analytics);
    CC_SAFE_DELETE(m_app);
}


NS_END

#endif // (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
