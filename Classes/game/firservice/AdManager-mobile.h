//
//  AdManager-mobile.h
//  snowfight
//
// Created by Luthier on 2020/8/12.
//

#ifndef __AD_MANAGER_MOBILE_H__
#define __AD_MANAGER_MOBILE_H__

#include "firebase/app.h"
#include "firebase/gma.h"
#include "firebase/gma/rewarded_ad.h"
#include "firebase/gma/interstitial_ad.h"

#include "common/Common.h"
#include "AdManagerProtocol.h"

NS_BEGIN

class RewardedVideoStateListener;
class InterstitialAdStateListener;

class AdManagerImpl: public AdManagerProtocol
{
    friend class RewardedVideoStateListener;
    friend class InterstitialAdStateListener;
public:
    AdManagerImpl();
    ~AdManagerImpl();

    void init(firebase::App* app);
    
    void addRewardedAdListener(RewardedAdConfig::AdType adType, RewardedAdListener* listener) override;
    void removeRewardedAdListener(RewardedAdListener* listener) override;

    bool requestShowRewardedVideo(RewardedAdConfig const& adConfig) override;
    bool isRewardedVideoInProgress() const override { return m_isRewardedVideoInProgress; }

    void addInterstitialAdListener(InterstitialAdListener* listener) override;
    void removeInterstitialAdListener(InterstitialAdListener* listener) override;

    bool requestInterstitialAd() override;
    bool showInterstitialAd() override;
    bool isInterstitialAdLoaded() const override { return m_isInterstitialAdLoaded; }

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    bool isNeedTrackingAuthorization() const override;
    void requestIDFA(std::function<void(bool)> const& callback) override;
#endif

private:
    AdErrorCode getAdErrorCode(int32 gmaAdErrorCode);
    firebase::gma::AdRequest createAdRequest();
    
    void loadRewardedVideo();
    void showRewardedVideo();
    void handleRewardedVideoError(int32 error, std::string const& message);

    void onUserEarnedReward(firebase::gma::AdReward const& reward);
    void onRewardedVideoDismissed();
    void onRewardedVideoFailedToShow(firebase::gma::AdError const& adError);

    void loadInterstitialAd();
    void handleInterstitialAdError(int32 error, std::string const& message);

    void onInterstitialAdDismissed();
    void onInterstitialAdFailedToShow(firebase::gma::AdError const& adError);

    template <typename ResultType>
    std::function<void(firebase::Future<ResultType> const&)> handleResultInCocosThread(std::function<void(int32, const char*)> const& callback)
    {
        std::function<void(firebase::Future<ResultType> const&)> func = [callback](firebase::Future<ResultType> const& future)
        {
            cocos2d::Director::getInstance()->getScheduler()->performFunctionInCocosThread([callback, future](){
                callback(future.error(), future.error_message());
            });

        };

        return func;
    }

    bool m_isInitialized;
    bool m_isInitializing;

    bool m_isRewardedVideoInProgress;
    bool m_isEarnedReward;
    firebase::gma::RewardedAd* m_rewardedAd;
    RewardedAdConfig m_rewardedAdConfig;
    RewardedVideoStateListener* m_rewardedVideoStateListener;
    std::unordered_map<int32, RewardedAdListener*> m_rewardedAdListeners;

    bool m_isInterstitialAdLoaded;
    firebase::gma::InterstitialAd* m_interstitialAd;
    std::vector<InterstitialAdListener*> m_interstitialAdListeners;
    InterstitialAdStateListener* m_interstitialAdStateListener;
};

NS_END

#endif // __AD_MANAGER_MOBILE_H__
