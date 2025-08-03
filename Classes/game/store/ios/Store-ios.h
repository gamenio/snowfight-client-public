//
//  Store-ios.h
//  snowfight
//
//  Created by Luthier on 2019/5/8.
//

#ifndef __STORE_IOS_H__
#define __STORE_IOS_H__

#include "common/Common.h"
#include "../StoreProtocol.h"
#include "../PaymentStorage.h"

USING_NS_CC;

NS_BEGIN

class StoreImpl: public StoreProtocol
{
public:
    StoreImpl();
    virtual ~StoreImpl();
    
    void addListener(StoreListener* listener) override;
    void removeListener(StoreListener* listener) override;
    // 注册交易观察者并加载交易数据
    void registerTransactionObserver() override;
    void unregisterTransactionObserver() override;
    
    void validateProductIds(std::vector<std::string> const& productIds) override;
    bool isValidatingProductIds() const override { return m_isValidatingProductIds; }
    bool isProductIdsValidated() const override { return m_isProductIdsValidated; }
    ProductInfo const* getProductById(std::string const& productId) const override;
    std::vector<ProductInfo*> getProducts() const override { return m_products; }
    
    bool canMakePayments() const override;
    bool requestPayment(std::string const& productId, int32 quantity) override;
    bool hasDeferredTransaction(std::string const& productId) override;
    void finishTransaction(std::string const& transactionId) override;
    bool validateReceipt() override;
    bool refreshReceipt() override;
    bool hasReceiptValid() const override { return m_hasReceiptValid; }
    bool restoreCompletedTransactions() override;
    std::vector<std::string> validatePurchasedProductIds(std::vector<std::string> const& productIds) override;
    
    void clearCachedData() override;
    
    // SKProductsRequestDelegate
    void productsRequestDidFinish();
    void productsRequestDidFail(StoreError const& error);
    void productsRequest(std::vector<ProductInfo*> const& products, std::vector<std::string> const& invalidProductIds);
    
    // SKPaymentTransactionObserver
    void updatedTransactions(std::vector<PaymentTransaction>& transactions);
    void restoreCompletedTransactionsFinished();
    void restoreCompletedTransactionsFailed(StoreError const& error);
    
    // ReceiptRequestDelegate
    void receiptRequestDidFinish();
    void receiptRequestDidFail(StoreError const& error);
    
    
private:
    std::vector<StoreListener*> m_listeners;
    void* m_skProductsDelegateForwarder;
    void* m_skPaymentDelegateForwarder;
    std::vector<ProductInfo*> m_products;
    bool m_isValidatingProductIds;
    bool m_isProductIdsValidated;
    bool m_isRefreshingReceipt;
    bool m_isRestoringPaidTransactions;
    bool m_isTransactionObserverRegistered;
    
    bool m_hasReceiptValid;
    PaymentStorage* m_deferredPaymentStorage;
    
};


NS_END

#endif // __STORE_IOS_H__
