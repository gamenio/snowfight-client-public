//
//  StoreError.h
//  snowfight
//
//  Created by Luthier on 2019/5/16.
//

#ifndef __STORE_ERROR_H__
#define __STORE_ERROR_H__

#include "common/Common.h"

USING_NS_CC;

NS_BEGIN

enum StoreErrorCode
{
    STORE_ERROR_NONE,
    STORE_ERROR_CLIENT_INVALID,                 // 不允许客户端执行尝试的操作
    STORE_ERROR_CANCELLED,                      // 表示用户取消了付款请求或其他请求的操作
    STORE_ERROR_STORE_PRODUCT_NOT_AVAILABLE,    // 表示请求的商品在商店中不可用
    STORE_ERROR_UNKNOWN,                        // 发生了未知或意外错误
    STORE_ERROR_VALIDATION_FAILED,              // 票据无效或交易未通过验证

#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS
    STORE_ERROR_URL_CANNOT_CONNECT_TO_HOST,     // 无法连接到主机
    STORE_ERROR_URL_TIMED_OUT,                  // 连接超时
    STORE_ERROR_URL_UNKNOWN,                    // 网络连接因未知原因而失败
    STORE_ERROR_INVALID,                        // 表示AppStore无法识别其中一个付款参数。例如：无效的商品ID
    STORE_ERROR_NOT_ALLOWED,                    // 表示不允许用户授权付款

#elif CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
    STORE_ERROR_SERVICE_DISCONNECTED,           // 该应用未通过Google Play结算库连接到PlayStore服务。
    STORE_ERROR_SERVICE_UNAVAILABLE,            // 服务当前不可用。
    STORE_ERROR_FEATURE_NOT_SUPPORTED,          // 当前设备上的Play Store不支持所请求的功能。
    STORE_ERROR_ITEM_ALREADY_OWNED,             // 购买失败，因为已拥有该物品。
    STORE_ERROR_ITEM_NOT_OWNED,                 // 由于该物品不属于用户，因此对该物品请求的操作失败。
    STORE_ERROR_NETWORK_ERROR,                  // 操作期间发生网络错误。

#endif
};

struct StoreError
{
    StoreError() :
    code(STORE_ERROR_NONE),
    description("")
    { }
    
    StoreErrorCode code;
    std::string description;
};

NS_END

#endif // __STORE_ERROR_H__
