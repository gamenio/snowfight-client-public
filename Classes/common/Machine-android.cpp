#include "Machine.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)

#include "platform/android/jni/JniHelper.h"

NS_BEGIN

static const std::string helperClassName = "io/gamen/snowfight/AppHelper";

std::string Machine::getModel() const
{
    std::string model = JniHelper::callStaticStringMethod(helperClassName, "getModel");
    return model;
}

std::string Machine::getOS() const
{
    std::string os = JniHelper::callStaticStringMethod(helperClassName, "getOSVersion");
    return os;
}

std::string Machine::getDeviceGUID() const
{
    std::string guid = JniHelper::callStaticStringMethod(helperClassName, "getDeviceGUID");
    return guid;
}

Rect Machine::getSafeAreaRect() const
{
    bool isSupportDisplayCutout =  JniHelper::callStaticBooleanMethod(helperClassName, "isSupportDisplayCutout");
    if(isSupportDisplayCutout)
    {
        float safeInsetLeft =  JniHelper::callStaticIntMethod(helperClassName, "getSafeInsetLeft");
        float safeInsetRight =  JniHelper::callStaticIntMethod(helperClassName, "getSafeInsetRight");
        float safeInsetTop =  JniHelper::callStaticIntMethod(helperClassName, "getSafeInsetTop");
        float safeInsetBottom =  JniHelper::callStaticIntMethod(helperClassName, "getSafeInsetBottom");

        GLView* glView = Director::getInstance()->getOpenGLView();
        if(glView)
        {
            Size screenSize = glView->getFrameSize();
            Rect viewPortRect = glView->getViewPortRect();
            float scaleX = glView->getScaleX();
            float scaleY = glView->getScaleY();
            Size designResolutionSize = glView->getDesignResolutionSize();

            // Get leftBottom and rightTop point in UI coordinates
            Vec2 leftBottom = Vec2(safeInsetLeft, screenSize.height - safeInsetBottom);
            Vec2 rightTop = Vec2(screenSize.width - safeInsetRight, safeInsetTop);

            // Convert a point from UI coordinates to which in design resolution coordinate.
            leftBottom.x = (leftBottom.x - viewPortRect.origin.x) / scaleX,leftBottom.y = (leftBottom.y - viewPortRect.origin.y) / scaleY;
            rightTop.x = (rightTop.x - viewPortRect.origin.x) / scaleX, rightTop.y = (rightTop.y - viewPortRect.origin.y) / scaleY;

            // Adjust points to make them inside design resolution
            leftBottom.x = MAX(leftBottom.x, 0);
            leftBottom.y = MIN(leftBottom.y, designResolutionSize.height);
            rightTop.x = MIN(rightTop.x, designResolutionSize.width);
            rightTop.y = MAX(rightTop.y, 0);

            // Convert to GL coordinates
            leftBottom = Director::getInstance()->convertToGL(leftBottom);
            rightTop = Director::getInstance()->convertToGL(rightTop);

            return Rect(leftBottom.x, leftBottom.y, rightTop.x - leftBottom.x, rightTop.y - leftBottom.y);
        }
    }

    return Director::getInstance()->getSafeAreaRect();
}

bool Machine::isTablet() const
{
    auto glview = Director::getInstance()->getOpenGLView();
    auto frameSize = glview->getFrameSize();
    float width = frameSize.width  / Device::getDPI();
    float height = frameSize.height  / Device::getDPI();
    float screenDiagonal = std::sqrt(width * width + height * height);

    return screenDiagonal >= 7.0f;
}

void Machine::setMultipleTouchEnabled(bool enabled)
{
    JniHelper::callStaticVoidMethod(helperClassName, "setMultipleTouchEnabled", enabled);
}

bool Machine::isAppInstalled(std::string const& packageName) const
{
    return JniHelper::callStaticBooleanMethod(helperClassName, "isAppInstalled", packageName);
}

void Machine::vibrate(VibrationType type)
{
    switch(type)
    {
    case VibrationType::VIBRATION_TYPE_TOUCH_HOLD:
        // amplitude (int): The strength of the vibration. This must be a value between 1 and 255, or DEFAULT_AMPLITUDE(-1).
        JniHelper::callStaticVoidMethod(helperClassName, "vibrate", 0.02f, -1);
        break;
    }
}

NS_END

#endif // CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID