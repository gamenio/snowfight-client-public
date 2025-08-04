#ifndef __APP_DEFINES_H__
#define __APP_DEFINES_H__

#include "cocos2d.h"

#include "Defines.h"
#include "PublicMacros.h"

//////////////////////////////////////////////////////////////////////////
// Application Configuration
//////////////////////////////////////////////////////////////////////////

NS_BEGIN

int32 getBuildNumber();

// pvr.czz file encryption key
extern uint32 PVR_ENCRYPTION_KEY[4];

// Map data decompression key
extern const char MAPDATA_KEY[];

NS_END


// Application version number
//    Format:        0x00  Major  Minor  Revision
//                   00  01     00     00
#define APP_VERSION				0x00030200

// Build version number
#define BUILD_NUMBER				getBuildNumber()

// Channel ID
// 0 No channel ID specified, 1-1000 are reserved channel IDs.
#define CHANNEL_ID			0

// Resource directory
// The resource root directory is located in “APP_ROOT/Resources”.
#define MAP_ROOT							"maps/"
#define IMAGE_ROOT							"images/"
#define SOUND_ROOT							"sounds/"
#define RES_IMAGE(RES_NAME)					IMAGE_ROOT RES_NAME
#define RES_SOUND(RES_NAME)					SOUND_ROOT RES_NAME
#define RES_FONT(RES_NAME)					"fonts/" RES_NAME

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
// Application package name
#define PACKAGE_NAME "io.gamen.snowfight"
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
// Application ID in App Store
#define APPSTORE_ID "1464511028"
#endif

//////////////////////////////////////////////////////////////////////////
// Adaptation Resolution 
// The current adaptation scheme uses a single resolution (480x320) to match different resources.
//////////////////////////////////////////////////////////////////////////

// Design resolution definitions
#define DESIGN_RESOLUTION_480X320		0
#define DESIGN_RESOLUTION_960X640		1
#define DESIGN_RESOLUTION_1920X1280		2
#define DESIGN_RESOLUTION_2880X1920		3

// Target design resolution
// Map textures must be created strictly according to the content scale factor (Director::getContentScaleFactor()).
// For example: Design resolution is 480x320, adaptive resolution is 1024x768, and content ratio is 2.13.
//              If you want to create a map tile texture with a size of 64x32, the texture size at adaptive resolution should be W: 64x2.13=136 H: 32x2.13=68.
#define TARGET_DESIGN_RESOLUTION_SIZE  DESIGN_RESOLUTION_480X320


// Design resolution matching screen resolution range
// For example: If ScreenHeight - mediumResource.size.height > DESIGN_RESOLUTION_MATCHING_RANGE, then select largeResource.
#define DESIGN_RESOLUTION_MATCHING_RANGE				100


// Resource configuration information
typedef struct tagResource
{
	cocos2d::Size size; // Design resolution
	int32 requiredTexSize;  // Texture size required by device
	std::string directory;  // Resource directory

} Resource;

static Resource smallResource		= { cocos2d::Size(480, 320),	1024,	""		}; // SD resources are stored by default in the resource root directory ("APP_ROOT/Resources").
static Resource mediumResource		= { cocos2d::Size(960, 640),	2048,	"hd"	}; // The resolution is 2 times that of SD
static Resource largeResource		= { cocos2d::Size(1920, 1280),	4096,	"hd2"	}; // The resolution is 2 times that of HD
static Resource xlargeResource		= { cocos2d::Size(2880, 1920),	4096,	"hd3"	}; // The resolution is 3 times that of HD

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

// Narrow screen design resolution scale.
// The aspect ratio is based on the magnification scale (approximate value) between devices
// with an aspect ratio of 4:3 and devices with an aspect ratio of 16:9 when they have the same design resolution area.
#define NARROW_SCREEN_DESIGN_RESOLUTION_SCALE		1.2f

// Narrow screen design resolution
// The size is calculated after enlarging the target design resolution height.
// Enlargement allows more content to be displayed on narrow-screen devices.
static cocos2d::Size narrowDesignResolutionSize = cocos2d::Size(
	designResolutionSize.height * NARROW_SCREEN_DESIGN_RESOLUTION_SCALE / (3 / 4.f),	// The minimum aspect ratio for narrow screen devices is 4:3
	designResolutionSize.height * NARROW_SCREEN_DESIGN_RESOLUTION_SCALE);

//////////////////////////////////////////////////////////////////////////
// Application Font
//////////////////////////////////////////////////////////////////////////

// The font size 24 is designed for small resolution, so we should change it to fit for current design resolution
#define TITLE_FONT_SIZE  (cocos2d::Director::getInstance()->getOpenGLView()->getDesignResolutionSize().width / smallResource.size.width * 24)

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
#define DEFAULT_SYSTEM_FONT			"sans-serif"
#else
#define DEFAULT_SYSTEM_FONT			"Arial"
#endif


#endif // __APP_DEFINES_H__
