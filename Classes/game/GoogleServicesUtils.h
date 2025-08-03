//
// Created by Luthier on 2019/9/19.
//

#ifndef __GOOGLE_SERVICES_UTILS_H__
#define __GOOGLE_SERVICES_UTILS_H__

#include "common/Common.h"

USING_NS_CC;

NS_BEGIN

// Error codes for when a client fails to connect to Google Play services.
// https://developers.google.com/android/reference/com/google/android/gms/common/ConnectionResult
enum ServicesConnectionResult
{
    SERVICE_SUCCESS                     = 0,
    SERVICE_MISSING                     = 1,
    SERVICE_VERSION_UPDATE_REQUIRED     = 2,
    SERVICE_DISABLED                    = 3,
    SERVICE_SIGN_IN_REQUIRED            = 4,
    SERVICE_INVALID_ACCOUNT             = 5,
    SERVICE_RESOLUTION_REQUIRED         = 6,
    SERVICE_NETWORK_ERROR               = 7,
    SERVICE_INTERNAL_ERROR              = 8,
    SERVICE_INVALID                     = 9,
    SERVICE_DEVELOPER_ERROR             = 10,
    SERVICE_LICENSE_CHECK_FAILED        = 11,
    SERVICE_CANCELED                    = 13,
    SERVICE_TIMEOUT                     = 14,
    SERVICE_INTERRUPTED                 = 15,
    SERVICE_API_UNAVAILABLE             = 16,
    SERVICE_SIGN_IN_FAILED              = 17,
    SERVICE_UPDATING                    = 18,
    SERVICE_MISSING_PERMISSION          = 19,
    SERVICE_RESTRICTED_PROFILE          = 20,
};

class GoogleServicesUtils
{
public:
    static int32 checkGooglePlayServices();
    static void showErrorDialog(int32 errorCode);
};

NS_END

#endif //__GOOGLE_SERVICES_UTILS_H__
