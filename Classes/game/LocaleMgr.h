#ifndef __LOCALE_MGR_H__
#define __LOCALE_MGR_H__

#include "json/document-wrapper.h"

#include "common/Common.h"
#include "entities/DataItem.h"

USING_NS_CC;

NS_BEGIN

// Languages supported by the application
enum LangType
{
	LANG_enUS,
	TOTAL_LANGS
};

// Language tag component
enum LangTagComponent
{
    LANGTAG_COUNTRY_CODE,
    LANGTAG_LANGUAGE_CODE,
    LANGTAG_SCRIPT_CODE,
    MAX_LANGETAG_COMPONENTS
};

struct CountryInfo
{
	CountryInfo() : 
		id(0), 
		code(""), 
		name("") { }
	CountryInfo(int32 _id, std::string _code, std::string _name) :
		id(_id),
		code(_code),
		name(_name) { }
	int32 id;
	std::string code;
	std::string name;
};

class LocaleMgr
{
public:
	static LocaleMgr* instance();

	void loadGeneral();

	void loadWorld();
	void unloadWorld();

	LangType getLangType() const { return m_langType; }
	// Get the language tag for BCP-47 standard. Format: Language-Region, for example: zh-CN.
	std::string getLangTag() const;
	std::string getLangTagByType(LangType lang) const;
	// Get the two-letter country code, following the ISO 3166-1 standard.
	std::string getCountryCode() const;
    
	// Get the number of seconds offset by UTC time.
    int32 getTimeZone() const;
    
	// Get the component for the current language tag (Following the BCP-47 standard) of the system.
	// For example, if the system language is zh-CN, the returned array will include: language code zh and country code CN.
    std::vector<std::string> getCurrentLangTagComponents() const;

	std::vector<CountryInfo> const& getCountryList() const { return m_countryList; }
	std::string getCountryName(std::string countryCode);

	std::string getString(std::string const& key);
	std::string getHeroName(uint32 heroId) const;
	std::string getGameCoinName(int32 gameCoinId) const;
	ItemLocale const* getItemLocale(uint32 itemId) const;

	std::string getImagePath(std::string const& resName) const;

private:
	LocaleMgr();
	~LocaleMgr();

	void setupLanguage();

	void loadGeneralStrings();
	void loadCountryList();
	void loadHeroNames();
	void loadGameCoinNames();
	void loadItemLocales();

	bool praseJSONData(rapidjson::Document& doc, std::string const& resName);

	LangType m_langType;

	std::unordered_map<std::string, std::string> m_generalStrings;
	std::unordered_map<uint32/* HeroID */, std::string> m_heroNames;
	std::unordered_map<int32/* GameCoinID */, std::string> m_gameCoinNames;
	std::vector<CountryInfo> m_countryList;
	std::unordered_map<uint32 /* ItemID */, ItemLocale> m_itemLocales;
};

#define sLocaleMgr LocaleMgr::instance()

NS_END



#endif // __LOCALE_MGR_H__
