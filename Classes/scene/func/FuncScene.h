#ifndef __FUNC_SCENE_H__
#define __FUNC_SCENE_H__

#include "common/Common.h"
#include "game/gamecenter/GameCenter.h"
#include "game/store/Store.h"
#include "game/nts/TimeService.h"
#include "game/firservice/FirebaseService.h"
#include "scene/gui/CheckLabelBox.h"
#include "scene/gui/LoadingView.h"
#include "scene/BaseScene.h"
#include "scene/ParticleEffects.h"
#include "scene/gui/Wicket.h"
#include "scene/gui/FingerTap.h"
#include "share/Share.h"
#include "herostat/HeroStatLayer.h"
#include "FuncNameInputBox.h"
#include "HeroCatwalk.h"
#include "FuncButton.h"
#include "HeroStatButton.h"
#include "PropertyBar.h"
#include "GiftBox.h"

USING_NS_CC;

NS_BEGIN

class FuncScene : public BaseScene, GameCenterListener, StoreListener, ShareListener, TimeSyncListener, RewardedAdListener, InterstitialAdListener
{
public:
	static Scene* createScene();
	static FuncScene* create();
	static FuncScene* create(RewardedAdConfig const& adConfig, bool isEnableInterstitialAd);

	bool init(RewardedAdConfig const& adConfig, bool isEnableInterstitialAd);
	Scene* addToScene();

	void onEnterTransitionDidFinish() override;
	void onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event) override;

private:
	enum FingerTapProcess
	{
		FINGERTAP_PROCESS_NONE,
		FINGERTAP_PROCESS_CLAIM_REWARD,
		FINGERTAP_PROCESS_UPGRADE_STATS,
		FINGERTAP_PROCESS_PLAY,
	};

	enum DailyRewardState
	{
		DAILY_REWARD_STATE_UNKNOWN,
		DAILY_REWARD_STATE_READY,
		DAILY_REWARD_STATE_CLAIMED,
	};

	FuncScene();
	~FuncScene();

	void initForeground();
	ui::Button* createShortcutButton(std::string const& title, std::string const& normalImage);
	void layoutShortcutButtons();

	void setFingerTapProcessCompleted(FingerTapProcess progress);
	void setupFingerTapProcess();
	void startFingerTap(bool isDelayed = true);

	void applyUpgradeStatsTips(bool isDelayed = true);
	void applyClaimRewardTips(bool isDelayed = true);
	void applyFirstWatchAdTips();
	void applyPlayFingerTap(bool isDelayed = true);
	void applyUpgradeStatsSuggestion(bool isDelayed = true);
	void updateRecommendedUpgradeTypes();
	StatUpgradeType recommendUpgradeType();
	bool getRecommendedUpgradeButtonPosition(Vec2& pos);
	void saveSuggestionAccepted(uint32 suggestion, bool accepted);

	void updateGiftBoxState();
	void giveDailyReward(DailyRewardType rewardType, int32 value, bool isDouble);
	void giveOutcomeReward(int32 amount);
	DailyRewardState getDailyRewardState() const;
	bool requestRewardedVideo(RewardedAdConfig const& adConfig);
	bool showInterstitialAdIfNeeded();
	void updateUIAfterAdHidden();

	void updateTotalKills();
	void startGameLoadScene();
	void showShopWicket(bool enableWatchAdHintIfNeeded);

    void applyNicknameToLocalPlayer();
    void updateNameInputBox();
    void nameInputBoxEditCallback(Ref* sender, InputBoxEditEvent event);

	void buyHeroWithGameCoin();
	void buyHeroWithCurrency();
	void unlockHero(HeroID heroId, bool playSoundForCurrentHero = true);
	void giveGameCoin(int32 amount);
	void removeAds(bool isRestored);
	void deliverGoods(bool isRestored, std::string const& productId);

	void buttonPlayCallback(Ref* sender);
	void buttonBuyCallback(Ref* sender, MoneyType type);
	void buttonOptionCallback(Ref* sender);
	void buttonRankingCallback(Ref* sender);
	void buttonShareCallback(Ref* sender);
	void buttonRemoveAdsCallback(Ref* sender);
	void buttonHeroStatCallback(Ref* sender);

	void propertyBarClickCallback(Ref* sender);
	void giftBoxClickCallback(Ref* sender);

	void updateSelectedHero();
	void resetUIForSelectHero();
	void updateUIForSelectedHero();
	void heroCatwalkSelectCallback(Ref* sender, HeroTemplate const* tmpl);
    void heroCatwalkBeginScrollCallback(Ref* sender);
	void heroStatUpgradedCallback(Ref* sender, HeroStatLayer::StatUpgradeAction action);
	void heroStatVisibleStateChangedCallback(Ref* sender, HeroStatLayer::VisibleState state);
	void heroStatUpgradePanelScrollCallback(Ref* sender, StatUpgradePanel::ListScrollEventType eventType);

	void enableUserInteraction(bool enabled);
    void saveUserConfigs();

	void scheduleSnowEffect();
	void startSnowEffect();
    
	void requestPayment(std::string productId, float delay = 0.1f);
    void validateProductIds();
    
    void showLoadingView();
    void dismissLoadingView();
    
    // GameCenterListener
	void onAuthStarted(AuthOperation op) override;
    void onAuthFinished(AuthOperation op, AuthResponse const& response) override;
    void onReportScoreSuccess() override;
    void onReportScoreFail(GameCenterError const& error) override;
#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
    void handleAuthError(GameCenterError error);
#endif // CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID

    // StoreListener
    void onProductsRequestFinished() override;
    void onProductsRequestFailed(StoreError const& error) override;
    void onPaymentTransactionUpdated(PaymentTransaction const& transaction) override;
    void handleTransactionError(StoreError const& error);
#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
    void onPurchasesFailed(StoreError const& error) override;
	void onConsumeFinished(PaymentTransaction const& transaction) override;
	void onConsumeFailed(StoreError const& error) override;
	void onAcknowledgePurchaseFinished(PaymentTransaction const& transaction) override;
	void onAcknowledgePurchaseFailed(StoreError const& error) override;
#endif // CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID

    // ShareListener
    void onShareState(ShareResponse const& response) override;

	// TimeSyncListener
	void onTimeSyncSuccess(int64 millis) override;
	void onTimeSyncFail() override;

	// RewardedAdListener
	void onRewardedAdClosed(RewardedAdConfig const& adConfig, bool isEarnedReward) override;
	void onUserEarnedReward(RewardedAdConfig const& adConfig) override;
	void onRewardedAdRequestFailed(RewardedAdConfig const& adConfig, AdError const& error) override;

	// InterstitialAdListener
    void onInterstitialAdClosed() override;
    void onInterstitialAdLoaded() override;
    void onInterstitialAdRequestFailed(AdError const& error) override;

#if USE_DEBUG_OPTION
	void buttonOptionTouchCallback(Ref* sender, ui::Widget::TouchEventType eventType);
	void onDebugOptionWicketResult(Wicket::ResultCode resultCode, ValueMapIntKey const& data);
#endif
    
	HeroTemplate const* m_currHeroTmpl;
	bool m_isShowingLeaderboard;
	bool m_isEnableInterstitialAd;
	RewardedAdConfig m_rewardedAdConfig;
	bool m_isNarrowScreen;
	float m_safeInsetLeft;
	float m_safeInsetRight;
	FingerTapProcess m_fingerTapProcess;

	Label* m_totalKillsLabel;
    Sprite* m_totalKillsBg;
	Node* m_fgContainer;
	Sprite* m_logoSp;
	Sprite* m_background;
	HeroCatwalk* m_heroCatwalk;
	FuncButton* m_funcBtn;
	FuncNameInputBox* m_nameInputBox;
	ui::Button* m_optionBtn;
	Sprite* m_gpCntlrSp;
	ui::Button* m_rankingBtn;
	ui::Button* m_shareBtn;
	ui::Button* m_removeAdsBtn;
	PropertyBar* m_propertyBar;
    LoadingView* m_loadingView;
	HeroStatButton* m_heroStatBtn;
	HeroStatLayer* m_heroStatLayer;
	FingerTap* m_fingerTap;
	GiftBox* m_giftBox;

	SnowEffect* m_snowEffectBg;
	SnowEffect* m_snowEffectFg;
};


NS_END

#endif // __FUNC_SCENE_H__
