//
//  SKProductsDelegateForwarder.m
//  snowfight
//
//  Created by Luthier on 2019/5/12.
//

#import "SKProductsDelegateForwarder.h"

#import "StoreError-ios.h"

@interface SKProductsDelegateForwarder ()

@property(nonatomic, copy) NSArray<SKProduct *> *products;
@end

@implementation SKProductsDelegateForwarder

- (id) initWithHandlerInstance:(StoreImpl *)handle
{
    self = [super init];
    if (self) {
        m_handlerInstance = handle;
    }
    return self;
}

- (void)validateProductIdentifiers:(NSArray *)productIdentifiers
{
    SKProductsRequest *productsRequest = [[SKProductsRequest alloc]
                                          initWithProductIdentifiers:[NSSet setWithArray:productIdentifiers]];
    
    
    productsRequest.delegate = self;
    [productsRequest start];
    
}

- (void)requestDidFinish:(SKRequest *)request
{
    [request autorelease];
    
    cocos2d::Director::getInstance()->getScheduler()->performFunctionInCocosThread([self]{
        if(m_handlerInstance)
            m_handlerInstance->productsRequestDidFinish();
    });
}

- (void)request:(SKRequest *)request didFailWithError:(NSError *)error
{
    [request autorelease];
    
    StoreError storeError = storeErrorWithNSError(error);
    cocos2d::Director::getInstance()->getScheduler()->performFunctionInCocosThread([self, storeError](){
        if(m_handlerInstance)
            m_handlerInstance->productsRequestDidFail(storeError);
    });
}

- (void)productsRequest:(SKProductsRequest *)request didReceiveResponse:(SKProductsResponse *)response
{
    @synchronized (self) {
        self.products = response.products;
    }
    
    std::vector<ProductInfo*> products;
    for (SKProduct *product in self.products) {
        ProductInfo* info = new ProductInfo();
        
        if(product.productIdentifier)
            info->productId = product.productIdentifier.UTF8String;
        if(product.localizedTitle)
            info->localizedTitle = product.localizedTitle.UTF8String;
        if(product.localizedDescription)
            info->localizedDescription = product.localizedDescription.UTF8String;
        
        NSNumberFormatter *numberFormatter = [[NSNumberFormatter alloc] init];
        [numberFormatter setFormatterBehavior:NSNumberFormatterBehavior10_4];
        [numberFormatter setNumberStyle:NSNumberFormatterCurrencyStyle];
        [numberFormatter setLocale:product.priceLocale];
        NSString *formattedString = [numberFormatter stringFromNumber:product.price];
        if(formattedString)
            info->localizedPrice = formattedString.UTF8String;
        [numberFormatter release];
        
        products.emplace_back(info);
    }
    
    std::vector<std::string> invalidProductIds;
    for (NSString *invalidIdentifier in response.invalidProductIdentifiers) {
        std::string id = invalidIdentifier.UTF8String;
        invalidProductIds.emplace_back(id);
    }
    
    cocos2d::Director::getInstance()->getScheduler()->performFunctionInCocosThread([self, products, invalidProductIds](){
        if (m_handlerInstance)
            m_handlerInstance->productsRequest(products, invalidProductIds);
    });
}

- (SKProduct *)productForId:(NSString *)productId
{
    SKProduct *retVal = nil;
    @synchronized (self) {
        for (SKProduct *product in self.products) {
            if([product.productIdentifier isEqualToString: productId]) {
                retVal =  product;
                break;
            }
        }
    }
    return retVal;
}

- (void)dealloc
{
    m_handlerInstance = nil;
    @synchronized (self) {
        self.products = nil;
    }
    
    [super dealloc];
}

@end
