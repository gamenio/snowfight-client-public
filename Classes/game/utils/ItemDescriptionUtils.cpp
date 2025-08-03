#include "ItemDescriptionUtils.h"

#include <regex>
#include <iostream>

#include "game/ObjectMgr.h"
#include "game/LocaleMgr.h"

NS_BEGIN

uint32 const DOLLAR = 0;
uint32 const SPECIFIER = 1;
uint32 const FIELD_NUMBER_START = 2;

int32 getValueIndex(std::string const& word)
{
	std::string num = word.substr(FIELD_NUMBER_START, word.length() - FIELD_NUMBER_START);
	return std::stoi(num) - 1;
}

std::string ItemDescriptionUtils::formatDuration(int32 duration)
{
	std::ostringstream oss;
	int32 min = duration / 60;
	int32 sec = duration % 60;
	if (min > 0)
		oss << min << sLocaleMgr->getString("item_desc_time_min");
	oss << sec << sLocaleMgr->getString("item_desc_time_sec");

	return oss.str();
}

std::string ItemDescriptionUtils::formatUnsignedInt(int32 value)
{
	std::ostringstream oss;
	oss << (uint32)std::abs(value);

	return oss.str();
}

std::string ItemDescriptionUtils::format(ItemTemplate const* tmpl, ItemLocale const* locale)
{
	ItemApplicationTemplate const* appTmpl = sObjectMgr->getItemApplicationTemplate(tmpl->appId);

	CCLOG("ItemDescriptionUtils: Format: %s", locale->description.c_str());

	std::regex wordRegex("\\$[a-zA-Z]\\d*");
	auto wordsBegin = std::sregex_iterator(locale->description.begin(), locale->description.end(), wordRegex);
	auto wordsEnd = std::sregex_iterator();

	CCLOG("ItemDescriptionUtils: Found %d words", (int32)std::distance(wordsBegin, wordsEnd));

	std::string result = locale->description;
	std::ptrdiff_t offset = 0;
	for (std::sregex_iterator i = wordsBegin; i != wordsEnd; ++i)
	{
		std::smatch match = *i;
		auto pos = match.position();
		auto length = match.length();
		CCLOG("ItemDescriptionUtils: %d-%d\t%s", (int32)pos, (int32)(pos + length), match.str().c_str());
		std::string word = match.str(0);
        NS_UNUSED_VARIABLE(DOLLAR);
		NS_ASSERT(word[DOLLAR] == '$');

		char specifier = word[SPECIFIER];
		std::ostringstream oss;
		bool ignored = true;

		// 物品应用持续时间
		// 格式: $d
		if (specifier == 'd')
		{
			if (appTmpl)
			{
				oss << formatDuration(appTmpl->duration);
				ignored = false;
			}
		}
		// 物品属性值
		// 格式: s<fieldnumber>
		// fieldnumber：字段编号
		else if (specifier == 's')
		{
			int32 statIndex = getValueIndex(word);
			if (statIndex >= 0 && statIndex < (int32)tmpl->itemStats.size())
			{
				ItemStat const& stat = tmpl->itemStats[statIndex];
				oss << formatUnsignedInt(stat.value);
				ignored = false;
			}
		}
		// 物品效果值
		// 格式: e<fieldnumber>
		// fieldnumber：字段编号
		else if (specifier == 'e')
		{
			if (appTmpl)
			{
				int32 effectIndex = getValueIndex(word);
				if (effectIndex >= 0 && effectIndex < (int32)appTmpl->effects.size())
				{
					ItemEffect const& effect = appTmpl->effects[effectIndex];
					oss << formatUnsignedInt(effect.value);
					ignored = false;
				}
			}
		}
		if (!ignored)
		{
			std::string fs = oss.str();
			result.replace(pos + offset, length, fs);
			offset = offset + fs.length() - length;
		}
		else
			CCLOG("ItemDescriptionUtils: Word '%s' is ignored.", word.c_str());
	}

	return result;
}

NS_END
