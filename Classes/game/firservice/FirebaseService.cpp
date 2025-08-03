//
//  FirebaseService.cpp
//  snowfight
//
// Created by Luthier on 2021/11/6.
//

#include "FirebaseService.h"

NS_BEGIN

FirebaseService* FirebaseService::instance()
{
    static FirebaseService instance;
    return &instance;
}


void FirebaseService::init()
{
    if(m_impl)
        m_impl->init();
}

AdManagerImpl* FirebaseService::getAdManager() const
{
    if(m_impl)
        return m_impl->getAdManager();

    return nullptr;
}

AnalyticsImpl* FirebaseService::getAnalytics() const
{
    if(m_impl)
        return m_impl->getAnalytics();

    return nullptr;
}


FirebaseService::FirebaseService() :
    m_impl(nullptr)
{
    m_impl = new FirebaseServiceImpl();
}

FirebaseService::~FirebaseService()
{
    CC_SAFE_DELETE(m_impl);
}

NS_END