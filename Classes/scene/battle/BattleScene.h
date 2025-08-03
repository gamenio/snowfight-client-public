#ifndef __BATTLE_SCENE_H__
#define __BATTLE_SCENE_H__

#include <bitset>

#include "common/Common.h"
#include "game/World.h"
#include "game/gamecenter/GameCenterProtocol.h"
#include "game/firservice/AdManagerProtocol.h"
#include "game/UserPreferences.h"
#include "scene/TutorialService.h"
#include "scene/BaseScene.h"
#include "scene/gui/LoadingView.h"
#include "scene/GameMapLayer.h"
#include "scene/GuidepostLayer.h"
#include "BattleSceneCommon.h"
#include "SmileyBox.h"
#include "gamepad/GamePad.h"
#include "StatusBar.h"
#include "Toaster.h"
#include "PreparationTimer.h"
#include "SignalIndicator.h"
#include "dangerzone/DangerZone.h"
#include "InventoryBar.h"
#include "ScreenGlowBorder.h"
#include "Minimap.h"
#include "BattleTimer.h"
#include "tutorial/TutorialLayer.h"
#include "MessageBar.h"

USING_NS_CC;

NS_BEGIN

class BattleScene : public BaseScene, GameCenterListener, GamePadListener, 
									  WorldLifecycleListener, 
									  WorldInitListener,
									  NetworkStatusListener,
									  WorldAuthListener,
									  MessageListener,
									  BattleUpdateListener,
									  MyCharacterListener,
									  WorldStatusListener,
									  ItemApplicationListener,
									  SafeZoneListener
{
public:
	static BattleScene* create(RegionalInfo const& regionalInfo, PlayerProfile const& playerProfile, SafeZone* safeZone);

    bool init(RegionalInfo const& regionalInfo, PlayerProfile const& playerProfile, SafeZone* safeZone);
	Scene* addToScene();

	BattleScene();
	~BattleScene();

	void onEnter() override;
	void onExit() override;

	virtual void update(float delta) override;
	void onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event) override;

	InventoryBar* getInventoryBar() const { return m_inventoryBar; }
	GameMapLayer* getGameMapLayer() const { return m_gameMapLayer; }
	StatusBar* getStatusBar() const { return m_statusBar; }
	SmileyBox* getSmileyBox() const { return m_smileyBox; }
	ui::Button* getEquipmentButton() const { return m_equipmentBtn; }
	GamePad* getGamePad() const { return m_gamePad; }

	void stopWorldAndPlayNext();

private:
	enum BattleEventType
	{
		BATTLE_EVENT_DANGER_ALERT,
		BATTLE_EVENT_SHOWDOWN,
		MAX_BATTLE_EVENTS,
	};

	void initBattleground();

	// WorldLifecycleListener
	void onWorldStopped(World* world) override;

	// NetworkStatusListener
	void onNetworkError(NetworkError const& error) override;
	void onNetworkRestored() override;

	// WorldAuthListener
	void onWorldSessionTimedout() override;
	void onWorldAuthSucceeded() override;
	void onWorldSessionExpired() override;
	void onWorldRecvQueueFulL() override;

	// MessageListener
	void onFlashMessage(FlashMessage const& flashMsg) override;
	void onPlayerActionMessage(PlayerActionMessage const& message) override;
	void onDeathMessage(DeathMessage const& message) override;

	// BattleUpdateListener
	void onBattleUpdate(BattleUpdate const& update) override;

	// MyCharacterListener
	void onInitSelfCompleted(DataPlayer* myChar) override;
	void onBattleResult(BattleResult const& result) override;
	
	// ItemApplicationListener
	void onItemApplicationUpdate(ItemApplicationUpdate const& update) override;
	void onItemApplicationUpdateAll(ItemApplicationUpdateAll const& updateAll) override;

	// SafeZoneListener
	void onSafeZoneUpdated(int32 currRadius) override;

	void handleSelfItemApplicationInfo(ItemApplicationInfo const& info);
	void applyItemForSelf(ItemApplicationTemplate const* appTmpl, int32 duration, int32 remainingTime);
	void unapplyItemForSelf(ItemApplicationTemplate const* appTmpl);

	void setupViewport();
	void setViewPointCenter(cocos2d::Point const& position);

#if USE_DEBUG_OPTION
	void onWorldStatus(WorldStatus const& status) override;
	void buttonTheaterStatusCallback(Ref* sender);
	void buttonGMCommandCallback(Ref* sender);
	void updateMyStatus();
	void updateWorldStatus(float delta);
	void buttonEquipmentTouchCallback(Ref* sender, ui::Widget::TouchEventType eventType);
	void toggleDebugInfo();
#endif // USE_DEBUG_OPTION

	void backToHome(RewardedAdConfig const& adConfig, bool isEnableInterstitialAd);
	void playNext();
	void cleanupBeforeExit();
	void stopWorldAndBackToHome();
	void stopWorld(std::function<void()> callback);

	void showQuitDialog();
	void buttonQuitCallback(Ref* sender);

	void showLoadingView();
	void dismissLoadingView();
	void showNetworkErrorDialog(std::string const& title, std::string const& message, std::string const& retryButtonTitle = "");

	// GamePadListener
	void onMoveControlMoving(Ref* sender, float direction) override;
	void onMoveControlStopped(Ref* sender) override;
	void onAttackControlAiming(Ref* sender, float direction) override;
	void onAttackControlFire(Ref* sender, float direction) override;
	void onAttackControlFire(Ref* sender, cocos2d::Point const& aimPoint) override;
	void onAttackControlCharging(Ref* sender) override;
	void onAttackControlCanceling(Ref* sender) override;
	void onAttackControlCanceled(Ref* sender) override;

	void updateLatency();
	void updateLabelWithLatency(NSTime latency);
    
    // GameCenterListener
    void onAuthStarted(AuthOperation op) override;
    void onAuthFinished(AuthOperation op, AuthResponse const& response) override;
    void onReportScoreSuccess() override;
    void onReportScoreFail(GameCenterError const& error) override;
    
    void smileyBoxSelectCallback(Ref* sender, uint16 code);
	void buttonEquipmentCallback(Ref* sender);

	bool tryRestoreConnection();

	PlayerProfile m_playerProfile;
	RegionalInfo m_regionalInfo;
	std::function<void()> m_onWorldStoppedCallback;
	Director::Projection m_prevProjection;
	float m_safeInsetLeft;
	float m_safeInsetRight;
	bool m_isTraining;
	std::bitset<MAX_BATTLE_EVENTS> m_triggeredBattleEvents;

	SmileyBox* m_smileyBox;
	ui::Button* m_equipmentBtn;
	Layer* m_viewportLayer;
	GameMapLayer* m_gameMapLayer;
	Label* m_latencyLabel;
	StatusBar* m_statusBar;
	Toaster* m_toaster;
	MessageBar* m_messageBar;
	GamePad* m_gamePad;
	PreparationTimer* m_preparationTimer;
	InventoryBar* m_inventoryBar;
	ScreenGlowBorder* m_screenGlowBorder;
	Minimap* m_minimap;

	SafeZone* m_safeZone;
	DangerZone* m_dangerZone;

	TutorialLayer* m_tutorialLayer;

	bool m_hasTryRestoreConn;
	int32 m_autoRestoreConnCount;
	LoadingView* m_loadingView;
	SignalIndicator* m_signalIndicator;

	bool m_isShowDebugInfo;
	Label* m_myStatusLabel;
	Label* m_worldStatusLabel;
	Label* m_regionalInfoLabel;
	ui::Button* m_quitBtn;
	ui::Button* m_theaterStatusBtn;
	ui::Button* m_gmCommandBtn;
	BattleTimer* m_battleTimer;

	bool m_isQueryingWorldStatus;
	IntervalTimer m_worldStatusUpdateTimer;
};

NS_END

#endif // __BATTLE_SCENE_H__
