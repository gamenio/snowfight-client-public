#include "ConnectivityHelper.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_MAC)

NS_BEGIN


NetworkType ConnectivityHelper::getNetworkType()
{
	return NetworkType::NETWORK_TYPE_UNKNOWN;
}

NS_END

#endif // CC_TARGET_PLATFORM == CC_PLATFORM_MAC
