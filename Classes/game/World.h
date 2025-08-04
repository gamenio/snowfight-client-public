#ifndef __WORLD_H__
#define __WORLD_H__

#include "common/Common.h"
#include "common/utils/MessageDispatcher.h"
#include "common/network/ISocketControl.h"
#include "game/entities/DataTypes.h"
#include "game/entities/DataPlayer.h"
#include "game/gamecenter/GameCenter.h"
#include "game/maps/BattleMap.h"
#include "game/client/WorldSession.h"
#include "game/client/AuthSession.h"
#include "WorldListeners.h"

NS_BEGIN

struct WorldConfig
{
	WorldConfig() :
		serverAddr(""),
		serverPort(0)
	{ }
	std::string serverAddr;
	uint16 serverPort;
};

class Object;
class MyCharacter;

class World
{
public:
	static World* getInstance();
	static void destoryInstance();

	World();
	~World();

	// Get your own character data. If the character has not joined the world 
	// or the session has been closed, null will be returned.
	DataPlayer* getMyself() const;

	// Get the world's message dispatcher
	MessageDispatcher* getDispatcher() const { return m_dispatcher; }

	void start();
	World* configure(WorldConfig const& config);
	WorldConfig const& getWorldConfig() const { return m_worldConfig; }

	void stopDelayed();
	bool isStopped() const { return m_isStopped; }

	void pause();
	void resume();

	void update(float delta);

	void startAuthSocket(std::string const& address, uint16 port);
	void startWorldSocket(std::string const& address, uint16 port);
	bool tryRestoreConnection();

	MyCharacter* getMyCharacter() const { if (m_map) return m_map->getMyChar(); return nullptr; }
	WorldSession::PlayerConfig const& getPlayerConfig() const { return m_playerConfig; }
	bool logoutPlayer();
	bool isLocalPlayerAuthed(LocalPlayer* localPlayer) const;

	Session* getSession() const { return m_currSession; }
	// Set the current session. The function is thread-safe.
	void setSession(Session* newSession);

	BattleMap* getMap() const { NS_ASSERT_LOG(m_map, "Map was not created"); return m_map; }
	BattleMap* createMapIfNotExist(MapData* data, uint8 combatGrade);

	void addToWorld(Object* object);
	void removeFromWorld(ObjectGuid const& guid, bool cleanup);
	Object* findObject(ObjectGuid const& guid, bool includeInactiveObjects = false) const;

	// Network status
	void setNetworkStatusListener(NetworkStatusListener* listener) { m_networkStatusListener = listener; }

	// Logon
	bool sendLogon(AuthSession::LogonConfig const& config);
	bool sendGetRealmList();
	void setLogonListener(LogonListener* listener) { m_logonListener = listener; }

	// World authorization
	bool sendAuthProof(std::string const& proof, std::string const& playerId, std::string const& originalPlayerId, uint32 requiredCapabilities);
	bool resendAuthProof();
	void setWorldAuthListener(WorldAuthListener* listener) { m_worldAuthListener = listener; }
	void setWorldInitListener(WorldInitListener* listener) { m_worldInitListener = listener; }

	// Player login and character status
	bool sendPlayerLogin(WorldSession::PlayerConfig const& config);
	bool sendJoinTheater();
	bool sendSmiley(uint16 code);
	void addMyCharacterListener(MyCharacterListener* listener);
	void removeMyCharacterListener(MyCharacterListener* listener);

	// Game message
	void addMessageListener(MessageListener* listener);
	void removeMessageListener(MessageListener* listener);

	// Battle status
	void setBattleUpdateListener(BattleUpdateListener* listener) { m_battleUpdateListener = listener; }

	// World Status
	bool sendQueryWorldStatus();
	bool sendQueryTheaterStatusList();
	bool sendQueryPlayerStatusList(uint32 theaterId);
	void addWorldStatusListener(WorldStatusListener* listener);
	void removeWorldStatusListener(WorldStatusListener* listener);

	// GM command
	bool sendGMCommand(std::string const & line);

	// Item
	bool sendUseItem(int32 slot, ObjectGuid const& item);
	void addCarriedItemListener(CarriedItemListener* listener);
	void removeCarriedItemListener(CarriedItemListener* listener);
	void addItemApplicationListener(ItemApplicationListener* listener);
	void removeItemApplicationListener(ItemApplicationListener* listener);

	// Projectile
	void addProjectileListener(ProjectileListener* listener);
	void removeProjectileListener(ProjectileListener* listener);

	// Object lifecycle
	void setWorldLifecycleListener(WorldLifecycleListener* listener) { m_worldLifecycleListener = listener; }
	void setObjectLifecycleListener(ObjectLifecycleListener* listener);
	void setLocatorObjectLifecycleListener(LocatorObjectLifecycleListener* listener);
	void setInventoryItemLifecycleListener(InventoryItemLifecycleListener* listener);
	void setEquipmentItemLifecycleListener(EquipmentItemLifecycleListener* listener);

private:
	friend AuthSession;
	friend WorldSession;

	// LogonListener
	void notifyLogonSucceeded();
	void notifyLogonFailed(LogonResult::ErrorCode errorCode);
	void notifyFetchRealmListSucceeded(Realm const& preferredRealm);
	void notifyFetchRealmlistFailed();

	// WorldAuthListener
	void notifyWorldSessionTimedout();
	void notifyWorldAuthSucceeded();
	void notifyWorldWaitQueue(int32 waitPos);
	void notifyWorldSessionExpired();
	void notifyWorldRecvQueueFulL();
	void notifyWorldAuthFailed();

	// NetworkStatusListener
	void notifyNetworkError(NetworkError const& error);
	void notifyNetworkRestored();

	// MyCharacterListener
	void notifyTheaterInfo(TheaterInfo const& info);
	void notifyWaitForPlayers(WaitForPlayers const& waitForPlayers);
	void notifyCharacterInfo(CharacterInfo const& info);
	void notifyRewardMessage(RewardMessage const& message);
	void notifyBattleResult(BattleResult const& result);
	void notifyItemPickupResult(ItemPickupResult const& result);

	// BattleUpdateListener
	void notifyBattleUpdate(BattleUpdate const& update);

	// WorldStatusListener
	void notifyWorldStatus(WorldStatus const& status);
	void notifyTheaterStatusList(TheaterStatusList const& statusList);
	void notifyPlayerStatusList(PlayerStatusList const& statusList);

	// MessageListener
	void notifyFlashMessage(FlashMessage const& flashMsg);
	void notifyPlayerActionMessage(PlayerActionMessage const& message);
	void notifyDeathMessage(DeathMessage const& message);
	void notifyItemActionMessage(ItemActionMessage const& message);

	// ProjectileListener
	void notifyLaunchResult(LaunchResult const& result);

	// CarriedItemListener
	void notifyItemUseResult(ItemUseResult const& result);

	// ItemApplicationListener
	void notifyItemApplicationUpdate(ItemApplicationUpdate const& update);
	void notifyItemApplicationUpdateAll(ItemApplicationUpdateAll const& updateAll);

	void handleMessage(Message const& message);

	void removeSession();
	void removeMySelf();
	
	bool m_isStopping;
	bool m_isStopped;

	Scheduler* m_scheduler;
	BattleMap* m_map;

	Session* m_currSession;
	std::mutex m_sessMutex;
	Session* m_newSession;
	WorldSession::PlayerConfig m_playerConfig;
	WorldConfig m_worldConfig;

	ISocketControl* m_socketCtrl;

	MessageDispatcher* m_dispatcher;

	WorldLifecycleListener* m_worldLifecycleListener;
	LogonListener* m_logonListener;
	WorldAuthListener* m_worldAuthListener;
	WorldInitListener* m_worldInitListener;
	NetworkStatusListener* m_networkStatusListener;
	std::vector<MyCharacterListener*> m_myCharacterListeners;
	BattleUpdateListener* m_battleUpdateListener;
	std::vector<WorldStatusListener*> m_worldStatusListeners;
	std::vector<MessageListener*> m_messageListeners;
	std::vector<ProjectileListener*> m_projectileListeners;
	std::vector<CarriedItemListener*> m_carriedItemListeners;
	std::vector<ItemApplicationListener*> m_itemApplicationListeners;
};

NS_END

#endif // __WORLD_H__

