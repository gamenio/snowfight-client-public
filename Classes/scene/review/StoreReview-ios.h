//
//  StoreReview-ios.h
//  snowfight
//
//  Created by Luthier on 2020/7/18.
//

#ifndef __STORE_REVIEW_IOS_H__
#define __STORE_REVIEW_IOS_H__

#include "common/Common.h"
#include "StoreReviewProtocol.h"

USING_NS_CC;

NS_BEGIN

class StoreReviewImpl: public StoreReviewProtocol
{
public:
    StoreReviewImpl();
    ~StoreReviewImpl();

    bool isNativeReviewViewSupported() override;
    void openNativeReviewView() override;
    void openExternalReviewView() override;
private:
    
};

NS_END


#endif // __STORE_REVIEW_IOS_H__

