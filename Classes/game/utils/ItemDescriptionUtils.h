#ifndef __ITEM_DESCRIPTION_UTILS_H__
#define __ITEM_DESCRIPTION_UTILS_H__

#include "common/Common.h"
#include "game/entities/DataItem.h"

NS_BEGIN

class ItemDescriptionUtils
{
public:
	static std::string formatDuration(int32 duration);
	static std::string formatUnsignedInt(int32 value);

	static std::string format(ItemTemplate const* tmpl, ItemLocale const* locale);
};

NS_END

#endif // __ITEM_DESCRIPTION_UTILS_H__