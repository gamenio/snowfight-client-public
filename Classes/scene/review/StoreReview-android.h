//
//  StoreReview-android.h
//  snowfight
//
//  Created by Luthier on 2020/7/18.
//

#ifndef __STORE_REVIEW_ANDROID_H__
#define __STORE_REVIEW_ANDROID_H__

#include "common/Common.h"
#include "StoreReviewProtocol.h"

USING_NS_CC;

NS_BEGIN

class StoreReviewImpl: public StoreReviewProtocol
{
public:
    StoreReviewImpl();
    ~StoreReviewImpl();

    bool isNativeReviewViewSupported() override { return false; }
    void openExternalReviewView() override;
private:
    bool openURL(std::string const& url);
};

NS_END


#endif // __STORE_REVIEW_ANDROID_H__

