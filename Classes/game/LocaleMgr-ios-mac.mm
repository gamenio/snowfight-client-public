#include "LocaleMgr.h"

#if ((CC_TARGET_PLATFORM == CC_PLATFORM_IOS) || (CC_TARGET_PLATFORM == CC_PLATFORM_MAC))

NS_BEGIN

std::string LocaleMgr::getCountryCode() const
{
	NSLocale* currentLocale = [NSLocale currentLocale];
	NSString* countryCode = [currentLocale objectForKey : NSLocaleCountryCode];

    return [countryCode cStringUsingEncoding: NSASCIIStringEncoding];
}

int32 LocaleMgr::getTimeZone() const
{
    int32 offset = (int32)([[NSTimeZone systemTimeZone] secondsFromGMT]);
    return offset;
}

std::vector<std::string> LocaleMgr::getCurrentLangTagComponents() const
{
    std::vector<std::string> components(MAX_LANGETAG_COMPONENTS);
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    NSArray *languages = [defaults objectForKey:@"AppleLanguages"];
    NSString *currentLanguage = [languages objectAtIndex:0];

    NSDictionary* temp = [NSLocale componentsFromLocaleIdentifier:currentLanguage];
    NSString *languageCode = [temp objectForKey:NSLocaleLanguageCode];
    NSString *countryCode = [temp objectForKey: NSLocaleCountryCode];
    NSString *scriptCode = [temp objectForKey: NSLocaleScriptCode];
    if(languageCode != nil)
        components[LANGTAG_LANGUAGE_CODE] = [languageCode cStringUsingEncoding:NSASCIIStringEncoding];
    if(countryCode != nil)
        components[LANGTAG_COUNTRY_CODE] = [countryCode cStringUsingEncoding:NSASCIIStringEncoding];
    if(scriptCode != nil)
        components[LANGTAG_SCRIPT_CODE] = [scriptCode cStringUsingEncoding:NSASCIIStringEncoding];

    return components;
}

NS_END

#endif // (CC_TARGET_PLATFORM == CC_PLATFORM_IOS) || (CC_TARGET_PLATFORM == CC_PLATFORM_MAC)
