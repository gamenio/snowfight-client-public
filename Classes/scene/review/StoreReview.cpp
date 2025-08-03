//
//  StoreReview.cpp
//  snowfight
//
//  Created by Luthier on 2020/7/18.
//

#include "StoreReview.h"

#include "common/utils/TimeUtil.h"
#include "game/UserPreferences.h"
#include "game/firservice/FirebaseService.h"
#include "ReviewDialog.h"

NS_BEGIN

#if NS_DEBUG
#define NO_CHECK_LAST_REVIEWED_VERSION			0

#endif // NS_DEBUG

// 评论被推迟时间。默认3天
static const int32 REVIEW_DEFERRED_TIME_SEC	= 3 * 24 * 60 * 60;

// 评论间隔时间。默认60天 
static const int32 REVIEW_INTERVAL_SEC = 60 * 24 * 60 * 60;

// 系统原生的评论间隔时间。默认120天 
static const int32 NATIVE_REVIEW_INTERVAL_SEC = 120 * 24 * 60 * 60;

#define SCHEDULE_KEY_REVIEW_DELAYED		"ReviewDelayed"

StoreReview::StoreReview() :
    m_storeReviewImpl(nullptr)
{
    m_storeReviewImpl = new StoreReviewImpl();
}

StoreReview::~StoreReview()
{
    CC_SAFE_DELETE(m_storeReviewImpl);
}

StoreReview* StoreReview::instance()
{
    static StoreReview instance;
    return &instance;
}

void StoreReview::requestReview()
{
	int32 lastReviewedVersion = sUserPreferences->getLastReviewedVersion();
#if NO_CHECK_LAST_REVIEWED_VERSION
	std::stringstream version;
	version << std::hex << ((lastReviewedVersion >> 16) & 0x000000FF) << "." << ((lastReviewedVersion >> 8) & 0x000000FF) << "." << (lastReviewedVersion & 0x000000FF);
	CCLOG("StoreReview: No check last reviewed version (%s).", version.str().c_str());
	lastReviewedVersion = 0;
#endif // NO_CHECK_LAST_REVIEWED_VERSION
	if (lastReviewedVersion != APP_VERSION)
	{
		int32 reviewTime = sUserPreferences->getNextReviewTime();
		int32 currTime = (int32)(time_util::getSystemTimeMillis() / 1000);
		if (currTime >= reviewTime)
		{
			int32 processes = sUserPreferences->getReviewProcesses();
			if ((processes & REVIEW_PROCESS_ALL) == REVIEW_PROCESS_ALL)
			{
				Director::getInstance()->getScheduler()->schedule([this](float dt) {
					this->openReviewView();
				}, this, 0, 0, 1.0f, false, SCHEDULE_KEY_REVIEW_DELAYED);
			}
		}
	}
}

void StoreReview::cancelRequest()
{
	auto scheduler = Director::getInstance()->getScheduler();
	if (scheduler->isScheduled(SCHEDULE_KEY_REVIEW_DELAYED, this))
		scheduler->unschedule(SCHEDULE_KEY_REVIEW_DELAYED, this);

}

bool StoreReview::isProcessCompleted(ReviewProcess process)
{
	bool completed = sUserPreferences->isReviewProcessCompleted(process);
	return completed;
}

void StoreReview::setProcessCompleted(ReviewProcess process)
{
	sUserPreferences->setReviewProcessCompleted(process, true);
}

void StoreReview::requestReviewManually()
{
	ReviewDialog* dialog = ReviewDialog::create();
	dialog->setTitle(sLocaleMgr->getString("review_dlg_title_rate_us"));
	dialog->addPositiveButton(sLocaleMgr->getString("review_dlg_btn_rate_now"), [this](Ref* sender) {
		this->openExternalReviewView();
	});
	dialog->addNegativeButton(sLocaleMgr->getString("review_dlg_btn_later"));
	dialog->setMessage(sLocaleMgr->getString("review_dlg_msg_want_to_rate"));
	dialog->show();
}

void StoreReview::openReviewView()
{
	ReviewDialog* dialog = ReviewDialog::create();
	dialog->setTitle(sLocaleMgr->getString("review_dlg_title_rate_us"));
	dialog->setMessage(sLocaleMgr->getString("review_dlg_msg_enjoy_playing"));
	dialog->addPositiveButton(sLocaleMgr->getString("review_dlg_btn_rate_now"), [this](Ref* sender) {
		int32 reviewTime;
		if (this->isNativeReviewViewSupported())
		{
			this->openNativeReviewView();
			reviewTime = this->getNextReviewTime(NATIVE_REVIEW_INTERVAL_SEC);
		}
		else
		{
			this->openExternalReviewView();
			reviewTime = this->getNextReviewTime(REVIEW_INTERVAL_SEC);
		}
		sUserPreferences->setLastReviewedVersion(APP_VERSION);
		sUserPreferences->setNextReviewTime(reviewTime);
        sAnalytics->logReview("rate_now");
	});
	dialog->addNeutralButton(sLocaleMgr->getString("review_dlg_btn_later"), [this](Ref* sender){
		int32 reviewTime = this->getNextReviewTime(REVIEW_DEFERRED_TIME_SEC);
		sUserPreferences->setNextReviewTime(reviewTime);
		sAnalytics->logReview("later");
	});
	dialog->addNegativeButton(sLocaleMgr->getString("review_dlg_btn_no_thanks"), [this](Ref* sender) {
		sUserPreferences->setLastReviewedVersion(APP_VERSION);
        sAnalytics->logReview("no_thanks");
	});
	dialog->show();
}

void StoreReview::openNativeReviewView()
{
	if (!m_storeReviewImpl)
		return;

	m_storeReviewImpl->openNativeReviewView();
}

bool StoreReview::isNativeReviewViewSupported()
{
    if(m_storeReviewImpl)
        return m_storeReviewImpl->isNativeReviewViewSupported();

    return false;
}

void StoreReview::openExternalReviewView()
{
    if(m_storeReviewImpl)
		m_storeReviewImpl->openExternalReviewView();
}

int32 StoreReview::getNextReviewTime(int32 delayTime)
{
	int32 currTime = (int32)(time_util::getSystemTimeMillis() / 1000);
	int32 reviewTime = currTime + delayTime;
	return reviewTime;
}


NS_END
