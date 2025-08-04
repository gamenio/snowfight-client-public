#ifndef __UTILS_H__
#define __UTILS_H__

#include "common/Common.h"

USING_NS_CC;

// If the platform supports outline, enable it; otherwise, enable shadow.
#if (CC_TARGET_PLATFORM != CC_PLATFORM_ANDROID) && (CC_TARGET_PLATFORM != CC_PLATFORM_IOS)
#define LABEL_ENABLE_OUTLINE_IF_SUPPORTED(label, color) label->enableShadow(color, cocos2d::Size(1, -1));
#else
#define LABEL_ENABLE_OUTLINE_IF_SUPPORTED(label, color) label->enableOutline(color, 1);
#endif

// Use medium-density screen (160 dpi) as the baseline density.
#define BASELINE_DENSITY                160.f

// On the baseline density screen, how many inches must the user's finger move before the system recognizes the gesture.
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
