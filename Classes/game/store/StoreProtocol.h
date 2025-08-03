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
    
    std::string productId;                  // 用于标识应用商店产品的字符串。
    std::string localizedDescription;       // 产品说明。
    std::string localizedTitle;             // 产品名称。
    std::string localizedPrice;             // 以当地货币计算的产品价格的字符串
};

enum PaymentState
{
    PAYMENT_STATE_DEFERRED,         // 该事务处于队列中，但其最终状态是待处理的外部操作，例如：用户确认购买
    PAYMENT_STATE_FAILED,           // 交易失败
    PAYMENT_STATE_PURCHASED,        // 应用商店成功处理了付款
    PAYMENT_STATE_RESTORED,         // 此交易恢复用户先前购买的内容
#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
    PAYMENT_STATE_UNSPECIFIED,      // 未指定的交易状态
#elif CC_TARGET_PLATFORM == CC_PLATFORM_IOS
    PAYMENT_STATE_PURCHASING,       // 该交易正在由应用商店处理
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
    uint32 transactionTime; // 交易时间戳，单位：秒
    PaymentState state;
    
    // 当PaymentState等于PAYMENT_STATE_FAILED时才会包含错误信息
    StoreError error;
};


class StoreListener
{
public:
    // 当产品请求完成或者失败时被调用
    virtual void onProductsRequestFinished() {}
    virtual void onProductsRequestFailed(StoreError const& error) {}

    // 当交易更新时被调用
    virtual void onPaymentTransactionUpdated(PaymentTransaction const& transaction) {}

    // 当购买失败时被调用。仅用于Android
    virtual void onPurchasesFailed(StoreError const& error) {}

    // 当消耗品确认完成或者失败时被调用。仅用于Android
    virtual void onConsumeFinished(PaymentTransaction const& transaction) {}
    virtual void onConsumeFailed(StoreError const& error) {}

    // 当非消耗品确认完成或者失败时被调用。仅用于Android
    virtual void onAcknowledgePurchaseFinished(PaymentTransaction const& transaction) {}
    virtual void onAcknowledgePurchaseFailed(StoreError const& error) {}

    // 当恢复交易完成或者失败时被调用。仅用于iOS
    virtual void onRestoreCompletedTransactionsFinished() {}
    virtual void onRestoreCompletedTransactionsFailed(StoreError const& error) {}

    // 当刷新票据完成或者失败时被调用。仅用于iOS
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

    // 完成所有类型商品的交易。例如：非消耗品。仅用于iOS
    virtual void finishTransaction(std::string const& transactionId) { }
    // 恢复购买。如果恢复购买操作正在进行将返回false，否则返回true。仅用于iOS
    virtual bool restoreCompletedTransactions() { return false; }

    // 验证票据。如果通过则返回true，否则返回false。仅用于iOS
    virtual bool validateReceipt() { return false; }
    // 刷新票据。如果票据正在刷新将返回false，否则返回true。仅用于iOS
    virtual bool refreshReceipt() { return false; }
    // 票据是否有效。仅用于iOS
    virtual bool hasReceiptValid() const { return false; }
    // 验证已购买商品ID。返回验证通过的商品ID。仅用于iOS
    virtual std::vector<std::string> validatePurchasedProductIds(std::vector<std::string> const& productIds) { return {}; }

    // 确认非消耗商品和订阅商品的购买。仅用于Android
    virtual void acknowledgeTransaction(PaymentTransaction const& transaction) { }
    // 确认消耗商品的购买。仅用于Android
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
