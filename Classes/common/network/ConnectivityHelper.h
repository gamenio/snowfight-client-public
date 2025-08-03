#ifndef __CONNECTIVITY_HELPER_H__
#define __CONNECTIVITY_HELPER_H__

#include "common/Common.h"

NS_BEGIN

enum NetworkType
{
    NETWORK_TYPE_UNKNOWN       = 0,
    NETWORK_TYPE_2G            = 1,
    NETWORK_TYPE_3G            = 2,
    NETWORK_TYPE_4G            = 3,
    NETWORK_TYPE_5G            = 4,
    NETWORK_TYPE_WIFI          = 8,
};

class ConnectivityHelper
{
public:
	static NetworkType getNetworkType();

private:
	ConnectivityHelper(){}
	~ConnectivityHelper(){}
};


NS_END


#endif // __CONNECTIVITY_HELPER_H__