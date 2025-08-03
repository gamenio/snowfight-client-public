//
//  StoreReview.h
//  snowfight
//
//  Created by Luthier on 2020/7/18.
//

#ifndef __STORE_REVIEW_H__
#define __STORE_REVIEW_H__

#include "common/Common.h"
#include "StoreReviewProtocol.h"

#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS
#include "StoreReview-ios.h"
#elif CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
#include "StoreReview-android.h"
#else
#include "StoreReview-unknown.h"
#endif

USING_NS_CC;

NS_BEGIN

enum ReviewProcess
{
	REVIEW_PROCESS_HERO_LEVEL						= 0x00000001,
	REVIEW_PROCESS_UPGRADE_STATS_OR_UNLOCK_HERO		= 0x00000002,
	REVIEW_PROCESS_LAUNCH_APP						= 0x00000004,
	REVIEW_PROCESS_ALL                              = REVIEW_PROCESS_HERO_LEVEL 
													| REVIEW_PROCESS_UPGRADE_STATS_OR_UNLOCK_HERO 
													| REVIEW_PROCESS_LAUNCH_APP,
};

#define REVIEW_UPGRADE_HERO_TO_LEVEL				5
#define REVIEW_UPGRADE_STAT_TIMES					5
#define REVIEW_APP_LAUNCH_TIMES						3

class StoreReview: public StoreReviewProtocol
{
public:
    static StoreReview* instance();
    
    void requestReview();
	void cancelRequest();
	bool isProcessCompleted(ReviewProcess process);
	void setProcessCompleted(ReviewProcess process);

	void requestReviewManually();
    
private:
    StoreReview();
    ~StoreReview();

    void openReviewView();
    bool isNativeReviewViewSupported() override;
    void openNativeReviewView() override;
	void openExternalReviewView() override;

	int32 getNextReviewTime(int32 delayTime);
    
    StoreReviewImpl* m_storeReviewImpl;
    
};

#define sStoreReview StoreReview::instance()

NS_END


#endif // __STORE_REVIEW_H__

