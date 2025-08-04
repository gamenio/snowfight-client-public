//
//  StoreError.h
//  snowfight
//
//  Created by Luthier on 2019/5/16.
//

#ifndef __STORE_ERROR_H__
#define __STORE_ERROR_H__

#include "common/Common.h"

USING_NS_CC;

NS_BEGIN

enum StoreErrorCode
{
    STORE_ERROR_NONE,
    STORE_ERROR_CLIENT_INVALID,                 // The client is not allowed to perform the attempted operation.
    STORE_ERROR_CANCELLED,                      // Indicates that the user canceled the operation of a payment request or other request.
    STORE_ERROR_STORE_PRODUCT_NOT_AVAILABLE,    // Indicates that the requested item is not available in the store.
    STORE_ERROR_UNKNOWN,                        // An unknown or unexpected error has occurred.
    STORE_ERROR_VALIDATION_FAILED,              // The receipt is invalid or the transaction did not pass validation.

#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS
    STORE_ERROR_URL_CANNOT_CONNECT_TO_HOST,     // Unable to connect to the host.
    STORE_ERROR_URL_TIMED_OUT,                  // Connection timeout.
    STORE_ERROR_URL_UNKNOWN,                    // The network connection failed for an unknown reason.
    STORE_ERROR_INVALID,                        // Indicates that the App Store cannot recognize one of the payment parameters. For example, an invalid product ID.
    STORE_ERROR_NOT_ALLOWED,                    // Indicates that the user is not allowed to authorize payments.

#elif CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
    STORE_ERROR_SERVICE_DISCONNECTED,           // The app is not connected to the Play Store service through the Google Play Billing Library.
    STORE_ERROR_SERVICE_UNAVAILABLE,            // The service is currently unavailable.
    STORE_ERROR_FEATURE_NOT_SUPPORTED,          // The requested feature is not supported by the Play Store on the current device.
    STORE_ERROR_ITEM_ALREADY_OWNED,             // Purchase failed because the item is already owned.
    STORE_ERROR_ITEM_NOT_OWNED,                 // Since the item does not own the user, the operation requested for the item fails.
    STORE_ERROR_NETWORK_ERROR,                  // A network error occurred during operation.

#endif
};

struct StoreError
{
    StoreError() :
    code(STORE_ERROR_NONE),
    description("")
    { }
    
    StoreErrorCode code;
    std::string description;
};

NS_END

#endif // __STORE_ERROR_H__
