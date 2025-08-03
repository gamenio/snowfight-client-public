//
//  FirebaseHelper-ios.cpp
//  snowfight
//
// Created by Luthier on 2020/8/14.
//

#include "FirebaseHelper.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)

#include "audio/include/AudioEngine.h"

#import <AppTrackingTransparency/AppTrackingTransparency.h>
#import <AdSupport/AdSupport.h>

USING_NS_CC;

NS_BEGIN


std::string FirebaseHelper::getAdMobAppID()
{
    NSString *appID = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"GADApplicationIdentifier"];
    return [appID cStringUsingEncoding: NSASCIIStringEncoding];
}

firebase::gma::AdParent FirebaseHelper::getAdParent()
{
    return (id)Director::getInstance()->getOpenGLView()->getEAGLView();
}

void FirebaseHelper::pauseGame()
{
    if(!Director::getInstance()->isPaused())
        Director::getInstance()->pause();

    experimental::AudioEngine::pauseAll();
}

void FirebaseHelper::resumeGame()
{
    if (Director::getInstance()->isPaused())
        Director::getInstance()->resume();

    experimental::AudioEngine::resumeAll();

}

bool FirebaseHelper::isNeedTrackingAuthorization()
{
    if (@available(iOS 14, *))
    {
        if(ATTrackingManager.trackingAuthorizationStatus == ATTrackingManagerAuthorizationStatusNotDetermined)
        {
            return true;
        }
    }
    
    return false;
}

void FirebaseHelper::requestIDFA(std::function<void(bool)> const& callback)
{
    if (@available(iOS 14, *))
    {
        std::function<void(bool)> callbackCopy = callback;
        [ATTrackingManager requestTrackingAuthorizationWithCompletionHandler:^(ATTrackingManagerAuthorizationStatus status) {
            bool success = false;
            switch(status)
            {
                case ATTrackingManagerAuthorizationStatusAuthorized:
                    success = true;
                    break;
                case ATTrackingManagerAuthorizationStatusDenied:
                case ATTrackingManagerAuthorizationStatusNotDetermined:
                case ATTrackingManagerAuthorizationStatusRestricted:
                    success = false;
                    break;
            }
            
            if(callbackCopy)
                callbackCopy(success);
        }];
    }
}

NS_END

#endif // CC_TARGET_PLATFORM == CC_PLATFORM_IOS
