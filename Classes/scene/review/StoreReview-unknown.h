//
//  StoreReview-unknown.h
//  snowfight
//
//  Created by Luthier on 2020/7/18.
//

#ifndef __STORE_REVIEW_UNKNOWN_H__
#define __STORE_REVIEW_UNKNOWN_H__

#include "common/Common.h"
#include "StoreReviewProtocol.h"

USING_NS_CC;

NS_BEGIN

class StoreReviewImpl: public StoreReviewProtocol
{
public:
	StoreReviewImpl() {}
	~StoreReviewImpl() {}

	bool isNativeReviewViewSupported() override {  return false; }
    
};

NS_END


#endif // __STORE_REVIEW_UNKNOWN_H__

