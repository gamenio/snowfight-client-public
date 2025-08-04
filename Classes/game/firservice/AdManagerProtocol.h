//
//  AdManagerProtocol.h
//  snowfight
//
// Created by Luthier on 2020/8/18.
//

#ifndef __AD_MANAGER_PROTOCOL_H__
#define __AD_MANAGER_PROTOCOL_H__

#include "base/CCValue.h"
#include "common/Common.h"

NS_BEGIN

struct RewardedAdConfig
{
    enum AdType
    {
        AD_TYPE_NONE,
        AD_TYPE_FREE_GOLDS,
        AD_TYPE_BATTLE_OUTCOME,
        AD_TYPE_DAILY_REWARD,
    };

    RewardedAdConfig() :
            adType(AD_TYPE_NONE)
    {}

    bool isNull() const { return adType == AD_TYPE_NONE; }
    void reset()
    {
        adType = AD_TYPE_NONE;
		value1 = cocos2d::Value::Null;
		value2 = cocos2d::Value::Null;
    }

    AdType adType;
    cocos2d::Value value1;
	cocos2d::Value value2;
};


enum AdErrorCode
{
    // Call completed successfully.
    AD_ERROR_NONE,
    // The ad has not been fully initialized.
    AD_ERROR_UNINITIALIZED,
    // A call to load an ad has failed due to an internal SDK error.
    AD_ERROR_INTERNAL_ERROR,
    // A call to load an ad has failed due to an invalid request.
    AD_ERROR_INVALID_REQUEST,
    // A call to load an ad has failed due to a network error.
    AD_ERROR_NETWORK_ERROR,
    // A call to load an ad has failed because no ad was available to serve.
    AD_ERROR_NO_FILL,
    // An attempt has been made to show an ad on an Android Activity that has
    // no window token (such as one that's not done initializing).
    AD_ERROR_NO_WINDOW_TOKEN,
    // An attempt to load an Ad Network extras class for an ad request has failed.
    AD_ERROR_AD_NETWORK_CLASS_LOAD_ERROR,
    // The ad server experienced a failure processing the request.
    AD_ERROR_SERVER_ERROR,
    // The current device’s OS is below the minimum required version.
    AD_ERROR_OS_VERSION_TOO_LOW,
    // The request was unable to be loaded before being timed out.
    AD_ERROR_TIMEOUT,
    // Will not send request because the interstitial object has already been used.
    AD_ERROR_INTERSTITIAL_ALREADY_USED,
    // The mediation response was invalid.
    AD_ERROR_MEDIATION_DATA_ERROR,
    // Error finding or creating a mediation ad network adapter.
    AD_ERROR_MEDIATION_ADAPTER_ERROR,
    // Attempting to pass an invalid ad size to an adapter.
    AD_ERROR_MEDIATION_INVALID_AD_SIZE,
    // Invalid argument error.
    AD_ERROR_INVALID_ARGUMENT,
    // Received invalid response.
    AD_ERROR_RECEIVED_INVALID_RESPONSE,
    // Will not send a request because the rewarded ad object has already been used.
    AD_ERROR_REWARDED_AD_ALREADY_USED,
    // A mediation ad network adapter received an ad request, but did not fill.
    // The adapter’s error is included as an underlyingError.
    AD_ERROR_MEDIATION_NO_FILL,
    // Will not send request because the ad object has already been used.
    AD_ERROR_AD_ALREADY_USED,
    // Will not send request because the application identifier is missing.
    AD_ERROR_APPLICATION_IDENTIFIER_MISSING,
    // Android Ad String is invalid.
    AD_ERROR_INVALID_AD_STRING,
    // The ad can not be shown when app is not in the foreground.
    AD_ERROR_APP_NOT_IN_FOREGROUND,
    // A mediation adapter failed to show the ad.
    AD_ERROR_MEDIATION_SHOW_ERROR,
    // The ad is not ready to be shown.
    AD_ERROR_AD_NOT_READY,
    // Ad is too large for the scene.
    AD_ERROR_AD_TOO_LARGE,
    // Attempted to present ad from a non-main thread. This is an internal error which should be reported to support if encountered.
    AD_ERROR_NOT_MAIN_THREAD,
    // A debug operation failed because the device is not in test mode.
    AD_ERROR_NOT_IN_TEST_MODE,
    // An attempt to load the Ad Inspector failed.
    AD_ERROR_INSPECTOR_FAILED_TO_LOAD,
    // The request to show the Ad Inspector failed because it's already open.
    AD_ERROR_INSEPCTOR_ALREADY_OPEN,
    // Fallback error for any unidentified cases.
    AD_ERROR_UNKNOWN,
};

struct AdError
{
    AdError() :
    errorCode(AD_ERROR_NONE),
    errorMsg("")
    { }
    
    AdErrorCode errorCode;
    std::string errorMsg;
};

class RewardedAdListener
{
public:
    virtual void onRewardedAdClosed(RewardedAdConfig const& adConfig, bool isEarnedReward){}
    virtual void onUserEarnedReward(RewardedAdConfig const& adConfig) {}
    virtual void onRewardedAdRequestFailed(RewardedAdConfig const& adConfig, AdError const& error) {}
};

class InterstitialAdListener
{
public:
    virtual void onInterstitialAdClosed(){}
	virtual void onInterstitialAdLoaded() {}
    virtual void onInterstitialAdRequestFailed(AdError const& error) {}
};

class AdManagerProtocol
{
public:
    virtual ~AdManagerProtocol() { }
    
    virtual void addRewardedAdListener(RewardedAdConfig::AdType adType, RewardedAdListener* listener) = 0;
    virtual void removeRewardedAdListener(RewardedAdListener* listener) = 0;

    virtual bool requestShowRewardedVideo(RewardedAdConfig const& adConfig) = 0;
    virtual bool isRewardedVideoInProgress() const { return false; }

    virtual void addInterstitialAdListener(InterstitialAdListener* listener) = 0;
    virtual void removeInterstitialAdListener(InterstitialAdListener* listener) = 0;

    virtual bool requestInterstitialAd() = 0;
    virtual bool showInterstitialAd() = 0;
    virtual bool isInterstitialAdLoaded() const { return false; }

	// Is user authorization required to use IDFA. Only applicable to iOS.
    virtual bool isNeedTrackingAuthorization() const { return false; }
    // Request user authorization to use IDFA. For iOS 14 and above.
    virtual void requestIDFA(std::function<void(bool)> const& callback) {}
};

NS_END

#endif // __AD_MANAGER_PROTOCOL_H__
