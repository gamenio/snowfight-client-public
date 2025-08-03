//
//  Store.cpp
//  snowfight
//
//  Created by Luthier on 2019/5/8.
//

#include "Store.h"

NS_BEGIN

Store::Store() :
    m_storeImpl(nullptr)
{
    m_storeImpl = new StoreImpl();
}

Store::~Store()
{
    CC_SAFE_DELETE(m_storeImpl);
}

Store* Store::instance()
{
    static Store instance;
    return &instance;
}

void Store::validateProductIds(std::vector<std::string> const& productIds)
{
    if(m_storeImpl)
        m_storeImpl->validateProductIds(productIds);
}

bool Store::isValidatingProductIds() const 
{
    if(m_storeImpl)
        return m_storeImpl->isValidatingProductIds();
    
    return false;
}

bool Store::isProductIdsValidated() const 
{
    if(m_storeImpl)
        return m_storeImpl->isProductIdsValidated();
    
    return false;
}


ProductInfo const* Store::getProductById(std::string const& productId) const
{
    if(m_storeImpl)
        return m_storeImpl->getProductById(productId);
    
    return nullptr;
}

std::vector<ProductInfo*> Store::getProducts() const
{
    std::vector<ProductInfo*> products;
    if(m_storeImpl)
        products =  m_storeImpl->getProducts();

    return products;
}

bool Store::canMakePayments() const
{
    if(m_storeImpl)
        return m_storeImpl->canMakePayments();
    return false;
}

bool Store::requestPayment(std::string const& productId, int32 quantity)
{
    if(m_storeImpl)
        return m_storeImpl->requestPayment(productId, quantity);
    
    return false;
}

bool Store::hasDeferredTransaction(std::string const& productId)
{
    if(m_storeImpl)
        return m_storeImpl->hasDeferredTransaction(productId);

    return false;
}

void Store::finishTransaction(std::string const& transactionId)
{
    if(m_storeImpl)
        m_storeImpl->finishTransaction(transactionId);
}

bool Store::validateReceipt()
{
    if(m_storeImpl)
        return m_storeImpl->validateReceipt();
    
    return false;
}

bool Store::refreshReceipt()
{
    if(m_storeImpl)
        return m_storeImpl->refreshReceipt();
    
    return false;
}

bool Store::hasReceiptValid() const
{
    if(m_storeImpl)
        return m_storeImpl->hasReceiptValid();
    
    return false;
}

bool Store::restoreCompletedTransactions()
{
    if(m_storeImpl)
        return m_storeImpl->restoreCompletedTransactions();
    
    return false;
}

std::vector<std::string> Store::validatePurchasedProductIds(std::vector<std::string> const& productIds)
{
    if(m_storeImpl)
        return m_storeImpl->validatePurchasedProductIds(productIds);
    return {};
}

void Store::acknowledgeTransaction(PaymentTransaction const& transaction)
{
    if(m_storeImpl)
        m_storeImpl->acknowledgeTransaction(transaction);
}

void Store::consume(PaymentTransaction const& transaction)
{
    if(m_storeImpl)
        m_storeImpl->consume(transaction);
}

void Store::addListener(StoreListener* listener)
{
    if(m_storeImpl)
        m_storeImpl->addListener(listener);
}

void Store::removeListener(StoreListener* listener)
{
    if(m_storeImpl)
        m_storeImpl->removeListener(listener);
}

void Store::registerTransactionObserver()
{
    if(m_storeImpl)
        m_storeImpl->registerTransactionObserver();
}

void Store::unregisterTransactionObserver()
{
    if(m_storeImpl)
        m_storeImpl->unregisterTransactionObserver();
}

void Store::onEnterBackground()
{
    if(m_storeImpl)
        m_storeImpl->onEnterBackground();
}

void Store::onEnterForeground()
{
    if(m_storeImpl)
        m_storeImpl->onEnterForeground();
}

void Store::clearCachedData()
{
    if(m_storeImpl)
        m_storeImpl->clearCachedData();
}

NS_END
