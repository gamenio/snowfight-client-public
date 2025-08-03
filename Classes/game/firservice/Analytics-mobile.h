//
//  Analytics-mobile.h
//  snowfight
//
// Created by Luthier on 2021/11/6.
//

#ifndef __ANALYTICS_MOBILE_H__
#define __ANALYTICS_MOBILE_H__

#include "firebase/app.h"

#include "common/Common.h"
#include "AnalyticsProtocol.h"

NS_BEGIN

class AnalyticsImpl: public AnalyticsProtocol
{
public:
    AnalyticsImpl();
    ~AnalyticsImpl();

    void init(firebase::App* app);

    void logSelectContent(char const* contentType, char const* itemId) override;
    void logTutorialBegin() override;
    void logTutorialComplete() override;
    void logException(char const* itemId, char const* description) override;
    void logLogin() override;
    void logReview(char const* itemId) override;

    void setCurrentScreen(char const* screenClass) override;
    void setCurrentScreen(char const* screenName, char const* screenClass) override;
    void setUserId(const char* userId) override;
    void setSignUpMethod(const char* method) override;
};

NS_END

#endif // __ANALYTICS_MOBILE_H__
