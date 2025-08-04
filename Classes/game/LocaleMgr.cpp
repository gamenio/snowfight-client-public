#include "LocaleMgr.h"

#include "platform/CCFileUtils.h"
#include "json/error/en.h"
#include "json/error/error.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
#ifdef GetObject
#undef GetObject
#endif

#endif

NS_BEGIN

#define LANG_FILENAME_FORMAT						"%s/%s.json"

// Define language tags supported by the application
static char const* sLangTags[TOTAL_LANGS] =
{
	"en-US",
	"zh-CN",
};
#define DEFAULT_LANG_TYPE							LANG_enUS

LocaleMgr::LocaleMgr() :
	m_langType(DEFAULT_LANG_TYPE)
{
	this->setupLanguage();
}

LocaleMgr::~LocaleMgr()
{
}

void LocaleMgr::setupLanguage()
{
    int32 langType = UserDefault::getInstance()->getIntegerForKey("language", -1);
    if(langType >= 0 && langType < TOTAL_LANGS)
        m_langType = (LangType)langType;
    else
    {
        auto components = this->getCurrentLangTagComponents();
        std::string languageCode = components[LANGTAG_LANGUAGE_CODE];
        std::string countryCode = components[LANGTAG_COUNTRY_CODE];
        std::string scriptCode = components[LANGTAG_SCRIPT_CODE];
        if(languageCode == "en")
        {
            m_langType = LANG_enUS;
        }
        else if(languageCode == "zh")
        {
            if(scriptCode == "Hans" || countryCode == "CN")
                m_langType = LANG_zhCN;
        }
    }
}

LocaleMgr* LocaleMgr::instance()
{
	static LocaleMgr instance;
	return &instance;
}


bool LocaleMgr::praseJSONData(rapidjson::Document& doc, std::string const& resName)
{
	std::string filename = StringUtils::format(LANG_FILENAME_FORMAT, sLangTags[m_langType], resName.c_str());
	bool isDefaultPopupNotify = FileUtils::getInstance()->isPopupNotify();
	FileUtils::getInstance()->setPopupNotify(false);
	bool isExist = FileUtils::getInstance()->isFileExist(filename);
	if (!isExist)
	{
		filename = StringUtils::format(LANG_FILENAME_FORMAT, sLangTags[DEFAULT_LANG_TYPE], resName.c_str());
		isExist = FileUtils::getInstance()->isFileExist(filename);
	}
	FileUtils::getInstance()->setPopupNotify(isDefaultPopupNotify);
	if (isExist)
	{
		std::string data = FileUtils::getInstance()->getStringFromFile(filename);
		rapidjson::ParseResult ret = doc.Parse<rapidjson::kParseCommentsFlag | rapidjson::kParseTrailingCommasFlag>(data.c_str());
		if (ret)
			return true;
		else
			CCLOG("LocaleMgr: JSON '%s' parsing error: %s (%u)", filename.c_str(), rapidjson::GetParseError_En(ret.Code()), (uint32)ret.Offset());
	}
	else
		CCLOG("LocaleMgr: JSON '%s' does not exist.", filename.c_str());


	return false;
}



void LocaleMgr::loadGeneral()
{
	this->loadGeneralStrings();
	this->loadCountryList();
	this->loadHeroNames();
	this->loadGameCoinNames();
}

void LocaleMgr::loadWorld()
{
	this->loadItemLocales();
}

void LocaleMgr::unloadWorld()
{
	m_itemLocales.clear();
}

std::string LocaleMgr::getLangTag() const
{
	return sLangTags[m_langType];
}

std::string LocaleMgr::getLangTagByType(LangType lang) const
{
	if(lang < TOTAL_LANGS)
		return sLangTags[lang];
	return sLangTags[DEFAULT_LANG_TYPE];
}

std::string LocaleMgr::getString(std::string const& key)
{
	auto it = m_generalStrings.find(key);
	if (it != m_generalStrings.end())
		return (*it).second;

	return "NULL";
}

std::string LocaleMgr::getHeroName(uint32 heroId) const
{
	auto it = m_heroNames.find(heroId);
	if (it != m_heroNames.end())
		return (*it).second;

	return "NULL";
}

std::string LocaleMgr::getGameCoinName(int32 gameCoinId) const
{
	auto it = m_gameCoinNames.find(gameCoinId);
	if (it != m_gameCoinNames.end())
		return (*it).second;

	return "NULL";
}

ItemLocale const* LocaleMgr::getItemLocale(uint32 itemId) const
{
	auto it = m_itemLocales.find(itemId);
	if (it != m_itemLocales.end())
		return &(*it).second;
	
	return nullptr;
}

std::string LocaleMgr::getImagePath(std::string const& resName) const
{
	std::string filename = StringUtils::format("%s%s/%s", IMAGE_ROOT, sLangTags[m_langType], resName.c_str());
	if (!FileUtils::getInstance()->isFileExist(filename))
		filename = StringUtils::format("%s%s", IMAGE_ROOT, resName.c_str());

	return filename;
}

void LocaleMgr::loadGeneralStrings()
{
	rapidjson::Document document;
	bool ret = this->praseJSONData(document, "general");

	if (ret && document.IsObject())
	{
		for (rapidjson::Value::ConstMemberIterator itr = document.MemberBegin();
			itr != document.MemberEnd(); ++itr)
		{
			m_generalStrings[itr->name.GetString()] = itr->value.GetString();
		}

	}
}

void LocaleMgr::loadCountryList()
{
	rapidjson::Document document;
	bool ret = this->praseJSONData(document, "countries");

	if (ret && document.IsArray())
	{
		int32 countryIdCounter = 0;
		auto array = document.GetArray();
		for (auto& v : array)
		{
			char const* countryCode = v["code"].GetString();
			char const* countryName = v["name"].GetString();
			m_countryList.emplace_back(++countryIdCounter, countryCode, countryName);
		}
	}
}

void LocaleMgr::loadHeroNames()
{
	rapidjson::Document document;
	bool ret = this->praseJSONData(document, "hero");

	if (ret && document.IsObject())
	{
		for (rapidjson::Value::ConstMemberIterator itr = document.MemberBegin();
			itr != document.MemberEnd(); ++itr)
		{
			m_heroNames[(uint32)std::stoul(itr->name.GetString())] = itr->value.GetString();
		}

	}
}

void LocaleMgr::loadGameCoinNames()
{
	rapidjson::Document document;
	bool ret = this->praseJSONData(document, "game_coin");

	if (ret && document.IsObject())
	{
		for (rapidjson::Value::ConstMemberIterator itr = document.MemberBegin();
			itr != document.MemberEnd(); ++itr)
		{
			m_gameCoinNames[std::stoi(itr->name.GetString())] = itr->value.GetString();
		}

	}
}

void LocaleMgr::loadItemLocales()
{
	rapidjson::Document document;
	bool ret = this->praseJSONData(document, "item");

	if (ret && document.IsObject())
	{
		for (rapidjson::Value::ConstMemberIterator itr = document.MemberBegin();
			itr != document.MemberEnd(); ++itr)
		{
			ItemLocale& locale = m_itemLocales[std::stoi(itr->name.GetString())];
			if (itr->value.IsObject())
			{
				auto obj = itr->value.GetObject();
				locale.name = obj["name"].GetString();
				locale.description = obj["description"].GetString();
			}
		}

	}
}

std::string LocaleMgr::getCountryName(std::string countryCode)
{
	for (auto& info : m_countryList)
	{
		if (info.code == countryCode)
			return info.name;
	}

	return "UNKNOWN";
}


NS_END

