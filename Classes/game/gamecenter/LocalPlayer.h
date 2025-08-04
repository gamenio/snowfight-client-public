//
//  LocalPlayer.h
//  snowfight
//
//  Created by Luthier on 2019/5/3.
//

#ifndef __LOCAL_PLAYER_H__
#define __LOCAL_PLAYER_H__

#include "SQLiteCpp/SQLiteCpp.h"

#include "common/Common.h"
#include "GameCenterProtocol.h"
#include "game/entities/DataPlayer.h"

USING_NS_CC;

NS_BEGIN

class LocalPlayer
{
	template<typename T>
	struct ObjectUpdate
	{
		ObjectUpdate() : 
			object(), 
			isUpdated(false)
		{
		}

		T object;
		bool isUpdated;
	};
	typedef std::unordered_map<std::string/* LeaderboardID */, ObjectUpdate<ScoreInfo>> ScoreInfoMap;
	typedef std::unordered_map<uint32 /* HeroID */, ObjectUpdate<StatStageList>> StatStageListMap;

	struct SamplePlayerData
	{
		std::string playerID;
		std::string originalPlayerID;
		std::string nickname;
		uint32 heroId;
		int32 property;
		uint8 level;
		int32 experience;
		uint32 rewardTime;
		uint8 rewardStage;
		bool isAdsRemoved;
		uint32 dailyRewardClaimTime;
		uint32 dailyRewardDays;
		int32 tutorialProcess;
		uint32 acceptedSuggestions;
		bool playerUpdated;
		ScoreInfoMap scores;
		StatStageListMap statStageLists;
	};
public:
    LocalPlayer();
    ~LocalPlayer();
    
	void setPlayerID(std::string const& playerID) { m_playerID = playerID; }
    std::string const& getPlayerID() const { return m_playerID; }

	void setOriginalPlayerID(std::string const& playerID) { m_originalPlayerID = playerID; }
    std::string const& getOriginalPlayerID() const { return m_originalPlayerID; }
    
    // The nickname the player displays in the game
	void setNickname(std::string const& nickname);
    std::string const& getNickname() const { return m_nickname; }
    
	// The name of the player in the Game Center
    void setDisplayName(std::string const& displayName) { m_displayName = displayName; }
    std::string const& getDisplayName() const { return m_displayName; }
    
	void setHeroId(uint32 heroId);
	uint32 getHeroId() const { return m_heroId; }
    
	void setProperty(int32 property);
	int32 getProperty() const { return m_property; }
	void addMoney(int32 money);
	void removeMoney(int32 money);

	void setLevel(uint8 level);
	uint8 getLevel() const { return m_level; }

	void setExperience(int32 xp);
	int32 getExperience() const { return m_experience; }

    void setScoreValue(std::string const& leaderboardID, int32 newValue);
    int32 getScoreValue(std::string const& leaderboardID) const;
	void setScoreReported(std::string const& leaderboardID, bool reported);
    bool isScoreReported(std::string const& leaderboardID) const;

	void setStatStage(uint32 heroId, StatType statType, uint8 stage);
	uint8 getStatStage(uint32 heroId, StatType statType) const;
	bool getStatStageList(uint32 heroId, StatStageList& result) const;

	uint8 getRewardStage() const { return m_rewardStage; }
	void setRewardStage(uint8 stage);
	uint32 getRewardTime() const { return m_rewardTime; }
	void setRewardTime(uint32 seconds);

	void setAdsRemoved(bool removed);
	bool isAdsRemoved() const{ return m_isAdsRemoved; }

	void setDailyRewardClaimTime(uint32 seconds);
	uint32 getDailyRewardClaimTime() const { return m_dailyRewardClaimTime; }
	void increaseDailyRewardDays();
	void setDailyRewardDays(int32 days);
	int32 getDailyRewardDays() const { return m_dailyRewardDays; }
	
	bool isTrainee() const { return m_tutorialProcess < TUTORIAL_PROCESS_FIND_ENEMY; }
	int32 getTutorialProcess() const { return m_tutorialProcess; }
	void setTutorialProcess(int32 process);
	bool isTutorialCompleted() const { return m_tutorialProcess >= MAX_TUTORIAL_PROCESSES - 1; }

	void setSuggestionAccepted(uint32 suggestion, bool accepted);
	bool isSuggestionAccepted(uint32 suggestion) const;
	uint32 getAcceptedSuggestions() { return m_acceptedSuggestions; }

	void unlockHero(uint32 heroId);
	bool hasUnlockedHero(uint32 heroId) const;
    
    void setAuthenticated(bool authenticated) { m_authenticated = authenticated; }
    bool isAuthenticated() const { return m_authenticated; }
    
    void setUnderage(bool underage) { m_underage = underage; }
    bool isUnderage() const { return m_underage; }
    
	// Is it a guest
    bool isGuest() const { return m_isGuest; }
    
    void setupDefaultValues();
    
	// Load data asynchronously. If a callback is set it will give a success status in the callback parameter.
	// If an attribute value is modified during the loading of data, the modification will likely be overwritten.
	void loadDataAsync(std::function<void(bool)> callback = nullptr);
    bool isLoadingData() const { return m_isLoadingData; }

	bool saveData();
	// Saves data asynchronously. If a callback is set it will give a success status in the callback parameter.
	void saveDataAsync(std::function<void(bool)> callback = nullptr);

	// Delete data asynchronously
	void deleteDataAsync(std::function<void()> callback = nullptr);
    
private:
	bool switchToNewPlayerID(std::string const& oldPlayerID, std::string const& newPlayerID);
	bool setupSQLiteDB(std::string const& dbFile);
	void closeDB();
	std::string getDBPath(std::string const& dbFile);
	bool isDBFile(std::string const& file) const;
	bool updateValuesFromDB(std::string const& dbFile);

	void execUpgradeToNewDB(int32 oldVersion, SQLite::Database* m_database);

	void execUpdatePlayer(SQLite::Database* m_database, SamplePlayerData const& data);
	void execInsertPlayer(SQLite::Database* m_database, SamplePlayerData const& data);
	bool execSelectPlayerID(std::string const& dbFile, std::string& playerID, std::string& originalPlayerID);
	bool execUpdatePlayerID(std::string const& dbFile, std::string const& playerID, std::string const& originalPlayerID);

	void execUpdateScore(SQLite::Database* m_database, std::string const& leaderboardID, ScoreInfo const& score);
	void execInsertScore(SQLite::Database* m_database, std::string const& leaderboardID, ScoreInfo const& score);

	void execUpdateStatStage(SQLite::Database* m_database, uint32 heroId, int32 statType, uint8 stage);
	void execInsertStatStage(SQLite::Database* m_database, uint32 heroId, int32 statType, uint8 stage);

	bool loadData();
	bool createSnapshot(SamplePlayerData& data);
	bool saveDataInternal(SamplePlayerData const& data);
	bool saveToDB(std::string const& dbFile, SamplePlayerData const& data);
	void deleteData();

	std::string generatePlayerID() const;
    std::string hashPlayerID(std::string const& playerID) const;

    std::string m_playerID;
	std::string m_originalPlayerID;
    bool m_isGuest;
    std::string m_nickname;
    std::string m_displayName;
    uint32 m_heroId;
	int32 m_property;
	uint8 m_level;
	int32 m_experience;
	uint32 m_rewardTime;
	uint8 m_rewardStage;
	bool m_isAdsRemoved;
	uint32 m_dailyRewardClaimTime;
	uint32 m_dailyRewardDays;
	int32 m_tutorialProcess;
	uint32 m_acceptedSuggestions;
	bool m_playerUpdated;
	ScoreInfoMap m_scores;
	StatStageListMap m_statStageLists;
    bool m_isLoadingData;
    
    bool m_authenticated;
    bool m_underage;
    
    std::string m_targetDataFile;
	SQLite::Database* m_database;
};

NS_END


#endif // __LOCAL_PLAYER_H__


