//
//  StoreReview-android.mm
//  snowfight
//
//  Created by Luthier on 2020/7/18.
//

#include "StoreReview-android.h"

#include "game/GameConfig.h"
#include "scene/gui/ModalDialog.h"

NS_BEGIN

static const std::string storeReviewHelperClassName = "io/gamen/snowfight/StoreReviewHelper";

StoreReviewImpl::StoreReviewImpl()
{
    
}

StoreReviewImpl::~StoreReviewImpl()
{
    
}

void StoreReviewImpl::openExternalReviewView()
{
    if(!this->openURL(STORE_APP_URL_MARKET))
        this->openURL(STORE_APP_URL);
}


bool StoreReviewImpl::openURL(std::string const& url)
{
    bool ret = JniHelper::callStaticBooleanMethod(storeReviewHelperClassName, "openURL", url);
    return ret;
}

NS_END
