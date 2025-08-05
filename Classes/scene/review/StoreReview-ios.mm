//
//  StoreReview-ios.mm
//  snowfight
//
//  Created by Luthier on 2020/7/18.
//

#include "StoreReview-ios.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)

#import <StoreKit/StoreKit.h>

#include "game/GameConfig.h"

NS_BEGIN

StoreReviewImpl::StoreReviewImpl()
{
    
}

StoreReviewImpl::~StoreReviewImpl()
{
    
}

bool StoreReviewImpl::isNativeReviewViewSupported()
{
    if (@available(iOS 10.3, *)) {
        return true;
    } else {
        return false;
    }
}

void StoreReviewImpl::openNativeReviewView()
{
	// When you call this method while your app is in development mode, a rating and review request view is always displayed so you can test the user interface and experience. 
	// However, this method has no effect when you call it in an app that you distribute using TestFlight.
    if (@available(iOS 10.3, *)) {
        [SKStoreReviewController requestReview];
    }
}

void StoreReviewImpl::openExternalReviewView()
{
    NSString  *urlStr = [NSString stringWithCString: STORE_REVIEW_URL_MARKET encoding:NSUTF8StringEncoding];
    NSURL *writeReviewURL = [NSURL URLWithString:urlStr];
    [[UIApplication sharedApplication] openURL: writeReviewURL options: @{} completionHandler: nil];
}

NS_END

#endif // CC_TARGET_PLATFORM == CC_PLATFORM_IOS
