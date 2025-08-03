#ifndef __USER_PREFERENCES_H__
#define __USER_PREFERENCES_H__

#include "common/Common.h"
#include "GameConfig.h"

USING_NS_CC;

NS_BEGIN

#if USE_DEBUG_OPTION
// 调试选项
enum DebugOption
{
	DEBUG_OPTION_GAME_MASTER					= 1 << 0,
	DEBUG_OPTION_ZOOM_IN_WINDOW					= 1 << 1,
	DEBUG_OPTION_REALM_BY_REGION				= 1 << 2,
	DEBUG_OPTION_SESSION_TIMEOUT_DISABLED		= 1 << 3,
};

#endif // USE_DEBUG_OPTION

class UserPreferences
{
public:
    static UserPreferences* instance();

	// 音效开关
	void setSoundEffectEnabled(bool enabled);
	bool isSoundEffectEnabled();

	// 音乐开关
	void setMusicEnabled(bool enabled);
	bool isMusicEnabled();
    
	// 最后登录的玩家ID
    void setLastLoggedInPlayerID(std::string const& playerID);
    std::string getLastLoggedInPlayerID();
    
	// 被延迟的交易是否有效
    void setDeferredTransactionsInvalid(bool invalid);
    bool isDeferredTransactionsInvalid();

	// 商店DB版本
	void setShopDBVersion(int32 version);
	int32 getShopDBVersion();

	// 玩家DB版本
	void setPlayerDBVersion(int32 version);
	int32 getPlayerDBVersion();

	// 物品DB版本
	void setItemDBVersion(int32 version);
	int32 getItemDBVersion();

	// 最后被评论的应用版本
	void setLastReviewedVersion(int32 version);
	int32 getLastReviewedVersion();
	// 评论步骤已完成
	void setReviewProcessCompleted(int32 process, bool completed);
	bool isReviewProcessCompleted(int32 process);
	int32 getReviewProcesses();
	// 下一次评论时间
	void setNextReviewTime(int32 time);
	int32 getNextReviewTime();
	// 升级属性次数
	void setUpgradeStatCount(int32 count);
	int32 getUpgradeStatsCount();
	// 应用运行次数
	void setAppLaunchCount(int32 count);
	int32 getAppLaunchCount();

	// 游戏控制器类型
	void setControllerType(int32 type);
	int32 getControllerType();

	// 自定义屏幕大小
	cocos2d::Size getScreenSize(cocos2d::Size const& defaultSize);
    
#if USE_DEBUG_OPTION

    // 调试选项
    void setDebugOptionEnabled(int32 option, bool enabled);
    bool isDebugOptionEnabled(int32 option);

	// 自定义国家
	void setCountry(std::string const& country);
	std::string getCountry();

#endif // USE_DEBUG_OPTION

	// 清理用户偏好设置。不包括Debug选项
	void clear();
    
private:
    UserPreferences() { }
    ~UserPreferences() { }

	void setBoolean(std::string const& key, bool value);
	bool getBoolean(std::string const& key, bool defaultValue);

	void setInteger(std::string const& key, int32 value);
	int32 getInteger(std::string const& key, int32 defaultValue);

	void setString(std::string const& key, std::string const& value);
	std::string getString(std::string const& key, std::string const& defaultValue);

	void setFloat(std::string const& key, float value);
	float getFloat(std::string const& key, float defaultValue);

	void remove(std::string const& key);

	std::unordered_map<std::string, Value> m_cachedValues;
	std::mutex m_cacheMutex;
};

#define sUserPreferences UserPreferences::instance()

NS_END


#endif // __USER_PREFERENCES_H__
