//
//  SKPaymentDelegateForwarder.h
//  snowfight
//
//  Created by Luthier on 2019/5/12.
//

#import <Foundation/Foundation.h>
#import <StoreKit/StoreKit.h>

#include "Store-ios.h"


USING_NS;

@interface SKPaymentDelegateForwarder: NSObject <SKRequestDelegate, SKPaymentTransactionObserver> {
    StoreImpl *m_handlerInstance;
}

- (id) initWithHandlerInstance:(StoreImpl*)handle;

- (void)finishTransaction:(NSString *)transactionId;
- (void)refreshReceipt;

// SKPaymentTransactionObserver
- (void)paymentQueue:(SKPaymentQueue *)queue updatedTransactions:(NSArray<SKPaymentTransaction *> *)transactions;
- (void)paymentQueue:(SKPaymentQueue *)queue removedTransactions:(NSArray<SKPaymentTransaction *> *)transactions;
- (void)paymentQueueRestoreCompletedTransactionsFinished:(SKPaymentQueue *)queue;
- (void)paymentQueue:(SKPaymentQueue *)queue restoreCompletedTransactionsFailedWithError:(NSError *)error;
@end
