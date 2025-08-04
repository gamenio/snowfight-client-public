#ifndef __USER_PREFERENCES_H__
#define __USER_PREFERENCES_H__

#include "common/Common.h"
#include "GameConfig.h"

USING_NS_CC;

NS_BEGIN

#if USE_DEBUG_OPTION
// Debug options
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

	// Sound effect switch
	void setSoundEffectEnabled(bool enabled);
	bool isSoundEffectEnabled();

	// Music switch
	void setMusicEnabled(bool enabled);
	bool isMusicEnabled();
    
	// Last logged-in player ID
    void setLastLoggedInPlayerID(std::string const& playerID);
    std::string getLastLoggedInPlayerID();
    
	// Is a deferred transaction invalid
    void setDeferredTransactionsInvalid(bool invalid);
    bool isDeferredTransactionsInvalid();

	// Shop database version
	void setShopDBVersion(int32 version);
	int32 getShopDBVersion();

	// Player database version
	void setPlayerDBVersion(int32 version);
	int32 getPlayerDBVersion();

	// Item database version
	void setItemDBVersion(int32 version);
	int32 getItemDBVersion();

	// The version of the app that was last reviewed.
	void setLastReviewedVersion(int32 version);
	int32 getLastReviewedVersion();
	// Is the review process complete
	void setReviewProcessCompleted(int32 process, bool completed);
	bool isReviewProcessCompleted(int32 process);
	int32 getReviewProcesses();
	// Next review time
	void setNextReviewTime(int32 time);
	int32 getNextReviewTime();
	// The number of times the stat has been upgraded
	void setUpgradeStatCount(int32 count);
	int32 getUpgradeStatsCount();
	// Number of times the app has been run
	void setAppLaunchCount(int32 count);
	int32 getAppLaunchCount();

	// Game controller type
	void setControllerType(int32 type);
	int32 getControllerType();

	// Customize screen size
	cocos2d::Size getScreenSize(cocos2d::Size const& defaultSize);
    
#if USE_DEBUG_OPTION

    // Debug option
    void setDebugOptionEnabled(int32 option, bool enabled);
    bool isDebugOptionEnabled(int32 option);

	// Customize country
	void setCountry(std::string const& country);
	std::string getCountry();

#endif // USE_DEBUG_OPTION

	// Clear user preferences. Does not include debug options
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
