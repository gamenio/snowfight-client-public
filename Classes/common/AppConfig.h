#ifndef __APP_DEFINES_H__
#define __APP_DEFINES_H__

#include "cocos2d.h"

#include "Defines.h"
#include "PublicMacros.h"

//////////////////////////////////////////////////////////////////////////
// 应用配置
//////////////////////////////////////////////////////////////////////////

NS_BEGIN

int32 getBuildNumber();

// pvr.czz文件解压密钥
extern uint32 PVR_ENCRYPTION_KEY[4];

// 地图数据解压密钥
extern const char MAPDATA_KEY[];

NS_END


// 应用版本号
//    格式:        0x00  Major  Minor  Revision
//                   00  01     00     00
#define APP_VERSION				0x00030200

// 构建版本号
#define BUILD_NUMBER				getBuildNumber()

// 渠道ID
// 0 未指定渠道ID, 1-1000为保留渠道ID
#define CHANNEL_ID			0

// 资源目录
// 资源根目录在 "APP_ROOT/Resources"
#define MAP_ROOT							"maps/"
#define IMAGE_ROOT							"images/"
#define SOUND_ROOT							"sounds/"
#define RES_IMAGE(RES_NAME)					IMAGE_ROOT RES_NAME
#define RES_SOUND(RES_NAME)					SOUND_ROOT RES_NAME
#define RES_FONT(RES_NAME)					"fonts/" RES_NAME

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
// 应用包名
#define PACKAGE_NAME "io.gamen.snowfight"
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
// 应用在AppStore的ID
#define APPSTORE_ID "1464511028"
#endif

//////////////////////////////////////////////////////////////////////////
// 分辨率适配
// 当前采用的适配方案是使用一种分辨率（480x320）匹配不同的资源 
//////////////////////////////////////////////////////////////////////////

// 设计分辨率定义
#define DESIGN_RESOLUTION_480X320		0
#define DESIGN_RESOLUTION_960X640		1
#define DESIGN_RESOLUTION_1920X1280		2
#define DESIGN_RESOLUTION_2880X1920		3

// 目标设计分辨率
// 地图纹理必须严格按照内容比例（Director::getContentScaleFactor()）的大小制作。
// 例如： 设计分辨率为480x320, 适配分辨率为1024x768，内容比为2.13。
//       如果要制作一个大小为64x32的地图瓦片纹理，则该纹理在适配分辨率大小应该为 W: 64x2.13=136 H: 32x2.13=68。
#define TARGET_DESIGN_RESOLUTION_SIZE  DESIGN_RESOLUTION_480X320


// 设计分辨率匹配屏幕分辨率的范围
// 例如： 如果 ScreenHeight - mediumResource.size.height > DESIGN_RESOLUTION_MATCHING_RANGE
//       则选择 largeResource.
#define DESIGN_RESOLUTION_MATCHING_RANGE				100


// 资源配置信息
typedef struct tagResource
{
	cocos2d::Size size; // 设计分辨率
	int32 requiredTexSize;  // 设备需要支持的纹理大小
	std::string directory;  // 资源目录

} Resource;

static Resource smallResource		= { cocos2d::Size(480, 320),	1024,	""		}; // SD资源默认存放在资源根目录("APP_ROOT/Resources")
static Resource mediumResource		= { cocos2d::Size(960, 640),	2048,	"hd"	}; // 分辨率是SD的2倍
static Resource largeResource		= { cocos2d::Size(1920, 1280),	4096,	"hd2"	}; // 分辨率是HD的2倍
static Resource xlargeResource		= { cocos2d::Size(2880, 1920),	4096,	"hd3"	}; // 分辨率是HD的3倍

#if (TARGET_DESIGN_RESOLUTION_SIZE == DESIGN_RESOLUTION_480X320)
static cocos2d::Size designResolutionSize = smallResource.size;
#elif (TARGET_DESIGN_RESOLUTION_SIZE == DESIGN_RESOLUTION_960X640)
static cocos2d::Size designResolutionSize = mediumResource.size;
#elif (TARGET_DESIGN_RESOLUTION_SIZE == DESIGN_RESOLUTION_1920X1280)
static cocos2d::Size designResolutionSize = largeResource.size;
#elif (TARGET_DESIGN_RESOLUTION_SIZE == DESIGN_RESOLUTION_2880X1920)
static cocos2d::Size designResolutionSize = xlargeResource.size;
#else
#error unknown target design resolution!
#endif

// 窄屏幕设计分辨率比例
// 比例值是依据宽高比为4:3的设备与宽高比为16:9的设备在具有相同设计分辨率面积时的放大比例（近似值）
#define NARROW_SCREEN_DESIGN_RESOLUTION_SCALE		1.2f

// 窄屏幕设计分辨率
// 基于目标设计分辨率的高度放大后得出，放大后能够在窄屏幕设备上显示更多的内容
static cocos2d::Size narrowDesignResolutionSize = cocos2d::Size(
	designResolutionSize.height * NARROW_SCREEN_DESIGN_RESOLUTION_SCALE / (3 / 4.f),	// 窄屏幕设备的最小宽高比为4:3
	designResolutionSize.height * NARROW_SCREEN_DESIGN_RESOLUTION_SCALE);

//////////////////////////////////////////////////////////////////////////
// 应用字体
//////////////////////////////////////////////////////////////////////////

// The font size 24 is designed for small resolution, so we should change it to fit for current design resolution
#define TITLE_FONT_SIZE  (cocos2d::Director::getInstance()->getOpenGLView()->getDesignResolutionSize().width / smallResource.size.width * 24)

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
#define DEFAULT_SYSTEM_FONT			"sans-serif"
#else
#define DEFAULT_SYSTEM_FONT			"Arial"
#endif


#endif // __APP_DEFINES_H__
