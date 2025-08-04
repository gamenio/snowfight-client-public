//
//  Store-android.h
//  snowfight
//
//  Created by Luthier on 2019/5/8.
//

#ifndef __STORE_ANDROID_H__
#define __STORE_ANDROID_H__

#include "common/Common.h"
#include "../StoreProtocol.h"


USING_NS_CC;

NS_BEGIN

struct GoogleBillingTransaction: PaymentTransaction
{
    GoogleBillingTransaction() :
            isAcknowledged(false),
            purchaseToken(""),
            packageName(""),
            signature(""),
            originalJson(""),
            developerPayload("")
    {

    }

    bool isAcknowledged; // Whether the transaction is acknowledged when its state is PURCHASED
    std::string purchaseToken;
    std::string packageName;
    std::string signature;
    std::string originalJson;
    std::string developerPayload;
};

struct GoogleBillingProduct: ProductInfo
{
	GoogleBillingProduct() :
			type("")
	{

	}

    std::string type;
};

class StoreImpl : public StoreProtocol
{
public:
	StoreImpl();
    virtual ~StoreImpl();

	void validateProductIds(std::vector<std::string> const& productIds) override;
	bool isValidatingProductIds() const override { return m_isValidatingProductIds; }
	bool isProductIdsValidated() const override { return m_isProductIdsValidated; }
	ProductInfo const* getProductById(std::string const& productId) const override;
	std::vector<ProductInfo*> getProducts() const override { return m_products; }

	bool canMakePayments() const  override { return true; }
	bool requestPayment(std::string const& productId, int32 quantity) override;
	bool hasDeferredTransaction(std::string const& productId) override;

	void addListener(StoreListener* listener) override;
	void removeListener(StoreListener* listener) override;

    void registerTransactionObserver() override;
    void unregisterTransactionObserver() override;

	void acknowledgeTransaction(PaymentTransaction const& transaction) override;
	void consume(PaymentTransaction const& transaction) override;

	void onProductDetailsRequestSuccess(std::vector<ProductInfo*> const& products);
	void onProductDetailsRequestFailed(StoreError const& error);

	void onQueryPurchasesSuccess(std::vector<GoogleBillingTransaction>& transactions);
	void onQueryPurchasesFailed(StoreError const& error);

	void onPurchasesUpdated(std::vector<GoogleBillingTransaction>& transactions);
	void onPurchasesFailed(StoreError const& error);

	void onAcknowledgePurchaseSuccess(std::string const& productId, std::string const& transactionId, std::string const& purchaseToken);
	void onAcknowledgePurchaseFailed(std::string const& productId, std::string const& transactionId, StoreError const& error);

    void onConsumeSuccess(std::string const& productId, std::string const& transactionId, std::string const& purchaseToken);
    void onConsumeFailed(std::string const& productId, std::string const& transactionId, StoreError const& error);

    void onEnterForeground() override ;

private:
	void initBillingManager();
	void destroyBillingManager();
    void queryCachedPurchases(std::string const& productType);
	bool verifyTransaction(GoogleBillingTransaction const &transaction);

    jobject m_billingMgrJObj;

    std::vector<StoreListener*> m_listeners;
    std::vector<ProductInfo*> m_products;
    bool m_isValidatingProductIds;
    bool m_isProductIdsValidated;
    bool m_isPurchasing;
    bool m_isTransactionObserverRegistered;

	std::unordered_map<std::string /* TransactionID */, GoogleBillingTransaction> m_acknowledgingTransactions;
};

NS_END

#endif // __STORE_ANDROID_H__
