#include "UserPreferences.h"

#include "common/utils/StringUtility.h"
#include "LocaleMgr.h"

NS_BEGIN

#define KEY_SOUND_EFFECT_ENABLED					"sound_effect_enabled"
#define KEY_MUSIC_ENABLED							"music_enabled"
#define KEY_LAST_LOGGEDIN_PLAYERID					"last_loggedin_playerid"
#define KEY_DEFERRED_TRANSACTIONS_INVALID			"deferred_transactions_invalid"
#define KEY_SHOP_DB_VERSION							"shop_db_version"
#define KEY_PLAYER_DB_VERSION						"player_db_version"
#define KEY_ITEM_DB_VERSION							"item_db_version"
#define KEY_LAST_REVIEWED_VERSION					"last_reviewed_version"
#define KEY_COMPLETED_REVIEW_PROCESSES				"completed_review_processes"
#define KEY_NEXT_REVIEW_TIME						"next_review_time"
#define KEY_UPGRADE_STAT_COUNT						"upgrade_stat_count"
#define KEY_APP_LAUNCH_COUNT						"app_launch_count"
#define KEY_CONTROLLER_TYPE           				"controller_type"

#define KEY_DEBUG_OPTIONS							"debug_options"
#define KEY_COUNTRY									"country"

UserPreferences* UserPreferences::instance()
{
    static UserPreferences instance;
    return &instance;
}

void UserPreferences::setSoundEffectEnabled(bool enabled)
{
	this->setBoolean(KEY_SOUND_EFFECT_ENABLED, enabled);
}

bool UserPreferences::isSoundEffectEnabled()
{
	bool value = this->getBoolean(KEY_SOUND_EFFECT_ENABLED, true);
	return value;
}

void UserPreferences::setMusicEnabled(bool enabled)
{
	this->setBoolean(KEY_MUSIC_ENABLED, enabled);
}

bool UserPreferences::isMusicEnabled()
{
	auto value = this->getBoolean(KEY_MUSIC_ENABLED, true);
	return value;
}

void UserPreferences::setLastLoggedInPlayerID(std::string const& playerID)
{
    this->setString(KEY_LAST_LOGGEDIN_PLAYERID, playerID);
}

std::string UserPreferences::getLastLoggedInPlayerID()
{
    auto value = this->getString(KEY_LAST_LOGGEDIN_PLAYERID, "");
    return value;
}

void UserPreferences::setDeferredTransactionsInvalid(bool invalid)
{
     this->setBoolean(KEY_DEFERRED_TRANSACTIONS_INVALID, invalid);
}

bool UserPreferences::isDeferredTransactionsInvalid()
{
    auto value = this->getBoolean(KEY_DEFERRED_TRANSACTIONS_INVALID, true);
    return value;
}

void UserPreferences::setShopDBVersion(int32 version)
{
    this->setInteger(KEY_SHOP_DB_VERSION, version);
}

int32 UserPreferences::getShopDBVersion()
{
    auto value = this->getInteger(KEY_SHOP_DB_VERSION, 0);
    return value;
}

void UserPreferences::setPlayerDBVersion(int32 version)
{
	this->setInteger(KEY_PLAYER_DB_VERSION, version);
}

int32 UserPreferences::getPlayerDBVersion()
{
	auto value = this->getInteger(KEY_PLAYER_DB_VERSION, 0);
	return value;
}

void UserPreferences::setItemDBVersion(int32 version)
{
	this->setInteger(KEY_ITEM_DB_VERSION, version);
}

int32 UserPreferences::getItemDBVersion()
{
	auto value = this->getInteger(KEY_ITEM_DB_VERSION, 0);
	return value;
}

void UserPreferences::setLastReviewedVersion(int32 version)
{
	this->setInteger(KEY_LAST_REVIEWED_VERSION, version);
}

int32 UserPreferences::getLastReviewedVersion()
{
	auto value = this->getInteger(KEY_LAST_REVIEWED_VERSION, 0);
	return value;
}

void UserPreferences::setReviewProcessCompleted(int32 process, bool completed)
{
	int32 processes = this->getInteger(KEY_COMPLETED_REVIEW_PROCESSES, 0);
	if (completed)
		processes |= process;
	else
		processes &= ~process;
	this->setInteger(KEY_COMPLETED_REVIEW_PROCESSES, processes);
}

bool UserPreferences::isReviewProcessCompleted(int32 process)
{
	bool ret = false;

	int32 processes = this->getInteger(KEY_COMPLETED_REVIEW_PROCESSES, 0);
	ret = (processes & process) != 0;

	return ret;
}

int32 UserPreferences::getReviewProcesses()
{
	int32 processes = this->getInteger(KEY_COMPLETED_REVIEW_PROCESSES, 0);
	return processes;
}

void UserPreferences::setNextReviewTime(int32 time)
{
	this->setInteger(KEY_NEXT_REVIEW_TIME, time);
}

int32 UserPreferences::getNextReviewTime()
{
	auto value = this->getInteger(KEY_NEXT_REVIEW_TIME, 0);
	return value;
}

void UserPreferences::setUpgradeStatCount(int32 count)
{
	this->setInteger(KEY_UPGRADE_STAT_COUNT, count);
}

int32 UserPreferences::getUpgradeStatsCount()
{
	auto value = this->getInteger(KEY_UPGRADE_STAT_COUNT, 0);
	return value;
}

void UserPreferences::setAppLaunchCount(int32 count)
{
	this->setInteger(KEY_APP_LAUNCH_COUNT, count);
}

int32 UserPreferences::getAppLaunchCount()
{
	auto value = this->getInteger(KEY_APP_LAUNCH_COUNT, 0);
	return value;
}

void UserPreferences::setControllerType(int32 type)
{
	this->setInteger(KEY_CONTROLLER_TYPE, type);
}

int32 UserPreferences::getControllerType()
{
	auto value = this->getInteger(KEY_CONTROLLER_TYPE, 0);
	return value;
}

Size UserPreferences::getScreenSize(Size const& defaultSize)
{
	Size size = defaultSize;
	std::string screenSizeStr = UserDefault::getInstance()->getStringForKey("screen_size");
	if (!screenSizeStr.empty())
	{
		std::vector<std::string> strs = StringUtility::split(screenSizeStr, '*');
		if (strs.size() >= 2)
		{
			size.width = std::stof(strs[0]);
			size.height = std::stof(strs[1]);
		}
	}
	return size;
}

#if USE_DEBUG_OPTION

void UserPreferences::setDebugOptionEnabled(int32 option, bool enabled)
{
    int32 options = this->getInteger(KEY_DEBUG_OPTIONS, 0);
    if(enabled)
        options |= option;
    else
        options &= ~option;
    this->setInteger(KEY_DEBUG_OPTIONS, options);
}

bool UserPreferences::isDebugOptionEnabled(int32 option)
{
	bool ret = false;

    int32 options = this->getInteger(KEY_DEBUG_OPTIONS, 0);
    ret = (options & option) != 0;

	return ret;
}

void UserPreferences::setCountry(std::string const& country)
{
	this->setString(KEY_COUNTRY, country);
}

std::string UserPreferences::getCountry()
{
	auto value = this->getString(KEY_COUNTRY, "");
	return value;
}

#endif // USE_DEBUG_OPTION

void UserPreferences::clear()
{
	this->remove(KEY_SOUND_EFFECT_ENABLED);
	this->remove(KEY_MUSIC_ENABLED);
	this->remove(KEY_LAST_LOGGEDIN_PLAYERID);
	this->remove(KEY_DEFERRED_TRANSACTIONS_INVALID);
	this->remove(KEY_SHOP_DB_VERSION);
	this->remove(KEY_PLAYER_DB_VERSION);
	this->remove(KEY_ITEM_DB_VERSION);
	this->remove(KEY_LAST_REVIEWED_VERSION);
	this->remove(KEY_COMPLETED_REVIEW_PROCESSES);
	this->remove(KEY_NEXT_REVIEW_TIME);
	this->remove(KEY_UPGRADE_STAT_COUNT);
	this->remove(KEY_APP_LAUNCH_COUNT);
	this->remove(KEY_CONTROLLER_TYPE);
}

void UserPreferences::setBoolean(std::string const& key, bool value)
{
	std::lock_guard<std::mutex> lock(m_cacheMutex);

	UserDefault::getInstance()->setBoolForKey(key.c_str(), value);
	m_cachedValues[key] = value;
}

bool UserPreferences::getBoolean(std::string const& key, bool defaultValue)
{

	std::lock_guard<std::mutex> lock(m_cacheMutex);

	bool value;
	auto it = m_cachedValues.find(key);
	if (it == m_cachedValues.end())
	{
		value = UserDefault::getInstance()->getBoolForKey(key.c_str(), defaultValue);
		auto ret = m_cachedValues.emplace(key, value);
        NS_UNUSED_VARIABLE(ret);
		NS_ASSERT(ret.second);
	}
	else
		value = (*it).second.asBool();

	return value;
}

void UserPreferences::setInteger(std::string const& key, int32 value)
{
	std::lock_guard<std::mutex> lock(m_cacheMutex);

	UserDefault::getInstance()->setIntegerForKey(key.c_str(), value);
	m_cachedValues[key] = value;
}

int32 UserPreferences::getInteger(std::string const& key, int32 defaultValue)
{
	std::lock_guard<std::mutex> lock(m_cacheMutex);

	int32 value;
	auto it = m_cachedValues.find(key);
	if (it == m_cachedValues.end())
	{
		value = UserDefault::getInstance()->getIntegerForKey(key.c_str(), defaultValue);
		auto ret = m_cachedValues.emplace(key, value);
        NS_UNUSED_VARIABLE(ret);
		NS_ASSERT(ret.second);
	}
	else
		value = (*it).second.asInt();

	return value;
}

void UserPreferences::setString(std::string const& key, std::string const& value)
{
	std::lock_guard<std::mutex> lock(m_cacheMutex);

	UserDefault::getInstance()->setStringForKey(key.c_str(), value);
	m_cachedValues[key] = value;
}

std::string UserPreferences::getString(std::string const& key, std::string const& defaultValue)
{
	std::lock_guard<std::mutex> lock(m_cacheMutex);

	std::string value;
	auto it = m_cachedValues.find(key);
	if (it == m_cachedValues.end())
	{
		value = UserDefault::getInstance()->getStringForKey(key.c_str(), defaultValue);
		auto ret = m_cachedValues.emplace(key, value);
        NS_UNUSED_VARIABLE(ret);
		NS_ASSERT(ret.second);
	}
	else
		value = (*it).second.asString();

	return value;
}

void UserPreferences::setFloat(std::string const& key, float value)
{
	std::lock_guard<std::mutex> lock(m_cacheMutex);

	UserDefault::getInstance()->setFloatForKey(key.c_str(), value);
	m_cachedValues[key] = value;
}

float UserPreferences::getFloat(std::string const& key, float defaultValue)
{
	std::lock_guard<std::mutex> lock(m_cacheMutex);

	float value;
	auto it = m_cachedValues.find(key);
	if (it == m_cachedValues.end())
	{
		value = UserDefault::getInstance()->getFloatForKey(key.c_str(), defaultValue);
		auto ret = m_cachedValues.emplace(key, value);
        NS_UNUSED_VARIABLE(ret);
		NS_ASSERT(ret.second);
	}
	else
		value = (*it).second.asFloat();

	return value;
}

void UserPreferences::remove(std::string const& key)
{
	std::lock_guard<std::mutex> lock(m_cacheMutex);

	UserDefault::getInstance()->deleteValueForKey(key.c_str());

	auto it = m_cachedValues.find(key);
	if (it != m_cachedValues.end())
		m_cachedValues.erase(it);
}

NS_END
