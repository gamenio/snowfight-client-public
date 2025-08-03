#ifndef __SPLASH_SCENE_H__
#define __SPLASH_SCENE_H__

#include "common/Common.h"
#include "common/utils/Timer.h"
#include "game/gamecenter/GameCenter.h"
#include "gui/LoadingIndicator.h"
#include "BaseScene.h"

USING_NS_CC;

NS_BEGIN

class SplashScene : public BaseScene, GameCenterListener
{
    enum TaskFlag
    {
		TASK_PENDING,
		TASK_INIT_GAME_CONFIGS,
        TASK_LOAD_ASSETS,
        TASK_ASSETS_LOADING,
        TASK_WAITING_SPLASH,
        TASK_VALIDATE_RECEIPT,
        TASK_AUTHENTICATING_LOCAL_PLAYER
    };

public:
	static Scene* createScene();

	bool init() override;
	CREATE_FUNC(SplashScene)

	SplashScene();
	~SplashScene();

	void update(float delta) override;

    // GameCenterListener
    void onAuthFinished(AuthOperation op, AuthResponse const& response) override;
    void onReportScoreSuccess() override;
    void onReportScoreFail(GameCenterError const& error) override;
    
private:
	void initGameConfigs();
	void loadAssets();
	void startFuncScene();
	void startGameLoadScene();
    void validateProductIds();
    void validateReceipt();
	void initPlayerData();

	void onAllTasksCompleted();
	void onGeneralAssetsLoaded();
	void onShopDataLoaded();
	void onObjectDataLoaded();
    
	void startTasks();
    void addTask(TaskFlag task);
    void removeTask(TaskFlag task);
    void performTaskInQueue();
	void debugTaskQueue();

	void showGameDataErrorDialog();

	bool m_isLoadPlayerDataFailed;

	LayerColor* m_background;
	Sprite* m_main;
	LoadingIndicator* m_loadingIndicator;
	DelayTimer m_delayTimer;

	Label* m_queuedTasksLabel;
    std::vector<TaskFlag> m_pendingTaskQueue;
};


NS_END

#endif // __SPLASH_SCENE_H__
