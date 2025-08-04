//
//  StoreProtocol.h
//  snowfight
//
//  Created by Luthier on 2019/5/8.
//

#ifndef __STORE_PROTOCOL_H__
#define __STORE_PROTOCOL_H__

#include "common/Common.h"
#include "StoreError.h"

USING_NS_CC;

NS_BEGIN

struct ProductInfo
{
    ProductInfo() :
        productId(""),
        localizedDescription(""),
        localizedTitle(""),
        localizedPrice("")
    {
    }
    
    std::string productId;                  // A string used to identify the app store product.
    std::string localizedDescription;       // Product description.
    std::string localizedTitle;             // Product name.
    std::string localizedPrice;             // String of product prices in local currency.
};

enum PaymentState
{
    PAYMENT_STATE_DEFERRED,         // The transaction is in a queue, but its final status is a pending external operation such as Ask to Buy.
    PAYMENT_STATE_FAILED,           // The transaction failed.
    PAYMENT_STATE_PURCHASED,        // The app store successfully processed the transaction.
    PAYMENT_STATE_RESTORED,         // The transaction restores content previously purchased by the user.
#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
    PAYMENT_STATE_UNSPECIFIED,      // Unspecified transaction state.
#elif CC_TARGET_PLATFORM == CC_PLATFORM_IOS
    PAYMENT_STATE_PURCHASING,       // A transaction that is being processed by the app store.
#endif
};

struct PaymentTransaction
{
    PaymentTransaction() :
        transactionId(""),
        productId(""),
        transactionTime(0),
        state(PAYMENT_STATE_DEFERRED)
    { }
    
    std::string transactionId;
    std::string productId;
    uint32 transactionTime; // Transaction timestamp. Unit: seconds
    PaymentState state;
    
	// Error message is included when PaymentState is equal to PAYMENT_STATE_FAILED.
    StoreError error;
};


class StoreListener
{
public:
	// Called when a product request completes or fails
    virtual void onProductsRequestFinished() {}
    virtual void onProductsRequestFailed(StoreError const& error) {}

	// Called when the transaction is updated
    virtual void onPaymentTransactionUpdated(PaymentTransaction const& transaction) {}

	// Called when a purchase fails. For Android only
    virtual void onPurchasesFailed(StoreError const& error) {}

	// Called when a consumable acknowledgement completes or fails. For Android only.
    virtual void onConsumeFinished(PaymentTransaction const& transaction) {}
    virtual void onConsumeFailed(StoreError const& error) {}

	// Called when a non-consumable acknowledgement completes or fails. For Android only.
    virtual void onAcknowledgePurchaseFinished(PaymentTransaction const& transaction) {}
    virtual void onAcknowledgePurchaseFailed(StoreError const& error) {}

	// Called when a restore transaction completes or fails. For iOS only.
    virtual void onRestoreCompletedTransactionsFinished() {}
    virtual void onRestoreCompletedTransactionsFailed(StoreError const& error) {}

	// Called when a refresh receipt completes or fails. For iOS only.
    virtual void onRefreshReceiptFinished() {}
    virtual void onRefreshReceiptFailed(StoreError const& error) {}
    
};

class StoreProtocol
{
public:
    virtual ~StoreProtocol()
    {
        
    }
    
    virtual void validateProductIds(std::vector<std::string> const& productIds) = 0;
    virtual bool isValidatingProductIds() const = 0;
    virtual bool isProductIdsValidated() const = 0;
    virtual ProductInfo const* getProductById(std::string const& productId) const = 0;
    virtual std::vector<ProductInfo*> getProducts() const = 0;
    
    virtual bool canMakePayments() const = 0;
    virtual bool requestPayment(std::string const& productId, int32 quantity = 1) = 0;
    virtual bool hasDeferredTransaction(std::string const& productId){ return false; }

	// Finish a transaction for all types of products. Example: non-consumable products. For iOS only
    virtual void finishTransaction(std::string const& transactionId) { }
	// Restore purchases. Returns false if restoring the purchase operation is in progress, otherwise returns true. iOS only.
    virtual bool restoreCompletedTransactions() { return false; }

	// Validates the receipt. Returns true if it passes, false otherwise. For iOS only.
    virtual bool validateReceipt() { return false; }
	// Refreshes the receipt. Returns false if the receipt is being refreshed, otherwise returns true. For iOS only.
    virtual bool refreshReceipt() { return false; }
	// Whether the receipt is valid or not. For iOS only
    virtual bool hasReceiptValid() const { return false; }
	// Validates the purchased product Ids. returns the validated product Ids. only for iOS.
    virtual std::vector<std::string> validatePurchasedProductIds(std::vector<std::string> const& productIds) { return {}; }

	// Acknowledges a purchase of non-consumable product and subscription product. For Android only.
    virtual void acknowledgeTransaction(PaymentTransaction const& transaction) { }
	// Acknowledges a purchase of a consumable product. For Android only.
    virtual void consume(PaymentTransaction const& transaction) { }
    
    virtual void addListener(StoreListener* listener) = 0;
    virtual void removeListener(StoreListener* listener) = 0;

    virtual void registerTransactionObserver() { }
    virtual void unregisterTransactionObserver() { }

    virtual void onEnterBackground() { }
    virtual void onEnterForeground() { }
    
    virtual void clearCachedData() {}
    
};

NS_END

#endif // __STORE_PROTOCOL_H__
