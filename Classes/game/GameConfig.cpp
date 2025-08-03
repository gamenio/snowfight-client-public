#include "GameConfig.h"

NS_BEGIN

#if !defined(NS_DEBUG) || NS_DEBUG == 0
#if !defined(_MSC_VER)
#if USE_DEBUG_OPTION
#warning "Debug option is used, but not in debug mode."
#endif // USE_DEBUG_OPTION

#endif // !defined(_MSC_VER)

#endif // !defined(NS_DEBUG) || NS_DEBUG == 0

char const* LOCALPLAYER_DB_KEY = "FwcBnNagD5OI7eQj0vyqB1xMpmvBXghU";

uint8 PAYMENT_DATA_KEY[] = "4uVC0aJZsFk6m6I";
uint32 PAYMENT_DATA_KEY_LENGTH = sizeof(PAYMENT_DATA_KEY);

NS_END
