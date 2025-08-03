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
    // 注意：当您在开发模式下调用此方法时，应用程序将始终显示评分和评论请求视图，以便您测试用户界面和体验。
    // 但是，如果您在使用 TestFlight 发布的应用程序中调用此方法，则不会产生任何效果。
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
