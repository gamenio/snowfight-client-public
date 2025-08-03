//
//  Analytics-mobile.cpp
//  snowfight
//
// Created by Luthier on 2021/11/6.
//

#include "Analytics-mobile.h"

#include "firebase/analytics.h"
#include "firebase/analytics/event_names.h"
#include "firebase/analytics/parameter_names.h"
#include "firebase/analytics/user_property_names.h"

USING_NS_CC;
namespace analytics = ::firebase::analytics;

NS_BEGIN

AnalyticsImpl::AnalyticsImpl()
{

}

AnalyticsImpl::~AnalyticsImpl()
{
    analytics::Terminate();
}

void AnalyticsImpl::init(firebase::App* app)
{
    analytics::Initialize(*app);
    analytics::SetAnalyticsCollectionEnabled(true);

    // App session times out after 30 minutes.
    // If the app is placed in the background and returns to the foreground after
    // the timeout is expired analytics will log a new session.
    analytics::SetSessionTimeoutDuration(1000 * 60 * 30);

    auto future_result = analytics::GetAnalyticsInstanceId();
    analytics::GetAnalyticsInstanceIdLastResult().OnCompletion([](firebase::Future<std::string> const& result) {
        if (result.status() == firebase::kFutureStatusComplete)
            CCLOG("AnalyticsImpl: Analytics Instance ID %s", result.result()->c_str());
        else
            CCLOG("AnalyticsImpl: Failed to fetch Analytics Instance ID %s (%d)", result.error_message(), result.error());
    });

}

void AnalyticsImpl::logSelectContent(char const* contentType, char const* itemId)
{
    const analytics::Parameter kSelectContentParameters[] = {
            analytics::Parameter(analytics::kParameterItemID, itemId),
            analytics::Parameter(analytics::kParameterContentType, contentType),
    };
    analytics::LogEvent(
            analytics::kEventSelectContent, kSelectContentParameters,
            sizeof(kSelectContentParameters) /
            sizeof(kSelectContentParameters[0]));
}

void AnalyticsImpl::logTutorialBegin()
{
    analytics::LogEvent(analytics::kEventTutorialBegin);
}

void AnalyticsImpl::logTutorialComplete()
{
    analytics::LogEvent(analytics::kEventTutorialComplete);
}

void AnalyticsImpl::logException(char const* itemId, char const* description)
{
    const analytics::Parameter kExceptionParameters[] = {
            analytics::Parameter(analytics::kParameterItemID, itemId),
            analytics::Parameter("description", description),
    };
    analytics::LogEvent(
            "exception", kExceptionParameters,
            sizeof(kExceptionParameters) /
            sizeof(kExceptionParameters[0]));
}

void AnalyticsImpl::logLogin()
{
    analytics::LogEvent(analytics::kEventLogin);
}

void AnalyticsImpl::logReview(char const* itemId)
{
    analytics::LogEvent("review", analytics::kParameterItemID, itemId);
}

void AnalyticsImpl::setCurrentScreen(char const* screenClass)
{
    analytics::LogEvent(analytics::kEventScreenView, analytics::kParameterScreenClass, screenClass);
}

void AnalyticsImpl::setCurrentScreen(char const* screenName, char const* screenClass)
{
    const analytics::Parameter kScreenViewParameters[] = {
            analytics::Parameter(analytics::kParameterScreenName, screenName),
            analytics::Parameter(analytics::kParameterScreenClass, screenClass),
    };
    analytics::LogEvent(
            analytics::kEventScreenView, kScreenViewParameters,
            sizeof(kScreenViewParameters) /
            sizeof(kScreenViewParameters[0]));
}

void AnalyticsImpl::setUserId(char const* userId)
{
    analytics::SetUserId(userId);
}

void AnalyticsImpl::setSignUpMethod(const char* method)
{
    analytics::SetUserProperty(analytics::kUserPropertySignUpMethod, method);
}


NS_END
