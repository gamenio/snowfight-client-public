//
//  StoreUtil-ios.m
//  snowfight
//
//  Created by Luthier on 2019/5/16.
//

#import "StoreError-ios.h"


StoreError storeErrorWithNSError(NSError *error)
{
    StoreError storeError;
    
    if(error.description)
        storeError.description = error.description.UTF8String;
    
    if([error.domain isEqualToString:NSURLErrorDomain]) {
        switch(error.code) {
            case NSURLErrorTimedOut:
                storeError.code = STORE_ERROR_URL_TIMED_OUT;
                break;
            case NSURLErrorCannotFindHost:
            case NSURLErrorCannotConnectToHost:
            case NSURLErrorNetworkConnectionLost:
            case NSURLErrorNotConnectedToInternet:
                storeError.code = STORE_ERROR_URL_CANNOT_CONNECT_TO_HOST;
                break;
            case NSURLErrorUserCancelledAuthentication:
            case NSURLErrorSecureConnectionFailed:
            default:
                storeError.code = STORE_ERROR_URL_UNKNOWN;
                break;
        }
    } else if([error.domain isEqualToString:SKErrorDomain]) {
        switch(error.code) {
            case SKErrorClientInvalid:
                storeError.code = STORE_ERROR_CLIENT_INVALID;
                break;
            case SKErrorPaymentCancelled:
                storeError.code = STORE_ERROR_CANCELLED;
                break;
            case SKErrorPaymentInvalid:
                storeError.code = STORE_ERROR_INVALID;
                break;
            case SKErrorPaymentNotAllowed:
                storeError.code = STORE_ERROR_NOT_ALLOWED;
                break;
            default:
                storeError.code = STORE_ERROR_UNKNOWN;
                break;
        }
    } else if([error.domain isEqualToString: @"SSErrorDomain"]) {
        switch(error.code) {
            case 16:
                storeError.code = STORE_ERROR_CANCELLED;
                break;
            default:
                storeError.code = STORE_ERROR_UNKNOWN;
                break;
        }
    } else {
        storeError.code = STORE_ERROR_UNKNOWN;
    }
    
    return storeError;
}
