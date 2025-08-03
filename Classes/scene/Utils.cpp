#include "Utils.h"

NS_BEGIN

std::string Utils::convertToPercentage(float value, bool isSigned, int32 precision)
{
	float percentage = value * 100;
	std::string result;
	if(isSigned)
		result = StringUtils::format("%+.*f", precision, percentage);
	else
		result = StringUtils::format("%.*f", precision, percentage);
	size_t pos = result.find_last_of('.');
	if (pos != std::string::npos)
	{
		for (size_t i = result.length() - 1; i >= pos; --i)
		{
			char ch = result[i];
			if (ch == '0' || ch == '.')
				result.erase(i, 1);
			else
				break;
		}
	}
	result += '%';
	return result;
}

std::string Utils::separateThousands(int32 number, bool isSigned)
{
	std::string result = std::to_string(number);
	int32 len = (int32)result.length();
	for (int32 i = len - 3; i >= 1; i -= 3) {
		result.insert(i, ",");
	}
	if (isSigned)
	{
		if (number > 0)
			result.insert(0, 1, '+');
		else if(number < 0)
			result.insert(0, 1, '-');
	}

	return result;
}

void Utils::enableBoldForLabel(Label* label)
{
    float fontSize = label->getSystemFontSize();
    if(fontSize >= 13)
        label->enableShadow(label->getTextColor(), Size(0.5f, 0), 0);
    else if(fontSize >= 10)
        label->enableShadow(label->getTextColor(), Size(0.4f, 0), 0);
    else
        label->enableShadow(label->getTextColor(), Size(0.3f, 0), 0);
}

Vec2 Utils::drawingOffsetToAnchorPoint(Point const& drawingOffset, Size const& contentSize)
{
	Vec2 anchorPoint;
	anchorPoint.x = std::abs(drawingOffset.x) / contentSize.width;
	anchorPoint.y = std::abs(drawingOffset.y) / contentSize.height;
	return anchorPoint;
}

float Utils::getSafeInsetTop(Rect const& safeArea)
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    float safeInsetTop = std::round(origin.y + visibleSize.height - safeArea.getMaxY());

    return safeInsetTop;
}

float Utils::getSafeInsetLeft(Rect const& safeArea)
{
	Vec2 origin = Director::getInstance()->getVisibleOrigin();
	float safeInsetLeft = std::round(safeArea.origin.x - origin.x);
	return safeInsetLeft;
}

float Utils::getSafeInsetBottom(Rect const& safeArea)
{
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    float safeInsetBottom = std::round(safeArea.origin.y - origin.y);
    return safeInsetBottom;
}

float Utils::getSafeInsetRight(Rect const& safeArea)
{
	auto visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();

	float safeInsetLeft = getSafeInsetLeft(safeArea);
	float safeInsetRight = std::round(origin.x + visibleSize.width - safeArea.getMaxX());
	if (safeInsetLeft > 0 && safeInsetRight <= 0)
		safeInsetRight = safeInsetLeft;

	return safeInsetRight;
}

float Utils::convertDistanceFromPointToInch(float pointDis)
{
	auto glview = Director::getInstance()->getOpenGLView();
	float factor = (glview->getScaleX() + glview->getScaleY()) / 2;
	return pointDis * factor / Device::getDPI();
}

float Utils::convertDistanceFromInchToPoint(float inchDis)
{
    auto glview = Director::getInstance()->getOpenGLView();
    float factor = (glview->getScaleX() + glview->getScaleY()) / 2;
    return inchDis * Device::getDPI() / factor;
}

void Utils::exitApp()
{
	Director::getInstance()->end();
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
	exit(0);
#endif

}

NS_END

