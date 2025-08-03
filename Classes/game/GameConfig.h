#ifndef __GAME_CONFIG_H__
#define __GAME_CONFIG_H__

#include "common/Common.h"

NS_BEGIN

// 使用调试选项
#define USE_DEBUG_OPTION                        0

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
// 应用详情页面链接
#define STORE_APP_URL				"https://play.google.com/store/apps/details?id=" PACKAGE_NAME
#define STORE_APP_URL_MARKET		"market://details?id=" PACKAGE_NAME
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
// 应用详情页面链接
#define STORE_APP_URL				"https://itunes.apple.com/app/id" APPSTORE_ID
#define STORE_APP_URL_MARKET		"itms-apps://itunes.apple.com/app/id" APPSTORE_ID
// 应用评论链接
#define STORE_REVIEW_URL_MARKET     "itms-apps://apps.apple.com/app/id" APPSTORE_ID "?action=write-review"
#else
#define STORE_APP_URL				"http://"
#endif


// 本地玩家数据库密钥
extern char const* LOCALPLAYER_DB_KEY;

// 商店数据秘钥
extern uint8 PAYMENT_DATA_KEY[];
extern uint32 PAYMENT_DATA_KEY_LENGTH;


// 登录验证凭证
#define AUTH_PROOF_PLAYER		"PLAYER"
#define AUTH_PROOF_GM			"GM"

// 动画帧率为12fps时的帧延迟
#define ANIM_NORMAL_FRAME_DELAY		0.083f

NS_END

#endif // __GAME_CONFIG_H__
