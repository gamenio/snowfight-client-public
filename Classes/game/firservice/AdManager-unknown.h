//
//  AdManager-unknown.h
//  snowfight
//
// Created by Luthier on 2020/8/18.
//

#ifndef __AD_MANAGER_UNKNOWN_H__
#define __AD_MANAGER_UNKNOWN_H__

#include "common/Common.h"

#include "AdManagerProtocol.h"

NS_BEGIN

class AdManagerImpl: public AdManagerProtocol
{
public:
    AdManagerImpl() { }
    ~AdManagerImpl() { }
    
    void addRewardedAdListener(RewardedAdConfig::AdType adType, RewardedAdListener* listener) override { }
    void removeRewardedAdListener(RewardedAdListener* listener) override { }

    bool requestShowRewardedVideo(RewardedAdConfig const& adConfig) override { return false; }

	void addInterstitialAdListener(InterstitialAdListener* listener) override {}
	void removeInterstitialAdListener(InterstitialAdListener* listener) override {}

	bool requestInterstitialAd() override { return false; }
	bool showInterstitialAd() override { return false; }

private:

};


NS_END

#endif // __AD_MANAGER_UNKNOWN_H__
