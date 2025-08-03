#include "LocaleMgr.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)

#include "platform/android/jni/JniHelper.h"

NS_BEGIN


static const std::string helperClassName = "io/gamen/snowfight/AppHelper";

std::string LocaleMgr::getCountryCode() const
{
	std::string countryCode = JniHelper::callStaticStringMethod(helperClassName, "getCountryCode");

	return countryCode;
}

int32 LocaleMgr::getTimeZone() const
{
    int32 timeZone = JniHelper::callStaticIntMethod(helperClassName, "getTimeZone");
    return timeZone;
}

std::vector<std::string> LocaleMgr::getCurrentLangTagComponents() const
{
    std::vector<std::string> components(MAX_LANGETAG_COMPONENTS);
    components[LANGTAG_LANGUAGE_CODE] = Application::getInstance()->getCurrentLanguageCode();
    components[LANGTAG_COUNTRY_CODE] = this->getCountryCode();

    std::string scriptCode = JniHelper::callStaticStringMethod(helperClassName, "getScriptCode");
    components[LANGTAG_SCRIPT_CODE] = scriptCode;

    return components;
}

NS_END

#endif // CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID