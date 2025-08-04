#ifndef __WORLD_SESSION_H__
#define __WORLD_SESSION_H__

#include "common/Common.h"
#include "common/utils/MessageDispatcher.h"
#include "game/entities/updates/UpdateObject.h"
#include "game/entities/DataPlayer.h"
#include "game/MessageEnum.h"
#include "WorldSocket.h"
#include "Session.h"


NS_BEGIN


class World;
class Unit;
class MyCharacter;

class WorldSession : public Session
{
public:
	enum RequiredCapabilities
	{
		REQUIRES_ALLOW_PLAYER_TO_RESTORE = 0x00000001
	};

	struct PlayerConfig
	{
		PlayerConfig() :
			charName(""),
			country(""),
			lang(""),
			charId(0),
			totalKills(0),
			property(0),
			level(0),
			experience(0),
			statStageList(),
			winSize(cocos2d::Size::ZERO),
			controllerType(CONTROLLER_TYPE_DUAL_STICKS),
			isMoveEnabled(false),
			isTrainee(false),
			rewardStage(0),
			dailyRewardDays(0)
		{ }

		std::string charName;
		std::string country;
		std::string lang;
		uint32 charId;
		int32 totalKills;
		int32 property;
		uint8 level;
		int32 experience;
		StatStageList statStageList;
		cocos2d::Size winSize;
		ControllerType controllerType;
		bool isMoveEnabled;
		bool isTrainee;
		uint8 rewardStage;
		int32 dailyRewardDays;
	};

	WorldSession(std::shared_ptr<WorldSocket> const& socket);
	~WorldSession();

	World* getWorld() const { return m_world; }
	void setNewSocket(std::shared_ptr<WorldSocket> const& socket);

	void setSessionId(uint32 sessionId) { m_sessionId = sessionId; }
	uint32 getSessionId() const { return m_sessionId; }
    
    std::string getPlayerId() const { return m_playerId; }
    std::string getOriginalPlayerId() const { return m_originalPlayerId; }

	uint32 getRequiredCapabilities() const { return m_requiredCapabilities; }
	uint32 getTheaterId() const { return m_theaterId; }

	void setMyCharacter(MyCharacter* myChar) { m_myCharacter = myChar; }
	MyCharacter* getMyCharacter() const { return m_myCharacter; }
	void logoutPlayer();

	// Set GM permission level
	void setGMLevel(uint8 level) { m_gmLevel = level; }
	uint8 getGMLevel() const { return m_gmLevel; }
	bool hasGMPermission() const { return m_gmLevel > 0; }

	bool addToRecvQueue(WorldPacket&& packet);
	void sendPacket(WorldPacket&& packet);

	// Log out the player and close the session
	void closeDelayed() override;
	void close() override;
	bool isClosing() const override { return m_isClosing; }
	bool isClosed() const override { return m_isClosed; }

	virtual bool update(float delta) override;

	// Session timeout timer
	void resetTimeoutTimer();
	void setTimeoutEnabled(bool enabled) { m_isTimeoutEnabled = enabled; }

    // Ping
	void startPing();
	void stopPing();
	void setLatency(NSTime latency) { m_latency = latency; }
	NSTime getLatency() const { return m_latency; }
    
	// WorldAuthHandler
	void handleAuthChallenge(WorldPacket& recvPacket);
	void handleAuthVerdict(WorldPacket& recvPacket);
	void sendAuthProof(std::string const& proof, std::string const& playerId, std::string const& originalPlayerId, uint32 requiredCapabilities);
	void resendAuthProof();

	// CharacterHandler
	void handleTheaterInfo(WorldPacket& recvPacket);
	void handleWaitForPlayers(WorldPacket& recvPacket);
	void handleCharacterInfo(WorldPacket& recvPacket);
	void handleTransport(WorldPacket& recvPacket);
	void handleRewardMessage(WorldPacket& recvPacket);
	void handleBattleResult(WorldPacket& recvPacket);
	void handleModifyStamina(WorldPacket& recvPacket);
	void handleItemPickupResult(WorldPacket& recvPacket);
	void sendPlayerLogin(PlayerConfig const& config);
	void sendJoinTheater();
	void sendQueryCharacterInfo(Unit* unit);
    void sendSmiley(uint16 code);

	// GMCommandHandler
	void sendGMCommand(std::string const& line);

	// MovementHandler
	void handleMovementInfo(WorldPacket& recvPacket);

	// CombatHandler
	void handleLaunchResult(WorldPacket& recvPacket);
	void handleStaminaInfo(WorldPacket& recvPacket);

	// UpdateObjectHandler
	void handleDestroyObject(WorldPacket& recvPacket);
	void handleUpdateObject(WorldPacket& recvPacket);

	// WorldStatusHandler
	void sendQueryWorldStatus();
	void handleWorldStatus(WorldPacket& recvPacket);
	void sendQueryTheaterStatusList();
	void handleTheaterStatusList(WorldPacket& recvPacket);
	void sendQueryPlayerStatusList(uint32 theaterId);
	void handlePlayerStatusList(WorldPacket& recvPacket);

	// MessageHandler
	void handleFlashMessage(WorldPacket& recvPacket);
	void handlePlayerActionMessage(WorldPacket& recvPacket);
	void handleDeathMessage(WorldPacket& recvPacket);
	void handleItemActionMessage(WorldPacket& recvPacket);

	// BattleUpdateHandler
	void handleBattleUpdate(WorldPacket& recvPacket);

	// RelocateLocatorHandler
	void handleRelocateLocator(WorldPacket& recvPacket);

	// ItemHandler
	void sendUseItem(int32 slot, ObjectGuid const& item);
	void handleItemUseResult(WorldPacket& recvPacket);
	void handleItemApplicationUpdate(WorldPacket& recvPacket);
	void handleItemApplicationUpdateAll(WorldPacket& recvPacket);
	void handleItemCooldownList(WorldPacket& recvPacket);

private:
	void handleMessage(Message const& message);
	bool applyNewSocket();
	void checkTimeout(float delta);

	// Object update
	void onObjectCreateFinished(uint32 updateFlags, DataBasic* data, void* holder);
	void onValuesUpdateFinished(uint32 updateFlags, DataBasic* data, void* holder);
	DataBasic* onUpdateValues(uint32 updateFlags, ObjectGuid const& guid, void** outHolder);
	DataBasic* onUpdateCreate(uint32 updateFlags, ObjectGuid const& guid, DataTypeID typeId, void** outHolder);
	void onUpdateOutOfRange(std::vector<ObjectGuid> const& guids);

	std::shared_ptr<WorldSocket> m_socket;
	std::shared_ptr<WorldSocket> m_newSocket;
	LockedQueue<WorldPacket> m_recvQueue;

	uint32 m_sessionId;
	std::string m_authProof;
	std::string m_playerId;
	std::string m_originalPlayerId;

	uint32 m_requiredCapabilities;
	uint32 m_theaterId;

	World* m_world;
	SocketMgr<WorldSocket, World>* m_socketMgr;

	MyCharacter* m_myCharacter;
	bool m_isPlayerLoggedIn;
	uint8 m_gmLevel;

	bool m_isClosing;
	bool m_isClosed;
    std::atomic<NSTime> m_latency;

	bool m_isTimeoutEnabled;
	std::mutex m_timeoutTimerMutex;
	DelayTimer m_timeoutTimer;
};

NS_END

#endif // __WORLD_SESSION_H__

