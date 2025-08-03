#ifndef __SHOP_WICKET_H__
#define __SHOP_WICKET_H__

#include "cocos/ui/UIListView.h"

#include "common/Common.h"
#include "game/ShopMgr.h"
#include "game/store/Store.h"
#include "game/firservice/FirebaseService.h"
#include "scene/gui/Wicket.h"
#include "scene/gui/LoadingView.h"
#include "GameCoinSaleItem.h"
#include "GameCoinRewardItem.h"

USING_NS_CC;

NS_BEGIN

enum ShopItemState
{
	SHOP_ITEM_STATE_PURCHASED,
	SHOP_ITEM_STATE_REWARDED,
};

typedef std::function<void(Ref* sender, ShopItemState state)> ShopItemStateChangedCallback;

class ShopWicket : public Wicket, StoreListener, RewardedAdListener, TimeSyncListener
{
public:
	static ShopWicket* create(Node* owner);
	virtual bool init(Node* owner);

	ShopWicket();
	~ShopWicket();

	void setGoldDeliveryPoint(cocos2d::Point const& point) { m_goldDeliveryPoint = point; }
	void setShopItemStateChangedEventListener(ShopItemStateChangedCallback const& callback) { m_itemStateChangedCallback = callback; }
	void setWatchAdHintEnabled(bool enabled) { m_isWatchAdHintEnabled = enabled; }

	void onClosed() override;
	void onEnter() override;

private:
	void loadData();
	void updateListItems();
    void reloadGameCoinSaleItems();
    GameCoinItem* getItemByGameCoinId(GameCoinID gameCoinId);

	void giveReward(GameCoinTemplate const* tmpl);
	void deliverGameCoin(GameCoinTemplate const* tmpl, std::function<void(GameCoinItem*)> const& deliveredCallback);
	void buyGameCoin(GameCoinTemplate const* tmpl);
	void playDeliveryAnimation(GameCoinItem* item, std::function<void()> const& complete);

	void watchRewardedVideo(GameCoinTemplate const* tmpl);
	void requestRewardedVideo(GameCoinTemplate const* tmpl);
	void showLoadingView();
	void dismissLoadingView();

	// RewardedAdListener
	void onRewardedAdClosed(RewardedAdConfig const& adConfig, bool isEarnedReward) override;
	void onUserEarnedReward(RewardedAdConfig const& adConfig) override;
	void onRewardedAdRequestFailed(RewardedAdConfig const& adConfig, AdError const& error) override;

	void selectedItemEvent(Ref* pSender, ui::ListView::EventType type);

	void validateProductIds();

	// StoreListener
	void onProductsRequestFinished() override;
	void onProductsRequestFailed(StoreError const& error) override;
#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
	void onConsumeFinished(PaymentTransaction const& transaction) override;
	void onConsumeFailed(StoreError const& error) override;
#elif CC_TARGET_PLATFORM == CC_PLATFORM_IOS
    void onPaymentTransactionUpdated(PaymentTransaction const& transaction) override;
#endif
    
    // TimeSyncListener
    void onTimeSyncSuccess(int64 millis) override;
    void onTimeSyncFail() override;

	void saveSuggestionAccepted(uint32 suggestion, bool accepted);

	cocos2d::Point m_goldDeliveryPoint;
	ShopItemStateChangedCallback m_itemStateChangedCallback;
	Node* m_main;
	ui::ListView* m_listView;
	LoadingView* m_loadingView;
	bool m_isWatchAdHintEnabled;
	bool m_isRequestingRewardedVideo;
	RewardedAdConfig m_rewardedAdConfig;
};

NS_END

#endif // __SHOP_WICKET_H__
