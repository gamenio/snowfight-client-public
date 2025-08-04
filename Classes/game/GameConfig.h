#ifndef __GAME_CONFIG_H__
#define __GAME_CONFIG_H__

#include "common/Common.h"

NS_BEGIN

// Use the debug option
#define USE_DEBUG_OPTION                        0

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
// Application details page link
#define STORE_APP_URL				"https://play.google.com/store/apps/details?id=" PACKAGE_NAME
#define STORE_APP_URL_MARKET		"market://details?id=" PACKAGE_NAME
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
// Application details page link
#define STORE_APP_URL				"https://itunes.apple.com/app/id" APPSTORE_ID
#define STORE_APP_URL_MARKET		"itms-apps://itunes.apple.com/app/id" APPSTORE_ID
// Application review link
#define STORE_REVIEW_URL_MARKET     "itms-apps://apps.apple.com/app/id" APPSTORE_ID "?action=write-review"
#else
#define STORE_APP_URL				"http://"
#endif


// Local player database key
extern char const* LOCALPLAYER_DB_KEY;

// Payment data key
extern uint8 PAYMENT_DATA_KEY[];
extern uint32 PAYMENT_DATA_KEY_LENGTH;


// Authentication proofs
#define AUTH_PROOF_PLAYER		"PLAYER"
#define AUTH_PROOF_GM			"GM"

// Frame delay when the animation frame rate is 12 fps
#define ANIM_NORMAL_FRAME_DELAY		0.083f

NS_END

#endif // __GAME_CONFIG_H__
