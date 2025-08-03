//
//  FirebaseServiceProtocol.h
//  snowfight
//
// Created by Luthier on 2021/11/6.
//

#ifndef __FIREBASE_SERVICE_PROTOCOL_H__
#define __FIREBASE_SERVICE_PROTOCOL_H__

#include "common/Common.h"

NS_BEGIN

class AdManagerImpl;
class AnalyticsImpl;

class FirebaseServiceProtocol
{
public:
    virtual ~FirebaseServiceProtocol() { }

    virtual void init() = 0;

    virtual AdManagerImpl* getAdManager() const = 0;
    virtual AnalyticsImpl* getAnalytics() const = 0;
};

NS_END

#endif // __FIREBASE_SERVICE_PROTOCOL_H__
