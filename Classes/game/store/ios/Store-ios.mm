//
//  Store-ios.cpp
//  snowfight
//
//  Created by Luthier on 2019/5/8.
//

#include "Store-ios.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)

#import <StoreKit/StoreKit.h>

#import "ValidateReceipt.h"
#import "SKProductsDelegateForwarder.h"
#import "SKPaymentDelegateForwarder.h"

#include "common/utils/TimeUtil.h"
#include "../../UserPreferences.h"

const NSString * global_bundleVersion = @"22";
const NSString * global_bundleIdentifier = @"io.gamen.snowfight";

NS_BEGIN

// APPLE: The parent has 24 hours to approve or cancel their child's purchase after the Ask to Buy process has begun.
// If the parent fails to respond within the 24 hours, the Ask to Buy request is deleted from iTunes Store servers and
// your app's observer does not receive any notifications.
#define DEFERRED_TRANSACTION_TIMEOUT_IN_SEC    86400

#if NS_DEBUG

#if defined(__IPHONE_8_3) && (__IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_8_3)
#define SIMULATES_ASK_TO_BUY_IN_SANDBOX      0
#endif

void validateReceiptWithAppStore(NSData *receipt)
{
    NSError *error;
    NSDictionary *requestContents = @{
                                      @"receipt-data": [receipt base64EncodedStringWithOptions:0]
                                      };
    NSData *requestData = [NSJSONSerialization dataWithJSONObject:requestContents
                                                          options:0
                                                            error:&error];
    if (!requestData) {
        NSLog(@"Request receipt data invalid.");
        return;
    }
    // Create a POST request with the receipt data.
    NSURL *storeURL = [NSURL URLWithString:@"https://sandbox.itunes.apple.com/verifyReceipt"];
    NSMutableURLRequest *storeRequest = [NSMutableURLRequest requestWithURL:storeURL];
    [storeRequest setHTTPMethod:@"POST"];
    [storeRequest setHTTPBody:requestData];
    // Make a connection to the iTunes Store.
    NSURLSession *session = [NSURLSession sharedSession];
    [[session dataTaskWithRequest: storeRequest completionHandler:^(NSData * _Nullable data, NSURLResponse * _Nullable response, NSError * _Nullable error) {
        if(error) {
            NSLog(@"AppStore: %@", error);
        } else {
            NSError *error;
            NSDictionary *jsonResponse = [NSJSONSerialization JSONObjectWithData:data options:0 error:&error];
            if (jsonResponse) {
                NSLog(@"%@", jsonResponse);
            } else {
                NSLog(@"AppStore: %@", error);
            }
        }
    }] resume];
}

#endif // NS_DEBUG

NSData* getReceiptData()
{
    NSBundle *mainBundle = [NSBundle mainBundle];
    NSURL *receiptURL = [mainBundle appStoreReceiptURL];
    NSError *receiptError;
    BOOL isPresent = [receiptURL checkResourceIsReachableAndReturnError:&receiptError];
    if (!isPresent) {
        return nullptr;
    }

    NSData *receiptData = [NSData dataWithContentsOfURL:receiptURL];
    return receiptData;
    
}


void restorePaidTransactions(std::vector<StoreListener*> const& listeners, NSDictionary *receipt)
{
    // 票据里将包含消耗型、非消耗型、自动续期订阅、非自动续期订阅或免费订阅的内购项目收据
    // 其中消耗型项目收据在App完成该笔交易之前一直保留在票据内。在交易完成后，该收据会在下次票据更新时从票据中移除。
    NSArray *purchases = [receipt objectForKey:kReceiptInApp];
    if(purchases && [purchases isKindOfClass:[NSArray class]])
    {
        for(NSDictionary *purchase in purchases)
        {
            NSString *ssProductId = (NSString *)[purchase objectForKey: kReceiptInAppProductIdentifier];
            NSString *ssTransactionId = (NSString *)[purchase objectForKey: kReceiptInAppTransactionIdentifier];
            std::string productId = ssProductId.UTF8String;
            std::string transactionId = ssTransactionId.UTF8String;
            for(StoreListener* listen: listeners)
            {
                PaymentTransaction transaction;
                transaction.productId = productId;
                transaction.transactionId = transactionId;
                transaction.state = PAYMENT_STATE_RESTORED;
                listen->onPaymentTransactionUpdated(transaction);
            }
        }
    }
    
    for(StoreListener* listen: listeners)
        listen->onRestoreCompletedTransactionsFinished();
}

std::vector<std::string> validatePurchasedProductIdsWithReceipt(NSDictionary *receipt, std::vector<std::string> const& productIds)
{
    std::vector<std::string> purchasedProductIds;
    
    NSArray *purchases = obtainInAppPurchasesWithReceipt(receipt);
    for(NSDictionary *purchase in purchases)
    {
        for(auto const& productId: productIds)
        {
            NSString *ssProductId = [NSString stringWithUTF8String: productId.c_str()];
            NSString *ssId = (NSString *)[purchase objectForKey: kReceiptInAppProductIdentifier];
            if([ssId isEqualToString:ssProductId])
                purchasedProductIds.push_back(productId);
        }
    }
    return purchasedProductIds;
}

StoreImpl::StoreImpl() :
    m_skProductsDelegateForwarder(nullptr),
    m_skPaymentDelegateForwarder(nullptr),
    m_isValidatingProductIds(false),
    m_isProductIdsValidated(false),
    m_isRefreshingReceipt(false),
    m_isRestoringPaidTransactions(false),
    m_isTransactionObserverRegistered(false),
    m_hasReceiptValid(false),
    m_deferredPaymentStorage(new PaymentStorage)
{
    m_skProductsDelegateForwarder = [[SKProductsDelegateForwarder alloc] initWithHandlerInstance:this];
    m_skPaymentDelegateForwarder = [[SKPaymentDelegateForwarder alloc] initWithHandlerInstance:this];
}

StoreImpl::~StoreImpl()
{
    CC_SAFE_DELETE(m_deferredPaymentStorage);

    for(auto it = m_products.begin(); it != m_products.end();)
    {
        ProductInfo* product = *it;
        it = m_products.erase(it);
        CC_SAFE_DELETE(product);
    }
    
    m_listeners.clear();
    this->unregisterTransactionObserver();
    
    if(m_skProductsDelegateForwarder)
    {
        [(id)m_skProductsDelegateForwarder release];
        m_skProductsDelegateForwarder = nullptr;
    }
    
    if(m_skPaymentDelegateForwarder)
    {
        [(id)m_skPaymentDelegateForwarder release];
        m_skPaymentDelegateForwarder = nullptr;
    }
}

void StoreImpl::addListener(StoreListener* listener)
{
    auto it = std::find(m_listeners.begin(), m_listeners.end(), listener);
    if(it == std::end(m_listeners))
        m_listeners.push_back(listener);
}
void StoreImpl::removeListener(StoreListener* listener)
{
    m_listeners.erase(std::remove(m_listeners.begin(), m_listeners.end(), listener), m_listeners.end());
}

void StoreImpl::registerTransactionObserver()
{
    if(m_isTransactionObserverRegistered)
        return;
    
    m_deferredPaymentStorage->loadData();
    [[SKPaymentQueue defaultQueue] addTransactionObserver:(SKPaymentDelegateForwarder *)m_skPaymentDelegateForwarder];
    m_isTransactionObserverRegistered = true;
}

void StoreImpl::unregisterTransactionObserver()
{
    m_isTransactionObserverRegistered = false;
    [[SKPaymentQueue defaultQueue] removeTransactionObserver:(SKPaymentDelegateForwarder *)m_skPaymentDelegateForwarder];
}

void StoreImpl::validateProductIds(std::vector<std::string> const& productIds)
{
    NSMutableArray *productIdentifiers = [NSMutableArray array];
    for(auto const& id : productIds)
    {
        NSString *ssId = [NSString stringWithUTF8String:id.c_str()];
        [productIdentifiers addObject:ssId];
    }
    [((SKProductsDelegateForwarder *)m_skProductsDelegateForwarder) validateProductIdentifiers:productIdentifiers];
    m_isValidatingProductIds = true;
    m_isProductIdsValidated = false;
}

ProductInfo const* StoreImpl::getProductById(std::string const& productId) const
{
    auto it = std::find_if(m_products.begin(), m_products.end(), [&productId](ProductInfo const* p){
        return p->productId == productId;
    });
    if(it != std::end(m_products))
        return (*it);
    return nullptr;
}

bool StoreImpl::canMakePayments() const
{
    return [SKPaymentQueue canMakePayments];
}

bool StoreImpl::requestPayment(std::string const& productId, int32 quantity)
{
    NSString *ssId = [NSString stringWithUTF8String: productId.c_str()];
    SKProduct *product = [((SKProductsDelegateForwarder *)m_skProductsDelegateForwarder) productForId: ssId];
    if(product)
    {
        sUserPreferences->setDeferredTransactionsInvalid(false);
        SKMutablePayment *payment = [SKMutablePayment paymentWithProduct:product];
        
#if SIMULATES_ASK_TO_BUY_IN_SANDBOX
        if([[UIDevice currentDevice].systemVersion floatValue] >= 8.3) {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpartial-availability"
            payment.simulatesAskToBuyInSandbox = YES;
#pragma clang diagnostic pop
        }
#endif // SIMULATES_ASK_TO_BUY_IN_SANDBOX
        
        payment.quantity = quantity;
        [[SKPaymentQueue defaultQueue] addPayment:payment];
        return true;
    }
    
    return false;
}

bool StoreImpl::hasDeferredTransaction(std::string const& productId)
{
    auto const& transactions = m_deferredPaymentStorage->getTransactions();
    for(auto it = transactions.begin(); it != transactions.end(); ++it)
    {
        if((*it).second.productId == productId)
            return true;
    }
    
    return false;
}

void StoreImpl::finishTransaction(std::string const& transactionId)
{
    NSString *ssId = [NSString stringWithUTF8String: transactionId.c_str()];
    [((SKPaymentDelegateForwarder *)m_skPaymentDelegateForwarder) finishTransaction:ssId];
}

bool StoreImpl::validateReceipt()
{
    NSData *data = getReceiptData();
    if(data)
    {
        NSDictionary *receipt = validateReceiptWithData(data);
        if(receipt)
            m_hasReceiptValid = true;
        
#if NS_DEBUG
        validateReceiptWithAppStore(data);
#endif
    }
    
    return m_hasReceiptValid;
}

bool StoreImpl::refreshReceipt()
{
    if(m_isRefreshingReceipt)
        return false;
    
    [((SKPaymentDelegateForwarder *)m_skPaymentDelegateForwarder) refreshReceipt];
    return true;
}

bool StoreImpl::restoreCompletedTransactions()
{
    if(m_isRestoringPaidTransactions)
        return false;
    
    m_isRestoringPaidTransactions = true;
    this->refreshReceipt();
    
    return true;
}

std::vector<std::string> StoreImpl::validatePurchasedProductIds(std::vector<std::string> const& productIds)
{
    NSData *data = getReceiptData();
    if(data)
    {
        NSDictionary *receipt = validateReceiptWithData(data);
        if(receipt)
            return validatePurchasedProductIdsWithReceipt(receipt, productIds);
    }
    
    return {};
}

void StoreImpl::clearCachedData()
{
    m_deferredPaymentStorage->deleteData();
}

void StoreImpl::productsRequestDidFinish()
{
}

void StoreImpl::productsRequestDidFail(StoreError const& error)
{
    m_isValidatingProductIds = false;
    m_isProductIdsValidated = false;
    for(StoreListener* listen: m_listeners)
        listen->onProductsRequestFailed(error);
}

void StoreImpl::productsRequest(std::vector<ProductInfo*> const& products, std::vector<std::string> const& invalidProductIds)
{
    m_isProductIdsValidated = true;
    m_isValidatingProductIds = false;
    m_products = products;
    
    for(StoreListener* listen: m_listeners)
        listen->onProductsRequestFinished();
}

void StoreImpl::updatedTransactions(std::vector<PaymentTransaction>& transactions)
{
    for(auto& transaction: transactions)
    {
        switch(transaction.state)
        {
            case PAYMENT_STATE_PURCHASED:
            {
                // 验证票据
                NSData *data = getReceiptData();
                NSDictionary *receipt = nil;
                if(data)
                    receipt = validateReceiptWithData(data);
                
                // 验证已购买的商品
                std::vector<std::string> validProductIds;
                if(receipt)
                {
                    validProductIds = validatePurchasedProductIdsWithReceipt(receipt, {transaction.productId});
                    m_hasReceiptValid = true;
                }
                if(validProductIds.empty())
                {
                    transaction.state = PAYMENT_STATE_FAILED;
                    if(!receipt)
                        transaction.error.description = "Receipt validation failed.";
                    else
                        transaction.error.description = "There is no transaction in receipt.";
                    transaction.error.code = STORE_ERROR_VALIDATION_FAILED;

                }
                break;
            }
            default:
                break;
        }
        
        switch(transaction.state)
        {
            case PAYMENT_STATE_PURCHASING:
            case PAYMENT_STATE_PURCHASED:
            case PAYMENT_STATE_RESTORED:
            case PAYMENT_STATE_FAILED:
            {
                bool isExist = m_deferredPaymentStorage->findTransaction(transaction.transactionId);
                if(isExist)
                {
                    m_deferredPaymentStorage->removeTransaction(transaction.transactionId);
                    m_deferredPaymentStorage->saveData();
                }

                for(StoreListener* listen: m_listeners)
                    listen->onPaymentTransactionUpdated(transaction);
                break;
            }
            case PAYMENT_STATE_DEFERRED:
            {
                if(sUserPreferences->isDeferredTransactionsInvalid())
                    this->finishTransaction(transaction.transactionId);
                else
                {
                    PaymentTransaction trans;
                    bool isExist = m_deferredPaymentStorage->getTransaction(transaction.transactionId, trans);
                    if(isExist)
                    {
                        uint32 currTimeInSec = (uint32)(time_util::getSystemTimeMillis() / 1000);
                        uint32 diff = currTimeInSec - trans.transactionTime;
                        if(diff > DEFERRED_TRANSACTION_TIMEOUT_IN_SEC)
                        {
                            m_deferredPaymentStorage->removeTransaction(transaction.transactionId);
                            m_deferredPaymentStorage->saveData();
                            this->finishTransaction(transaction.transactionId);
                        }
                    }
                    else
                    {
                        // iOS15+ 延迟交易的TransactionId为空，不记录ID为空的延迟交易
                        if(!transaction.transactionId.empty())
                        {
                            transaction.transactionTime = uint32(time_util::getSystemTimeMillis() / 1000);
                            m_deferredPaymentStorage->addTransaction(transaction);
                            m_deferredPaymentStorage->saveData();
                        }

                        for(StoreListener* listen: m_listeners)
                            listen->onPaymentTransactionUpdated(transaction);
                    }
                }
                break;
            }
        }
    }
}

void StoreImpl::restoreCompletedTransactionsFinished()
{
    for(StoreListener* listen: m_listeners)
        listen->onRestoreCompletedTransactionsFinished();
}

void StoreImpl::restoreCompletedTransactionsFailed(StoreError const& error)
{
    for(StoreListener* listen: m_listeners)
        listen->onRestoreCompletedTransactionsFailed(error);
}

void StoreImpl::receiptRequestDidFinish()
{
    m_isRefreshingReceipt = false;
    
    NSData *data = getReceiptData();
    NSDictionary *receipt = nil;
    if(data)
    {
        receipt = validateReceiptWithData(data);
        m_hasReceiptValid = receipt != nil;
    }
    
    if(m_isRestoringPaidTransactions)
    {
        m_isRestoringPaidTransactions = false;
        if(receipt)
            restorePaidTransactions(m_listeners, receipt);
        else
        {
            StoreError error;
            error.description = "Receipt validation failed.";
            error.code = STORE_ERROR_VALIDATION_FAILED;
            for(StoreListener* listen: m_listeners)
                listen->onRestoreCompletedTransactionsFailed(error);
        }
    }
    else
    {
        for(StoreListener* listen: m_listeners)
            listen->onRefreshReceiptFinished();
    }
}

void StoreImpl::receiptRequestDidFail(StoreError const& error)
{
    m_isRefreshingReceipt = false;
    if(m_isRestoringPaidTransactions)
    {
         m_isRestoringPaidTransactions = false;
        for(StoreListener* listen: m_listeners)
            listen->onRestoreCompletedTransactionsFailed(error);
    }
    else
    {
        for(StoreListener* listen: m_listeners)
            listen->onRefreshReceiptFailed(error);
    }
}

NS_END

#endif // CC_TARGET_PLATFORM == CC_PLATFORM_IOS
