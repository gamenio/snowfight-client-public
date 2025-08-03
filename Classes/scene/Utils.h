#ifndef __UTILS_H__
#define __UTILS_H__

#include "common/Common.h"

USING_NS_CC;

// 如果平台支持则开启标签的包边效果，否则开启阴影效果
#if (CC_TARGET_PLATFORM != CC_PLATFORM_ANDROID) && (CC_TARGET_PLATFORM != CC_PLATFORM_IOS)
#define LABEL_ENABLE_OUTLINE_IF_SUPPORTED(label, color) label->enableShadow(color, cocos2d::Size(1, -1));
#else
#define LABEL_ENABLE_OUTLINE_IF_SUPPORTED(label, color) label->enableOutline(color, 1);
#endif

// 以中密度屏幕160dpi作为基准密度
#define BASELINE_DENSITY                160.f

// 在基准屏幕上，用户的手指必须移动多少英寸，系统才会识别该手势
// https://developer.android.com/training/multiscreen/screendensities
#define GESTURE_THRESHOLD_INCH			(7.0f/BASELINE_DENSITY)

NS_BEGIN

class Utils
{
public:
	static std::string convertToPercentage(float value, bool isSigned = true, int32 precision = 2);
	static std::string separateThousands(int32 number, bool isSigned = false);

    static void enableBoldForLabel(Label* label);
	static Vec2 drawingOffsetToAnchorPoint(cocos2d::Point const& drawingOffset, cocos2d::Size const& contentSize);

    static float getSafeInsetTop(Rect const& safeArea);
	static float getSafeInsetLeft(Rect const& safeArea);
    static float getSafeInsetBottom(Rect const& safeArea);
	static float getSafeInsetRight(Rect const& safeArea);

	static float convertDistanceFromPointToInch(float pointDis);
    static float convertDistanceFromInchToPoint(float inchDis);

	static void exitApp();
};

NS_END

#endif // __UTILS_H__
