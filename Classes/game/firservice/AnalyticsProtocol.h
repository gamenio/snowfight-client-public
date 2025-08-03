//
//  AnalyticsProtocol.h
//  snowfight
//
// Created by Luthier on 2021/11/6.
//

#ifndef __ANALYTICS_PROTOCOL_H__
#define __ANALYTICS_PROTOCOL_H__

#include "common/Common.h"

NS_BEGIN

// kEventSelectContent parameters
static const char *const kContentTypeButton = "button";

// kUserPropertySignUpMethod parameters
static const char *const kSignUpMethodNone = "None";
static const char *const kSignUpMethodGoogle = "Google";
static const char *const kSignUpMethodApple = "Apple";

class AnalyticsProtocol
{
public:
    virtual ~AnalyticsProtocol() { }

    virtual void logSelectContent(char const* contentType, char const* itemId) {}
    virtual void logTutorialBegin(){}
    virtual void logTutorialComplete(){}
    virtual void logException(char const* itemId, char const* description){}
    virtual void logLogin(){}
    virtual void logReview(char const* itemId){}

    virtual void setCurrentScreen(char const* screenClass) {}
    virtual void setCurrentScreen(char const* screenName, char const* screenClass) {}
    virtual void setUserId(const char* userId) {}
    virtual void setSignUpMethod(const char* method) {}
};


NS_END

#endif // __ANALYTICS_PROTOCOL_H__
