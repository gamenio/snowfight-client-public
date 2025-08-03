//
//  SKPaymentDelegateForwarder.mm
//  snowfight
//
//  Created by Luthier on 2019/5/12.
//

#import "SKPaymentDelegateForwarder.h"

#import "StoreError-ios.h"

#include "../StoreProtocol.h"

@implementation SKPaymentDelegateForwarder

- (id) initWithHandlerInstance:(StoreImpl*)handle
{
    self = [super init];
    if (self) {
        m_handlerInstance = handle;
    }
    return self;
}


- (void)finishTransaction:(NSString *)transactionId
{
    NSArray* transactions = [[SKPaymentQueue defaultQueue] transactions];
    for (SKPaymentTransaction *transaction in transactions) {
        if([transaction.transactionIdentifier isEqualToString:transactionId])
           [[SKPaymentQueue defaultQueue] finishTransaction:transaction];
    }
}

- (void)refreshReceipt
{
    SKReceiptRefreshRequest *request = [[SKReceiptRefreshRequest alloc] init];
    
    request.delegate = self;
    [request start];
}

- (void)requestDidFinish:(SKRequest *)request
{
    [request autorelease];
    
    cocos2d::Director::getInstance()->getScheduler()->performFunctionInCocosThread([self](){
        if(m_handlerInstance)
            m_handlerInstance->receiptRequestDidFinish();
    });
}

- (void)request:(SKRequest *)request didFailWithError:(NSError *)error
{
    [request autorelease];
    
    StoreError storeError = storeErrorWithNSError(error);
    cocos2d::Director::getInstance()->getScheduler()->performFunctionInCocosThread([self, storeError](){
        if(m_handlerInstance)
            m_handlerInstance->receiptRequestDidFail(storeError);
    });
}

- (void)paymentQueue:(SKPaymentQueue *)queue updatedTransactions:(NSArray<SKPaymentTransaction *> *)transactions
{
    std::vector<PaymentTransaction> payments;
    
    for (SKPaymentTransaction *transaction in transactions) {
        PaymentTransaction payment;
        if(transaction.transactionIdentifier)
            payment.transactionId = transaction.transactionIdentifier.UTF8String;
        if(transaction.payment && transaction.payment.productIdentifier)
            payment.productId = transaction.payment.productIdentifier.UTF8String;
        if(transaction.transactionDate)
            payment.transactionTime = uint32(transaction.transactionDate.timeIntervalSince1970);
        
        switch (transaction.transactionState) {
            case SKPaymentTransactionStatePurchasing:
                payment.state = PAYMENT_STATE_PURCHASING;
                break;
            case SKPaymentTransactionStateDeferred:
                payment.state = PAYMENT_STATE_DEFERRED;
                break;
            case SKPaymentTransactionStateFailed:
            {
                payment.state = PAYMENT_STATE_FAILED;
                NSError *error = transaction.error;
                if(error) {
                    NSLog(@"Failed transaction. TransactionID: %@ ProductID: %@ Error(Code:%d): %@",
                          transaction.transactionIdentifier,
                          transaction.payment.productIdentifier,
                          (int)error.code,
                          error.description);
                    payment.error = storeErrorWithNSError(error);
                }
                else
                    payment.error.code = STORE_ERROR_UNKNOWN;
                break;
            }
            case SKPaymentTransactionStatePurchased:
                payment.state = PAYMENT_STATE_PURCHASED;
                break;
            case SKPaymentTransactionStateRestored:
                payment.state = PAYMENT_STATE_RESTORED;
                break;
        }
        
        payments.emplace_back(payment);
    }
    
    cocos2d::Director::getInstance()->getScheduler()->performFunctionInCocosThread([self, payments]() mutable {
        if(m_handlerInstance)
            m_handlerInstance->updatedTransactions(payments);
    });

}

- (void)paymentQueue:(SKPaymentQueue *)queue removedTransactions:(NSArray<SKPaymentTransaction *> *)transactions
{
    
}

- (void)paymentQueueRestoreCompletedTransactionsFinished:(SKPaymentQueue *)queue
{

}

- (void)paymentQueue:(SKPaymentQueue *)queue restoreCompletedTransactionsFailedWithError:(NSError *)error
{

}

- (void)dealloc
{
    m_handlerInstance = nil;
    
    [super dealloc];
}

@end
