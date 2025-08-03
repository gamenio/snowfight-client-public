#include "ConnectivityHelper.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)

#import <CoreTelephony/CTTelephonyNetworkInfo.h>

#import "Reachability.h"

NS_BEGIN


NetworkType ConnectivityHelper::getNetworkType()
{
    NetworkType networkType = NETWORK_TYPE_UNKNOWN;
    
    Reachability *reachability = [Reachability reachabilityForInternetConnection];
    NetworkStatus status = [reachability currentReachabilityStatus];
    if(status == NotReachable)
    {
        networkType = NETWORK_TYPE_UNKNOWN;
    }
    else if (status == ReachableViaWiFi)
    {
        networkType = NETWORK_TYPE_WIFI;
    }
    else if (status == ReachableViaWWAN)
    {
        CTTelephonyNetworkInfo *networkInfo = [[[CTTelephonyNetworkInfo alloc] init] autorelease];
        NSDictionary<NSString *, NSString *> *currentRadioAccessTechnology = networkInfo.serviceCurrentRadioAccessTechnology;
        for (NSString *service in currentRadioAccessTechnology)
        {
            if (!currentRadioAccessTechnology[service])
            {
              continue;
            }
            NSString *carrierType = currentRadioAccessTechnology[service];
            if ([carrierType isEqualToString:CTRadioAccessTechnologyGPRS]
                || [carrierType isEqualToString:CTRadioAccessTechnologyEdge]
                || [carrierType isEqualToString:CTRadioAccessTechnologyCDMA1x])
            {
                networkType = NETWORK_TYPE_2G;
                break;
            }
            else if ([carrierType isEqualToString:CTRadioAccessTechnologyWCDMA]
                       || [carrierType isEqualToString:CTRadioAccessTechnologyHSDPA]
                       || [carrierType isEqualToString:CTRadioAccessTechnologyHSUPA]
                       || [carrierType isEqualToString:CTRadioAccessTechnologyCDMAEVDORev0]
                       || [carrierType isEqualToString:CTRadioAccessTechnologyCDMAEVDORevA]
                       || [carrierType isEqualToString:CTRadioAccessTechnologyCDMAEVDORevB]
                       || [carrierType isEqualToString:CTRadioAccessTechnologyeHRPD])
            {
                networkType = NETWORK_TYPE_3G;
                break;
            }
            else if ([carrierType isEqualToString:CTRadioAccessTechnologyLTE])
            {
                networkType = NETWORK_TYPE_4G;
                break;
            }
        }
    }
    
    return networkType;
}

NS_END

#endif // CC_TARGET_PLATFORM == CC_PLATFORM_IOS
