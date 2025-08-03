//
//  StoreReviewProtocol.h
//  snowfight
//
//  Created by Luthier on 2020/7/18.
//

#ifndef __STORE_REVIEW_PROTOCOL_H__
#define __STORE_REVIEW_PROTOCOL_H__

#include "common/Common.h"

NS_BEGIN

class StoreReviewProtocol
{
public:
    virtual ~StoreReviewProtocol()
    {
    }

    virtual bool isNativeReviewViewSupported() = 0;
    virtual void openNativeReviewView() {}
    virtual void openExternalReviewView(){}
};

NS_END


#endif // __STORE_REVIEW_PROTOCOL_H__

