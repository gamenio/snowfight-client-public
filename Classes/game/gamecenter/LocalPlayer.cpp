#include "LocalPlayer.h"
//
//  LocalPlayer.cpp
//  snowfight
//
//  Created by Luthier on 2019/5/3.
//

#include "LocalPlayer.h"

#include "external/xxtea/xxtea.h"

#include "common/utils/TimeUtil.h"
#include "common/utils/StringUtility.h"
#include "common/Machine.h"
#include "game/utils/DBUtils.h"
#include "game/UserPreferences.h"
#include "game/LocaleMgr.h"
#include "game/firservice/FirebaseService.h"

NS_BEGIN

#if NS_DEBUG

// Check for database upgrades
#define CHECK_DB_UPGRADE		1

#endif // NS_DEBUG

enum DBVersionNumber
{
	DB_VERSION_UNDEFINED = 0,
	DB_VERSION_1,
	DB_VERSION_2,
	DB_VERSION_3,
	DB_VERSION_4,
	DB_VERSION_5,
};

// Current required database version
#define REQUIRED_DB_VERSION			DB_VERSION_5

#define GUEST_DB_FILE			"guest.db"
#define PLAYER_DATA_DIR			"player_data"

#define TABLE_NAME_METADATA			"metadata"
#define TABLE_NAME_LOCAL_PLAYER		"local_player"
#define TABLE_NAME_STAT_STAGE		"stat_stage"
#define TABLE_NAME_SCORE			"score"

#define SQL_CREATE_TABLE_METADATA	"CREATE TABLE IF NOT EXISTS " TABLE_NAME_METADATA " (" \
				"version				INTEGER NOT NULL DEFAULT 1" \
				")"

#define SQL_CREATE_TABLE_LOCAL_PLAYER  "CREATE TABLE IF NOT EXISTS " TABLE_NAME_LOCAL_PLAYER " (" \
				"player_id					TEXT NOT NULL DEFAULT ''," \
				"original_player_id			TEXT NOT NULL DEFAULT ''," \
				"nickname					TEXT NOT NULL DEFAULT ''," \
				"hero_id					INTEGER NOT NULL DEFAULT 0," \
				"property					INTEGER NOT NULL DEFAULT 0," \
				"level						INTEGER NOT NULL DEFAULT 0," \
				"experience					INTEGER NOT NULL DEFAULT 0," \
				"reward_time				INTEGER NOT NULL DEFAULT 0," \
				"reward_stage				INTEGER NOT NULL DEFAULT 0," \
				"ads_removed				INTEGER NOT NULL DEFAULT 0," \
				"daily_reward_claim_time	INTEGER NOT NULL DEFAULT 0," \
				"daily_reward_days			INTEGER NOT NULL DEFAULT 0," \
				"tutorial_process			INTEGER NOT NULL DEFAULT 0," \
				"accepted_suggestions		INTEGER NOT NULL DEFAULT 0" \
				")"
#define SQL_CREATE_TABLE_STAT_STAGE	"CREATE TABLE IF NOT EXISTS " TABLE_NAME_STAT_STAGE " (" \
				"hero_id					INTEGER NOT NULL," \
				"stat_type					INTEGER NOT NULL," \
				"stage						INTEGER NOT NULL DEFAULT 0," \
				"PRIMARY KEY(hero_id,stat_type)" \
				")"
#define SQL_CREATE_TABLE_SCORE	"CREATE TABLE IF NOT EXISTS " TABLE_NAME_SCORE " (" \
				"leaderboard_id				TEXT NOT NULL," \
				"value						INTEGER NOT NULL DEFAULT 0," \
				"reported					INTEGER NOT NULL DEFAULT 0," \
				"PRIMARY KEY(leaderboard_id)" \
				")"

#define SQL_INSERT_METADATA "INSERT INTO " TABLE_NAME_METADATA " VALUES (:version)"
#define SQL_SELECT_METADATA "SELECT * FROM " TABLE_NAME_METADATA
#define SQL_UPDATE_METADATA "UPDATE " TABLE_NAME_METADATA " SET version=:version"

#define SQL_INSERT_LOCAL_PLAYER	"INSERT INTO " TABLE_NAME_LOCAL_PLAYER "(" \
				"player_id," \
				"original_player_id," \
				"nickname," \
				"hero_id," \
				"property," \
				"level," \
				"experience," \
				"reward_time," \
				"reward_stage," \
				"ads_removed," \
				"daily_reward_claim_time," \
				"daily_reward_days," \
				"tutorial_process," \
				"accepted_suggestions" \
				") VALUES (" \
				":player_id, " \
				":original_player_id," \
				":nickname," \
				":hero_id," \
				":property," \
				":level," \
				":experience," \
				":reward_time," \
				":reward_stage," \
				":ads_removed," \
				":daily_reward_claim_time," \
				":daily_reward_days," \
				":tutorial_process," \
				":accepted_suggestions" \
				")"
#define SQL_SELECT_LOCAL_PLAYER "SELECT * FROM " TABLE_NAME_LOCAL_PLAYER
#define SQL_UPDATE_LOCAL_PLAYER "UPDATE " TABLE_NAME_LOCAL_PLAYER " SET " \
				"player_id						=:player_id, " \
				"original_player_id				=:original_player_id, " \
				"nickname						=:nickname, " \
				"hero_id						=:hero_id, " \
				"property						=:property, " \
				"level							=:level, " \
				"experience						=:experience, " \
				"reward_time					=:reward_time, " \
				"reward_stage					=:reward_stage, " \
				"ads_removed					=:ads_removed, " \
				"daily_reward_claim_time		=:daily_reward_claim_time, " \
				"daily_reward_days				=:daily_reward_days, " \
				"tutorial_process				=:tutorial_process, " \
				"accepted_suggestions			=:accepted_suggestions"

#define SQL_INSERT_STAT_STAGE "INSERT INTO " TABLE_NAME_STAT_STAGE " VALUES (:hero_id, :stat_type, :stage)"
#define SQL_SELECT_STAT_STAGE "SELECT * FROM " TABLE_NAME_STAT_STAGE
#define SQL_UPDATE_STAT_STAGE "UPDATE " TABLE_NAME_STAT_STAGE " SET stage=:stage WHERE hero_id=:hero_id AND stat_type=:stat_type"

#define SQL_INSERT_SCORE "INSERT INTO " TABLE_NAME_SCORE " VALUES (:leaderboard_id, :value, :reported)"
#define SQL_SELECT_SCORE "SELECT * FROM " TABLE_NAME_SCORE
#define SQL_UPDATE_SCORE "UPDATE " TABLE_NAME_SCORE " SET value=:value, reported=:reported WHERE leaderboard_id=:leaderboard_id"

#define RETURN_IF_FAILED(cond) do { if(!cond) return false; } while(0)


LocalPlayer::LocalPlayer() :
    m_playerID(""),
	m_originalPlayerID(""),
    m_isGuest(false),
    m_nickname(""),
    m_displayName(""),
    m_heroId(HERO_NONE),
	m_property(0),
	m_level(0),
	m_experience(0),
	m_rewardTime(0),
	m_rewardStage(0),
	m_isAdsRemoved(false),
	m_dailyRewardClaimTime(0),
	m_dailyRewardDays(0),
	m_tutorialProcess(TUTORIAL_PROCESS_NONE),
	m_acceptedSuggestions(0),
	m_playerUpdated(false),
    m_isLoadingData(false),
    m_authenticated(false),
    m_underage(false),
	m_database(nullptr)
{
    this->setupDefaultValues();
}

LocalPlayer::~LocalPlayer()
{
	this->closeDB();
}

void LocalPlayer::setNickname(std::string const& nickname)
{
	if (m_nickname != nickname)
	{
		m_nickname = nickname;
		m_playerUpdated = true;
	}
}

void LocalPlayer::setHeroId(uint32 heroId)
{
	if (m_heroId != heroId)
	{
		m_heroId = heroId;
		m_playerUpdated = true;
	}
}

void LocalPlayer::setScoreValue(std::string const& leaderboardID, int32 newValue)
{
    ObjectUpdate<ScoreInfo>& score = m_scores[leaderboardID];
    if(score.object.value != newValue)
    {
        score.object.value = newValue;
        score.object.reported = false;
        score.isUpdated = true;
    }
}

int32 LocalPlayer::getScoreValue(std::string const& leaderboardID) const
{
    auto it = m_scores.find(leaderboardID);
    if(it != m_scores.end())
        return (*it).second.object.value;
    
    return 0;
}

void LocalPlayer::setScoreReported(std::string const& leaderboardID, bool reported)
{
	ObjectUpdate<ScoreInfo>& score = m_scores[leaderboardID];
	if (score.object.reported != reported)
	{
		score.object.reported = reported;
		score.isUpdated = true;
	}
}

bool LocalPlayer::isScoreReported(std::string const& leaderboardID) const
{
    auto it = m_scores.find(leaderboardID);
    if(it != m_scores.end())
        return (*it).second.object.reported;
    
    return false;
}

void LocalPlayer::setStatStage(uint32 heroId, StatType statType, uint8 stage)
{
	auto it = m_statStageLists.find(heroId);
	NS_ASSERT_LOG(it != m_statStageLists.end(), "Before setting the stat stage, you need to call unlockHero() function to unlock.");

	if (it != m_statStageLists.end())
	{
		auto& statStageList = (*it).second;
		if (statStageList.object[statType] != stage)
		{
			statStageList.object[statType] = stage;
			statStageList.isUpdated = true;
		}
	}
}

uint8 LocalPlayer::getStatStage(uint32 heroId, StatType statType) const
{
	auto it = m_statStageLists.find(heroId);
	if (it != m_statStageLists.end())
		return (*it).second.object[statType];
	
	return 0;
}

bool LocalPlayer::getStatStageList(uint32 heroId, StatStageList& result) const
{
	auto it = m_statStageLists.find(heroId);
	if (it != m_statStageLists.end())
	{
		result = (*it).second.object;
		return true;
	}

	return false;
}

void LocalPlayer::setRewardStage(uint8 stage)
{
	if (m_rewardStage != stage)
	{
		m_rewardStage = stage;
		m_playerUpdated = true;
	}
}

void LocalPlayer::setRewardTime(uint32 seconds)
{
	if (m_rewardTime != seconds)
	{
		m_rewardTime = seconds;
		m_playerUpdated = true;
	}
}

void LocalPlayer::setAdsRemoved(bool removed)
{
	if (m_isAdsRemoved != removed)
	{
		m_isAdsRemoved = removed;
		m_playerUpdated = true;
	}
}

void LocalPlayer::setDailyRewardClaimTime(uint32 seconds)
{
	if (m_dailyRewardClaimTime != seconds)
	{
		m_dailyRewardClaimTime = seconds;
		m_playerUpdated = true;
	}
}

void LocalPlayer::increaseDailyRewardDays()
{
	this->setDailyRewardDays(this->getDailyRewardDays() + 1);
}

void LocalPlayer::setDailyRewardDays(int32 days)
{
	if (m_dailyRewardDays != days)
	{
		m_dailyRewardDays = days;
		m_playerUpdated = true;
	}
}

void LocalPlayer::setTutorialProcess(int32 process)
{
	if (m_tutorialProcess != process)
	{
		m_tutorialProcess = process;
		m_playerUpdated = true;
	}
}

void LocalPlayer::setSuggestionAccepted(uint32 suggestion, bool accepted)
{
	uint32 suggestions = m_acceptedSuggestions;
	if (accepted)
		suggestions |= suggestion;
	else
		suggestions &= ~suggestion;
	if (m_acceptedSuggestions != suggestions)
	{
		m_acceptedSuggestions = suggestions;
		m_playerUpdated = true;
	}
}

bool LocalPlayer::isSuggestionAccepted(uint32 suggestion) const
{
	return (m_acceptedSuggestions & suggestion) != 0;
}

void LocalPlayer::unlockHero(uint32 heroId)
{
	if (this->hasUnlockedHero(heroId))
		return;

	auto& statStageList = m_statStageLists[heroId];
	for (int32 type = 0; type < MAX_STAT_TYPES; ++type)
	{
		statStageList.object[type] = STAT_STAGE_MIN;
		statStageList.isUpdated = true;
	}
}

bool LocalPlayer::hasUnlockedHero(uint32 heroId) const
{
	auto it = m_statStageLists.find(heroId);
	return it != m_statStageLists.end();
}

void LocalPlayer::setProperty(int32 property)
{
	if (m_property != property)
	{
		m_property = property;
		m_playerUpdated = true;
	}
}

void LocalPlayer::addMoney(int32 money)
{
	this->setProperty(m_property + money);
}

void LocalPlayer::removeMoney(int32 money)
{
	this->setProperty(m_property - money);
}

void LocalPlayer::setLevel(uint8 level)
{
	if (m_level != level)
	{
		m_level = level;
		m_playerUpdated = true;
	}
}

void LocalPlayer::setExperience(int32 xp)
{
	if (m_experience != xp)
	{
		m_experience = xp;
		m_playerUpdated = true;
	}
}

bool LocalPlayer::switchToNewPlayerID(std::string const& oldPlayerID, std::string const& newPlayerID)
{
	std::string writablePath = FileUtils::getInstance()->getWritablePath();
	std::string oldPlayerDBFile = StringUtils::format("%s%s/%s.db", writablePath.c_str(), PLAYER_DATA_DIR, this->hashPlayerID(oldPlayerID).c_str());

	if (FileUtils::getInstance()->isFileExist(oldPlayerDBFile))
	{
		std::string newPlayerDBFile = StringUtils::format("%s%s/%s.db", writablePath.c_str(), PLAYER_DATA_DIR, this->hashPlayerID(newPlayerID).c_str());
		if (!FileUtils::getInstance()->isFileExist(newPlayerDBFile))
		{
			std::string playerID, originalPlayerID;
			bool ret = this->execSelectPlayerID(oldPlayerDBFile, playerID, originalPlayerID);
			if (ret)
			{
				NS_ASSERT(!playerID.empty());
				// Verify that it is the current player's data file
				if (playerID == oldPlayerID || playerID == newPlayerID)
				{
					// Write the new PlayerID to the player DB file
					ret = this->execUpdatePlayerID(oldPlayerDBFile, newPlayerID, oldPlayerID);
					if (ret)
					{
						this->closeDB();
						ret = FileUtils::getInstance()->renameFile(oldPlayerDBFile, newPlayerDBFile);
						if (ret)
						{
							sUserPreferences->setLastLoggedInPlayerID(newPlayerID);
						}
						// Rename file failed, revert to previous changes.
						else
						{
							this->execUpdatePlayerID(oldPlayerDBFile, playerID, originalPlayerID);
						}
					}
				}
			}
			return ret;
		}
	}

	return true;
}

bool LocalPlayer::setupSQLiteDB(std::string const& dbFile)
{
    NS_ASSERT(this->isDBFile(dbFile));

    if(m_database)
    {
        if(m_database->getFilename() != dbFile)
            this->closeDB();
        else
            return true;
    }

	try
	{
		if (FileUtils::getInstance()->isFileExist(dbFile))
			m_database = new SQLite::Database(dbFile, SQLite::OPEN_READWRITE);
		else
		{
			std::string dbPath = this->getDBPath(dbFile);
			if (!dbPath.empty())
			{
				if (!FileUtils::getInstance()->isDirectoryExist(dbPath))
				{
					if (!FileUtils::getInstance()->createDirectory(dbPath))
						throw std::runtime_error(StringUtils::format("Failed to create directory %s", dbPath.c_str()));
				}
			}
			m_database = new SQLite::Database(dbFile, SQLite::OPEN_CREATE | SQLite::OPEN_READWRITE);
		}

		m_database->key(LOCALPLAYER_DB_KEY);


		if (!m_database->tableExists(TABLE_NAME_METADATA))
		{
			SQLite::Transaction transaction(*m_database);

			m_database->exec(SQL_CREATE_TABLE_METADATA);
			SQLite::Statement metadataQuery(*m_database, SQL_INSERT_METADATA);
			metadataQuery.bind(":version", REQUIRED_DB_VERSION);
			metadataQuery.exec();

			m_database->exec(SQL_CREATE_TABLE_LOCAL_PLAYER);
			m_database->exec(SQL_CREATE_TABLE_STAT_STAGE);
			m_database->exec(SQL_CREATE_TABLE_SCORE);

			transaction.commit();
		}
		else
		{
			SQLite::Statement query(*m_database, SQL_SELECT_METADATA);
			if (!query.executeStep())
				throw SQLite::Exception("No results returned: " SQL_SELECT_METADATA);

#if CHECK_DB_UPGRADE
			int32 dbVersion = 0;
#else
			int32 dbVersion = query.getColumn("version");
#endif
			if (REQUIRED_DB_VERSION > dbVersion)
			{
				SQLite::Transaction transaction(*m_database);

				this->execUpgradeToNewDB(dbVersion, m_database);

				SQLite::Statement metadataQuery(*m_database, SQL_UPDATE_METADATA);
				metadataQuery.bind(":version", REQUIRED_DB_VERSION);
				metadataQuery.exec();

				transaction.commit();
			}
		}

		return true;
	}
	catch (std::exception const& ex)
	{
		this->closeDB();
		CCLOG("LocalPlayer: Failed to initialize DB file %s. error: %s", dbFile.c_str(), ex.what());
	}
	return false;
}

void LocalPlayer::closeDB()
{
	CC_SAFE_DELETE(m_database);
}

// The format of the returned path: /home/player_data/
std::string LocalPlayer::getDBPath(std::string const& dbFile)
{
	std::string dbPath;

	size_t pos = dbFile.find_last_of('/');
	if (pos != std::string::npos)
		dbPath = dbFile.substr(0, pos + 1);

	return dbPath;
}

bool LocalPlayer::isDBFile(std::string const& file) const
{
	return StringUtility::endsWith(file, ".db");
}

bool LocalPlayer::updateValuesFromDB(std::string const& dbFile)
{
	if (!this->setupSQLiteDB(dbFile))
		return false;

	try
	{
		// Local player
		SQLite::Statement playerQuery(*m_database, SQL_SELECT_LOCAL_PLAYER);
		if (playerQuery.executeStep())
		{
			m_playerID =  playerQuery.getColumn("player_id").getString();
			m_originalPlayerID = playerQuery.getColumn("original_player_id").getString();
			m_nickname = playerQuery.getColumn("nickname").getString();
			m_heroId = playerQuery.getColumn("hero_id");
			m_property = playerQuery.getColumn("property");
			m_level = playerQuery.getColumn("level");
			m_experience = playerQuery.getColumn("experience");
			m_isAdsRemoved = playerQuery.getColumn("ads_removed").getInt() != 0;
			m_rewardTime = playerQuery.getColumn("reward_time");
			m_rewardStage = playerQuery.getColumn("reward_stage");
			m_dailyRewardClaimTime = playerQuery.getColumn("daily_reward_claim_time");
			m_dailyRewardDays = playerQuery.getColumn("daily_reward_days");
			m_tutorialProcess = playerQuery.getColumn("tutorial_process").getInt();
			m_acceptedSuggestions = playerQuery.getColumn("accepted_suggestions").getUInt();
		}

		// Statistic stage
		SQLite::Statement statStageQuery(*m_database, SQL_SELECT_STAT_STAGE);
		while (statStageQuery.executeStep())
		{
			uint32 heroId = statStageQuery.getColumn("hero_id");
			int32 statType = statStageQuery.getColumn("stat_type");
			auto& statStageList = m_statStageLists[heroId].object;
			statStageList[statType] = statStageQuery.getColumn("stage");
		}

		// Score
		SQLite::Statement scoreQuery(*m_database, SQL_SELECT_SCORE);
		while (scoreQuery.executeStep())
		{
			std::string leaderboardID = scoreQuery.getColumn("leaderboard_id").getString();
			auto& score = m_scores[leaderboardID].object;
			score.value = scoreQuery.getColumn("value");
			score.reported = scoreQuery.getColumn("reported").getInt() != 0;
		}

		return true;
	}
	catch (std::exception const& ex)
	{
		CCLOG("LocalPlayer: Failed to access DB file %s. error: %s", dbFile.c_str(), ex.what());
	}

	return false;
}

void LocalPlayer::execUpgradeToNewDB(int32 oldVersion, SQLite::Database* m_database)
{
	if (oldVersion < DB_VERSION_2)
	{
		if(!DBUtils::isColumnExists(m_database, TABLE_NAME_LOCAL_PLAYER, "property"))
			m_database->exec("ALTER TABLE " TABLE_NAME_LOCAL_PLAYER " ADD COLUMN property INTEGER DEFAULT 0");
		if (!DBUtils::isColumnExists(m_database, TABLE_NAME_LOCAL_PLAYER, "reward_time"))
			m_database->exec("ALTER TABLE " TABLE_NAME_LOCAL_PLAYER " ADD COLUMN reward_time INTEGER DEFAULT 0");
		if (!DBUtils::isColumnExists(m_database, TABLE_NAME_LOCAL_PLAYER, "reward_stage"))
			m_database->exec("ALTER TABLE " TABLE_NAME_LOCAL_PLAYER " ADD COLUMN reward_stage INTEGER DEFAULT 0");
		m_database->exec(SQL_CREATE_TABLE_STAT_STAGE);
	}

	if (oldVersion < DB_VERSION_3)
	{
		if (!DBUtils::isColumnExists(m_database, TABLE_NAME_LOCAL_PLAYER, "level"))
			m_database->exec("ALTER TABLE " TABLE_NAME_LOCAL_PLAYER " ADD COLUMN level INTEGER DEFAULT 0");
		if (!DBUtils::isColumnExists(m_database, TABLE_NAME_LOCAL_PLAYER, "experience"))
			m_database->exec("ALTER TABLE " TABLE_NAME_LOCAL_PLAYER " ADD COLUMN experience INTEGER DEFAULT 0");
		if (!DBUtils::isColumnExists(m_database, TABLE_NAME_LOCAL_PLAYER, "ads_removed"))
			m_database->exec("ALTER TABLE " TABLE_NAME_LOCAL_PLAYER " ADD COLUMN ads_removed INTEGER DEFAULT 0");
	}

	if (oldVersion < DB_VERSION_4)
	{
		if (!DBUtils::isColumnExists(m_database, TABLE_NAME_LOCAL_PLAYER, "daily_reward_claim_time"))
			m_database->exec("ALTER TABLE " TABLE_NAME_LOCAL_PLAYER " ADD COLUMN daily_reward_claim_time INTEGER DEFAULT 0");
		if (!DBUtils::isColumnExists(m_database, TABLE_NAME_LOCAL_PLAYER, "daily_reward_days"))
			m_database->exec("ALTER TABLE " TABLE_NAME_LOCAL_PLAYER " ADD COLUMN daily_reward_days INTEGER DEFAULT 0");
	}

	if (oldVersion < DB_VERSION_5)
	{
		if (!DBUtils::isColumnExists(m_database, TABLE_NAME_LOCAL_PLAYER, "tutorial_process"))
			m_database->exec("ALTER TABLE " TABLE_NAME_LOCAL_PLAYER " ADD COLUMN tutorial_process INTEGER DEFAULT 0");
		if (!DBUtils::isColumnExists(m_database, TABLE_NAME_LOCAL_PLAYER, "accepted_suggestions"))
			m_database->exec("ALTER TABLE " TABLE_NAME_LOCAL_PLAYER " ADD COLUMN accepted_suggestions INTEGER DEFAULT 0");
	}
}

bool LocalPlayer::execUpdatePlayerID(std::string const& dbFile, std::string const& playerID, std::string const& originalPlayerID)
{
	if (!this->setupSQLiteDB(dbFile))
		return false;

	try
	{
		SQLite::Statement query(*m_database, "UPDATE " TABLE_NAME_LOCAL_PLAYER " SET player_id=:player_id, original_player_id=:original_player_id");
		query.bind(":player_id", playerID);
		query.bind(":original_player_id", originalPlayerID);
		query.exec();

		return true;
	}
	catch (std::exception const& ex)
	{
		CCLOG("LocalPlayer: Failed to access DB file %s. error: %s", dbFile.c_str(), ex.what());
	}
	return false;
}

void LocalPlayer::execUpdateScore(SQLite::Database* m_database, std::string const& leaderboardID, ScoreInfo const& score)
{
	SQLite::Statement query(*m_database, SQL_UPDATE_SCORE);
	query.bind(":value", score.value);
	query.bind(":reported", score.reported);
	query.bind(":leaderboard_id", leaderboardID);
	query.exec();
}

void LocalPlayer::execInsertScore(SQLite::Database* m_database, std::string const& leaderboardID, ScoreInfo const& score)
{
	SQLite::Statement query(*m_database, SQL_INSERT_SCORE);
	query.bind(":leaderboard_id", leaderboardID);
	query.bind(":value", score.value);
	query.bind(":reported", score.reported);
	query.exec();
}

void LocalPlayer::execUpdateStatStage(SQLite::Database* m_database, uint32 heroId, int32 statType, uint8 stage)
{
	SQLite::Statement query(*m_database, SQL_UPDATE_STAT_STAGE);
	query.bind(":stage", stage);
	query.bind(":hero_id", heroId);
	query.bind(":stat_type", statType);
	query.exec();
}

void LocalPlayer::execInsertStatStage(SQLite::Database* m_database, uint32 heroId, int32 statType, uint8 stage)
{
	SQLite::Statement query(*m_database, SQL_INSERT_STAT_STAGE);
	query.bind(":hero_id", heroId);
	query.bind(":stat_type", statType);
	query.bind(":stage", stage);
	query.exec();
}

std::string LocalPlayer::generatePlayerID() const
{
	return Machine::instance()->getDeviceGUID();
}

std::string LocalPlayer::hashPlayerID(std::string const& playerID) const
{
    unsigned int seed = 131; // 31 131 1313 13131 131313 etc..
    unsigned int hash = 0;
    char const* str = playerID.c_str();
    while (*str)
    {
        hash = hash * seed + (*str++);
    }
    
    return std::to_string(hash & 0x7FFFFFFF);
}

void LocalPlayer::setupDefaultValues()
{
    m_playerID = "";
	m_originalPlayerID = "";
    m_isGuest = false;
	m_playerUpdated = false;
    m_nickname = "";
    m_displayName = "";
    m_heroId = HeroID::HERO_NONE;
	m_property = 0;
	m_level = 0;
	m_experience = 0;
	m_rewardStage = 0;
	m_rewardTime = 0;
	m_isAdsRemoved = false;
	m_dailyRewardClaimTime = 0;
	m_dailyRewardDays = 0;
	m_tutorialProcess = TUTORIAL_PROCESS_NONE;
	m_acceptedSuggestions = 0;
    m_scores.clear();
    m_statStageLists.clear();
    m_authenticated = false;
    m_underage = false;
    m_targetDataFile = "";
}

bool LocalPlayer::loadData()
{
    bool ret = false;

	NSTime currTime = time_util::getUptimeMillis();
    
    std::string writablePath = FileUtils::getInstance()->getWritablePath();
	std::string guestDBFile = StringUtils::format("%s%s/%s", writablePath.c_str(), PLAYER_DATA_DIR, GUEST_DB_FILE);

    this->closeDB();
    
    // Player login success
    if(m_authenticated)
    {
		std::string targetPlayerID = m_playerID;

        // Switch to the new PlayerID
        if(!m_playerID.empty() && !m_originalPlayerID.empty())
        {
			if(!this->switchToNewPlayerID(m_originalPlayerID, m_playerID))
				targetPlayerID = m_originalPlayerID;
        }

        NS_ASSERT(!targetPlayerID.empty());
		std::string playerDBFile = StringUtils::format("%s%s/%s.db", writablePath.c_str(), PLAYER_DATA_DIR, this->hashPlayerID(targetPlayerID).c_str());

		// Find the data file for the logged in player
        if(FileUtils::getInstance()->isFileExist(playerDBFile))
        {
			ret = this->updateValuesFromDB(playerDBFile);
			if (ret)
			{
				NS_ASSERT(!m_playerID.empty());
				// Verify that it is the current player's data file
				if (m_playerID == targetPlayerID || m_originalPlayerID == targetPlayerID)
				{
					m_targetDataFile = playerDBFile;
					sUserPreferences->setLastLoggedInPlayerID(targetPlayerID);
				}
				else
					ret = false;
			}
        }
        else
        {
			// Find the guest data file
			// Previous players not logged in (canceled or login failed), associating the guest data file to the logged in player.
			if (FileUtils::getInstance()->isFileExist(guestDBFile))
			{
				std::string loggedInPlayerID = m_playerID;
				ret = this->updateValuesFromDB(guestDBFile);
				if (ret)
				{
					// Check if the player ID in the database has been updated to the logged in player ID.
					// This may occur when renaming a data file fails.
					if (loggedInPlayerID != m_playerID)
						m_originalPlayerID = m_playerID;

					m_playerID = loggedInPlayerID;
					ret = this->execUpdatePlayerID(guestDBFile, m_playerID, m_originalPlayerID);
					this->closeDB();
					if (ret)
					{
						ret = FileUtils::getInstance()->renameFile(guestDBFile, playerDBFile);
						if (ret)
						{
							m_targetDataFile = playerDBFile;
							sUserPreferences->setLastLoggedInPlayerID(targetPlayerID);
						}
					}
				}
			}
            // New player login
            else
            {
				m_playerUpdated = true;
                m_targetDataFile = playerDBFile;
                ret = true;
            }
        }
    }
	// The player failed to login or canceled the login
    else
    {
		// If there is a previously successfully logged in player then load his data file.
        std::string lastLoggedInPlayerID = sUserPreferences->getLastLoggedInPlayerID();
        if(!lastLoggedInPlayerID.empty())
        {
			std::string playerDBFile = StringUtils::format("%s%s/%s.db", writablePath.c_str(), PLAYER_DATA_DIR, this->hashPlayerID(lastLoggedInPlayerID).c_str());
            if(FileUtils::getInstance()->isFileExist(playerDBFile))
            {
				ret = updateValuesFromDB(playerDBFile);
				if (ret)
				{
					if (m_playerID.empty())
						CCLOG("LocalPlayer: Player ID is empty from host data file %s", playerDBFile.c_str());
					m_targetDataFile = playerDBFile;
				}
            }
        }
		// Guest player
        else
        {
            // Find the guest data file
            if(FileUtils::getInstance()->isFileExist(guestDBFile))
            {
				ret = updateValuesFromDB(guestDBFile);
				if (ret)
				{
					// If the guest data store fails, the player ID will likely be a null value.
					if (m_playerID.empty())
					{
						m_playerID = this->generatePlayerID();
						m_playerUpdated = true;
					}
                    m_isGuest = true;
					m_targetDataFile = guestDBFile;
				}
            }
			// New guest
            else
            {
				m_playerID = this->generatePlayerID();
                m_isGuest = true;
				m_playerUpdated = true;
				m_targetDataFile = guestDBFile;
                ret = true;
            }
        }

    }

	if (ret)
	{
		NSTime diff = time_util::getUptimeMillis() - currTime;
        NS_UNUSED_VARIABLE(diff);
		CCLOG("LocalPlayer: Data load time: %dms.", diff);
	}
    
    return ret;
}

void LocalPlayer::loadDataAsync(std::function<void(bool)> callback)
{
	if (m_isLoadingData)
	{
		CCLOG("LocalPlayer: Player data is already loading.");
		return;
	}
    
	m_isLoadingData = true;
    
	AsyncTaskPool::getInstance()->enqueue(AsyncTaskPool::TaskType::TASK_IO, [](void*) {}, nullptr, [this, callback]() {
		bool ret = this->loadData();
		Director::getInstance()->getScheduler()->performFunctionInCocosThread([this, callback, ret] {
			m_isLoadingData = false;
			if (callback)
				callback(ret);

			if(!ret)
				sAnalytics->logException("local_player", "Load data failed");
		});
	});
}

bool LocalPlayer::saveData()
{
	SamplePlayerData data;
	bool ret = this->createSnapshot(data);
	if(ret)
		ret = this->saveDataInternal(data);

	return ret;
}

void LocalPlayer::saveDataAsync(std::function<void(bool)> callback)
{
	SamplePlayerData data;
	bool ret = this->createSnapshot(data);
	if (ret)
	{
		AsyncTaskPool::getInstance()->enqueue(AsyncTaskPool::TaskType::TASK_IO, [](void*) {}, nullptr, [this, callback, data]() {
			bool ret = this->saveDataInternal(data);
			if(callback)
				Director::getInstance()->getScheduler()->performFunctionInCocosThread(std::bind(callback, ret));
		});
	}
	else
	{
		Director::getInstance()->getScheduler()->performFunctionInCocosThread(std::bind(callback, ret));
	}
}

void LocalPlayer::deleteDataAsync(std::function<void()> callback)
{
	AsyncTaskPool::getInstance()->enqueue(AsyncTaskPool::TaskType::TASK_IO, [callback](void*)
	{
		if (callback)
			callback();
	}, nullptr, [this]() {
		this->deleteData();
	});
}

void LocalPlayer::execUpdatePlayer(SQLite::Database* m_database, SamplePlayerData const& data)
{
	SQLite::Statement query(*m_database, SQL_UPDATE_LOCAL_PLAYER);
	query.bind(":player_id", data.playerID);
	query.bind(":original_player_id", data.originalPlayerID);
	query.bind(":nickname", data.nickname);
	query.bind(":hero_id", data.heroId);
	query.bind(":property", data.property);
	query.bind(":level", data.level);
	query.bind(":experience", data.experience);
	query.bind(":ads_removed", data.isAdsRemoved);
	query.bind(":reward_time", data.rewardTime);
	query.bind(":reward_stage", data.rewardStage);
	query.bind(":daily_reward_claim_time", data.dailyRewardClaimTime);
	query.bind(":daily_reward_days", data.dailyRewardDays);
	query.bind(":tutorial_process", data.tutorialProcess);
	query.bind(":accepted_suggestions", data.acceptedSuggestions);
	query.exec();
}


void LocalPlayer::execInsertPlayer(SQLite::Database* m_database, SamplePlayerData const& data)
{
	SQLite::Statement query(*m_database, SQL_INSERT_LOCAL_PLAYER);
	query.bind(":player_id", data.playerID);
	query.bind(":original_player_id", data.originalPlayerID);
	query.bind(":nickname", data.nickname);
	query.bind(":hero_id", data.heroId);
	query.bind(":property", data.property);
	query.bind(":level", data.level);
	query.bind(":experience", data.experience);
	query.bind(":ads_removed", data.isAdsRemoved);
	query.bind(":reward_time", data.rewardTime);
	query.bind(":reward_stage", data.rewardStage);
	query.bind(":daily_reward_claim_time", data.dailyRewardClaimTime);
	query.bind(":daily_reward_days", data.dailyRewardDays);
	query.bind(":tutorial_process", data.tutorialProcess);
	query.bind(":accepted_suggestions", data.acceptedSuggestions);
	query.exec();

}

bool LocalPlayer::execSelectPlayerID(std::string const& dbFile, std::string& playerID, std::string& originalPlayerID)
{
	if (!this->setupSQLiteDB(dbFile))
		return false;

	try
	{
		SQLite::Statement query(*m_database, "SELECT player_id, original_player_id FROM " TABLE_NAME_LOCAL_PLAYER);
		bool next = query.executeStep();
		if (!next)
			throw SQLite::Exception(StringUtils::format("No results returned: %s", query.getQuery().c_str()));

		playerID = query.getColumn("player_id").getString();
		originalPlayerID = query.getColumn("original_player_id").getString();

		return true;
	}
	catch (std::exception const& ex)
	{
		CCLOG("LocalPlayer: Failed to access DB file %s. error: %s", dbFile.c_str(), ex.what());
	}
	return false;
}

bool LocalPlayer::createSnapshot(SamplePlayerData& data)
{
	if (m_isLoadingData)
    {
        CCLOG("LocalPlayer: Cannot create player data snapshot because the data is loading.");
        return false;
    }

	data.playerID = m_playerID;
	data.originalPlayerID = m_originalPlayerID;
	data.nickname = m_nickname;
	data.heroId = m_heroId;
	data.property = m_property;
	data.level = m_level;
	data.experience = m_experience;
	data.rewardTime = m_rewardTime;
	data.rewardStage = m_rewardStage;
	data.isAdsRemoved = m_isAdsRemoved;
	data.dailyRewardClaimTime = m_dailyRewardClaimTime;
	data.dailyRewardDays = m_dailyRewardDays;
	data.tutorialProcess = m_tutorialProcess;
	data.acceptedSuggestions = m_acceptedSuggestions;
	data.playerUpdated = m_playerUpdated;
	if (m_playerUpdated)
		m_playerUpdated = false;

	for (auto& statStageList : m_statStageLists)
	{
		auto& objectUpdate = statStageList.second;
		data.statStageLists[statStageList.first] = objectUpdate;
		if (objectUpdate.isUpdated)
			objectUpdate.isUpdated = false;
	}

	for (auto& score : m_scores)
	{
		auto& objectUpdate = score.second;
		data.scores[score.first] = objectUpdate;
		if (objectUpdate.isUpdated)
			objectUpdate.isUpdated = false;
	}

	return true;
}

bool LocalPlayer::saveDataInternal(SamplePlayerData const& data)
{
	if (m_targetDataFile.empty())
	{
		CCLOG("LocalPlayer: Unable to save data, you need to successfully loadData() first.");
		return false;
	}

	NSTime currTime = time_util::getUptimeMillis();

	bool ret = this->saveToDB(m_targetDataFile, data);
	if (ret)
	{
		NSTime diff = time_util::getUptimeMillis() - currTime;
        NS_UNUSED_VARIABLE(diff);
		CCLOG("LocalPlayer: Data save time: %dms.", diff);
	}

	return ret;
}

bool LocalPlayer::saveToDB(std::string const& dbFile, SamplePlayerData const& data)
{
	NS_ASSERT(!data.playerID.empty());
	if(data.playerID.empty())
	{
		CCLOG("LocalPlayer: Cannot save empty Player ID to DB.");
		return false;
	}

	if (!this->setupSQLiteDB(dbFile))
		return false;

	bool ret = true;
	if (data.playerUpdated)
	{
		try
		{
			SQLite::Statement query(*m_database, "SELECT count(*) FROM " TABLE_NAME_LOCAL_PLAYER);
			(void)query.executeStep();
			bool hasPlayer = (1 == query.getColumn(0).getInt());
			if (hasPlayer)
				this->execUpdatePlayer(m_database, data);
			else
				this->execInsertPlayer(m_database, data);
		}
		catch (std::exception const& ex)
		{
			ret = false;
			CCLOG("LocalPlayer: Failed to access DB file %s. error: %s", dbFile.c_str(), ex.what());
		}
	}

	for (auto& statStageList : data.statStageLists)
	{
		if (!statStageList.second.isUpdated)
			continue;

		try
		{
			int32 nStatTypes = (int32)statStageList.second.object.size();
			for (int32 i = 0; i < nStatTypes; ++i)
			{
				SQLite::Statement query(*m_database, "SELECT count(*) FROM " TABLE_NAME_STAT_STAGE " WHERE hero_id=? AND stat_type=?");
				query.bind(1, statStageList.first);
				query.bind(2, i);
				(void)query.executeStep();
				bool hasStatStage = (1 == query.getColumn(0).getInt());
				if (hasStatStage)
					this->execUpdateStatStage(m_database, statStageList.first, i, statStageList.second.object[i]);
				else
					this->execInsertStatStage(m_database, statStageList.first, i, statStageList.second.object[i]);
			}
		}
		catch (std::exception const& ex)
		{
			ret = false;
			CCLOG("LocalPlayer: Failed to access DB file %s. error: %s", dbFile.c_str(), ex.what());
		}
	}

	for (auto& score : data.scores)
	{
		if (!score.second.isUpdated)
			continue;

		try
		{
			SQLite::Statement query(*m_database, "SELECT count(*) FROM " TABLE_NAME_SCORE " WHERE leaderboard_id=?");
			query.bind(1, score.first);
			(void)query.executeStep();
			bool hasScore = (1 == query.getColumn(0).getInt());
			if (hasScore)
				this->execUpdateScore(m_database, score.first, score.second.object);
			else
				this->execInsertScore(m_database, score.first, score.second.object);
		}
		catch (std::exception const& ex)
		{
			ret = false;
			CCLOG("LocalPlayer: Failed to access DB file %s. error: %s", dbFile.c_str(), ex.what());
		}
	}

	return ret;
}

void LocalPlayer::deleteData()
{
	this->closeDB();
	std::string writablePath = FileUtils::getInstance()->getWritablePath();
	std::string dbPath = writablePath + PLAYER_DATA_DIR + "/";
	if (FileUtils::getInstance()->isDirectoryExist(dbPath))
    {
        std::vector<std::string> files = FileUtils::getInstance()->listFiles(dbPath);
        for(auto it = files.begin(); it != files.end(); ++it)
        {
            std::string file = *it;
            if(this->isDBFile(file))
            {
                bool ret = FileUtils::getInstance()->removeFile(file);
                NS_UNUSED_VARIABLE(ret);
                NS_ASSERT(ret);
            }
        }
    }
		
	sUserPreferences->setLastLoggedInPlayerID("");
}


NS_END
