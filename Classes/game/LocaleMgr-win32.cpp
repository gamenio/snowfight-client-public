#include "LocaleMgr.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)

NS_BEGIN

std::string LocaleMgr::getCountryCode() const
{
	char code[3] = { 0 };
	GetLocaleInfoA(LOCALE_USER_DEFAULT, LOCALE_SISO3166CTRYNAME, code, sizeof(code));
	code[2] = '\0';
	return code;
}

int32 LocaleMgr::getTimeZone() const
{
	return 0;
}


std::vector<std::string> LocaleMgr::getCurrentLangTagComponents() const
{
	std::vector<std::string> components(MAX_LANGETAG_COMPONENTS);
	components[LANGTAG_LANGUAGE_CODE] = Application::getInstance()->getCurrentLanguageCode();
	components[LANGTAG_COUNTRY_CODE] = this->getCountryCode();

	return components;
}


NS_END

#endif // CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
