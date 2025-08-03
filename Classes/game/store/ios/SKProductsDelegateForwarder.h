//
//  SKProductsDelegateForwarder.h
//  snowfight
//
//  Created by Luthier on 2019/5/12.
//

#import <Foundation/Foundation.h>
#import <StoreKit/StoreKit.h>

#include "Store-ios.h"

USING_NS;

@interface SKProductsDelegateForwarder: NSObject <SKProductsRequestDelegate> {
    StoreImpl *m_handlerInstance;
}

- (id) initWithHandlerInstance:(StoreImpl*)handle;

- (void)validateProductIdentifiers:(NSArray *)productIdentifiers;
- (SKProduct *)productForId:(NSString *)productId;

// SKProductsRequestDelegate
- (void)requestDidFinish:(SKRequest *)request;
- (void)request:(SKRequest *)request didFailWithError:(NSError *)error;
- (void)productsRequest:(SKProductsRequest *)request didReceiveResponse:(SKProductsResponse *)response;

@end
