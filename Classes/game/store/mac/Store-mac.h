//
//  Store-mac.h
//  snowfight
//
//  Created by Luthier on 2019/5/8.
//

#ifndef __STORE_MAC_H__
#define __STORE_MAC_H__

#include "common/Common.h"
#include "../StoreProtocol.h"


USING_NS_CC;

NS_BEGIN


class StoreImpl : public StoreProtocol
{
public:
	StoreImpl();
    virtual ~StoreImpl();
    
	void validateProductIds(std::vector<std::string> const& productIds) override { }
	bool isValidatingProductIds() const override { return false; }
	bool isProductIdsValidated() const override { return false; }
	ProductInfo const* getProductById(std::string const& productId) const override { return nullptr; }
	std::vector<ProductInfo*> getProducts() const override { return {}; }

	bool canMakePayments() const  override { return false; }
	bool requestPayment(std::string const& productId, int32 quantity) override { return false; }
    
	virtual void addListener(StoreListener* listener) override {  }
	virtual void removeListener(StoreListener* listener) override { }
    
private:
};

NS_END

#endif // __STORE_MAC_H__
