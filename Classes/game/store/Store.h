//
//  Store.h
//  snowfight
//
//  Created by Luthier on 2019/5/8.
//

#ifndef __STORE_H__
#define __STORE_H__

#include "common/Common.h"
#include "StoreProtocol.h"

#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS
#include "ios/Store-ios.h"
#elif CC_TARGET_PLATFORM == CC_PLATFORM_MAC
#include "mac/Store-mac.h"
#elif CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
#include "android/Store-android.h"
#elif CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
#include "win32/Store-win32.h"
#else
#error "Store: Target platform not supported."
#endif



USING_NS_CC;

NS_BEGIN


class Store: public StoreProtocol
{
public:
    Store();
    virtual ~Store();
    
    static Store* instance();
    
    void validateProductIds(std::vector<std::string> const& productIds) override;
    bool isValidatingProductIds() const override;
    bool isProductIdsValidated() const override;
    ProductInfo const* getProductById(std::string const& productId) const override;
    std::vector<ProductInfo*> getProducts() const override;
    
    bool canMakePayments() const override;
    bool requestPayment(std::string const& productId, int32 quantity = 1) override;
    bool hasDeferredTransaction(std::string const& productId) override;
    
    void finishTransaction(std::string const& transactionId) override;
    bool restoreCompletedTransactions() override;
    
    bool validateReceipt() override;
    bool refreshReceipt() override;
    bool hasReceiptValid() const override;
    std::vector<std::string> validatePurchasedProductIds(std::vector<std::string> const& productIds) override;

    void acknowledgeTransaction(PaymentTransaction const& transaction) override;
    void consume(PaymentTransaction const& transaction) override;

    virtual void addListener(StoreListener* listener) override;
    virtual void removeListener(StoreListener* listener) override;

    void registerTransactionObserver() override;
    void unregisterTransactionObserver() override;

    void onEnterBackground() override;
    void onEnterForeground() override;
    
    void clearCachedData() override;
    
private:
    StoreProtocol* m_storeImpl;
};

NS_END

#define sStore Store::instance()

#endif // __STORE_H__
