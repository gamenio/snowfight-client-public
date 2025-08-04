#include "World.h"

#include "game/client/AuthSocketMgr.h"
#include "game/client/WorldSocketMgr.h"
#include "game/client/AuthSession.h"
#include "game/client/WorldSession.h"
#include "game/behaviors/MyCharacter.h"
#include "game/behaviors/LocatorObject.h"
#include "game/behaviors/CarriedItem.h"
#include "firservice/FirebaseService.h"

NS_BEGIN

static World* s_instance = nullptr;

World* World::getInstance()
{
	if (!s_instance)
	{
		s_instance = new (std::nothrow) World;
		NS_ASSERT_LOG(s_instance, "FATAL: Not enough memory");
	}
	return s_instance;
}

void World::destoryInstance()
{
	CC_SAFE_DELETE(s_instance);
}

World::World() :
	m_scheduler(nullptr),
	m_isStopping(false),
	m_isStopped(true),
	m_map(nullptr),
	m_currSession(nullptr),
	m_newSession(nullptr),
	m_socketCtrl(nullptr),
	m_dispatcher(new MessageDispatcher()),
	m_worldLifecycleListener(nullptr),
	m_logonListener(nullptr),
	m_worldAuthListener(nullptr),
	m_worldInitListener(nullptr),
	m_networkStatusListener(nullptr),
	m_battleUpdateListener(nullptr)
{
	this->getDispatcher()->registerHandler(SOCKMSG_AUTH_NETWORK_ERROR, this, &World::handleMessage);
	this->getDispatcher()->registerHandler(SOCKMSG_WORLD_NETWORK_ERROR, this, &World::handleMessage);

	m_scheduler = Director::getInstance()->getScheduler();
	CC_SAFE_RETAIN(m_scheduler);
	m_scheduler->scheduleUpdate(this, UPDATE_PRIORITY_WORLD, false);
}


World::~World()
{
	this->getDispatcher()->removeHandlersWithTarget(this);
	if (m_scheduler)
	{
		m_scheduler->unscheduleUpdate(this);
		CC_SAFE_RELEASE_NULL(m_scheduler);
	}

	CC_SAFE_DELETE(m_map);
	CC_SAFE_DELETE(m_currSession);
	CC_SAFE_DELETE(m_socketCtrl);
	CC_SAFE_DELETE(m_dispatcher);

	m_newSession = nullptr;

	m_worldLifecycleListener = nullptr;
	m_logonListener = nullptr;
	m_worldAuthListener = nullptr;
	m_worldInitListener = nullptr;
	m_networkStatusListener = nullptr;
	m_battleUpdateListener = nullptr;
}

DataPlayer* World::getMyself() const
{
	if (this->getMyCharacter()) 
		return this->getMyCharacter()->getData(); 

	return nullptr;
}

void World::start()
{
	if (m_isStopping || !m_isStopped)
		return;
	m_isStopped = false;

	NS_ASSERT_LOG(!m_worldConfig.serverAddr.empty(), "Server IP address is not configured.");
	NS_ASSERT_LOG(m_worldConfig.serverPort > 0, "Server port is not configured.");

	this->startAuthSocket(m_worldConfig.serverAddr, m_worldConfig.serverPort);
}

World* World::configure(WorldConfig const& config)
{
	m_worldConfig = static_cast<WorldConfig const&>(config);
	return this;
}


void World::stopDelayed()
{
	if (m_isStopped || m_isStopping)
		return;

	// Wait for the session to close
	if (m_currSession)
	{
		if (!m_isStopping)
		{
			m_isStopping = true;
			m_currSession->closeDelayed();
		}
	}
	else
	{
		this->removeMySelf();
		m_isStopped = true;
		if (m_worldLifecycleListener)
			m_worldLifecycleListener->onWorldStopped(this);
	}
}

void World::pause()
{
	if (m_map)
		m_map->pause();

	if (m_currSession && m_currSession->asWorldSession())
		m_currSession->asWorldSession()->stopPing();
}

void World::resume()
{
	if (m_map)
		m_map->resume();

	if (m_currSession && m_currSession->asWorldSession())
		m_currSession->asWorldSession()->startPing();
}

void World::update(float delta)
{
	// Switch to a new session
	std::unique_lock<std::mutex> lock(m_sessMutex);
	if (m_newSession)
	{
		if (m_currSession)
		{
			NS_ASSERT_LOG(m_currSession->isClosed(), "A session exists in the world and it's not closed.");
			if (m_currSession->isClosed())
				this->removeSession();
		}
		m_currSession = m_newSession;

		// Notify UI that the session has been opened
		if (m_currSession->getType() == SESSION_TYPE_WORLD)
		{
			if (m_worldAuthListener)
				m_worldAuthListener->onWorldSessionOpened(m_currSession->asWorldSession());
		}
		else
		{
			if (m_logonListener)
				m_logonListener->onAuthSessionOpened(m_currSession->asAuthSession());
		}

		m_newSession = nullptr;
	}
	lock.unlock();

	// Update the session, and if the return value is false, remove it afterwards
	bool removeSess = m_currSession && !m_currSession->update(delta);

	// Dispatch message
	m_dispatcher->dispatch();

	// Update business module associated with WorldSession
	if (m_currSession && m_currSession->getType() == SESSION_TYPE_WORLD)
	{
		if (m_map)
			m_map->update(delta);
	}

	// Remove current session
	if (removeSess)
		this->removeSession();

}

void World::startAuthSocket(std::string const& address, uint16 port)
{
	if (m_socketCtrl)
	{
		m_socketCtrl->stop();
		CC_SAFE_DELETE(m_socketCtrl);
	}

	m_socketCtrl = new AuthSocketMgr(this);
	m_socketCtrl->start(address, port);
}

void World::startWorldSocket(std::string const& address, uint16 port)
{
	if (m_socketCtrl)
	{
		m_socketCtrl->stop();
		CC_SAFE_DELETE(m_socketCtrl);
	}

	m_socketCtrl = new WorldSocketMgr(this);
	m_socketCtrl->start(address, port);
}

bool World::tryRestoreConnection()
{
	if (!m_currSession 
		|| m_currSession->isClosed() 
		|| m_currSession->isClosing())
	{
		CCLOG("Unable to restore connection because session is closed.");
		return false;
	}

	if (!m_socketCtrl)
		return false;

	if (!m_socketCtrl->isStopped())
	{
		CCLOG("Unable to restore connection because socket is not closed.");
		return false;
	}

	m_socketCtrl->restart();

	return true;
}

bool World::logoutPlayer()
{
	if (m_currSession && m_currSession->asWorldSession())
	{
		m_currSession->asWorldSession()->logoutPlayer();
		return true;
	}

	return false;
}

bool World::isLocalPlayerAuthed(LocalPlayer* localPlayer) const
{
	if (!localPlayer->isLoadingData())
	{
		if (m_currSession)
		{
			WorldSession* session = m_currSession->asWorldSession();
			if (session)
				return session->getPlayerId() == localPlayer->getPlayerID();
		}
	}
	return false;
}

void World::setSession(Session* newSession)
{
	std::lock_guard<std::mutex> lock(m_sessMutex);

	NS_ASSERT_LOG(m_newSession == nullptr, "A new session exists in the world and it's not unhandled.");
	m_newSession = newSession;
}

BattleMap* World::createMapIfNotExist(MapData* data, uint8 combatGrade)
{
	if (!m_map)
		m_map = new BattleMap(data, this, combatGrade);

	return m_map;
}

void World::removeFromWorld(ObjectGuid const& guid, bool cleanup)
{
	NS_ASSERT(m_map != nullptr);

	if (guid.isWorldObject())
		m_map->removeFromMap(guid, cleanup);
	else if (guid.isLocatorObject())
		m_map->removeLocatorObject(guid, cleanup);
	else if(guid.isCarriedItem())
		assertNotNull(m_map->getMyChar())->removeCarriedItem(guid, cleanup);
	else
		NS_ASSERT_LOG(false, "World: Unhandled object type");
}

void World::addToWorld(Object* object)
{
	NS_ASSERT(m_map != nullptr);

	if (object->isType(TYPEMASK_WORLDOBJECT))
	{
		if (object->getTypeID() == TYPEID_MYCHARACTER)
		{
			m_map->addMyCharToMap(static_cast<MyCharacter*>(object));
			if (m_worldInitListener)
				m_worldInitListener->onInitSelfCompleted(object->getData()->asDataPlayer());
		}
		else
			m_map->addToMap(static_cast<WorldObject*>(object));
	}
	else if (object->isType(TYPEMASK_LOCATOR_OBJECT))
		m_map->addLocatorObject(static_cast<LocatorObject*>(object));
	else if(object->isType(TYPEMASK_CARRIED_ITEM))
		assertNotNull(m_map->getMyChar())->addCarriedItem(static_cast<CarriedItem*>(object));
	else
		NS_ASSERT_LOG(false, "World: Unhandled object type");
}

Object* World::findObject(ObjectGuid const& guid, bool includeInactiveObjects) const
{
	NS_ASSERT(m_map != nullptr);
	Object* object = nullptr;
	if (guid.isWorldObject())
		object = m_map->findObject(guid, includeInactiveObjects);
	else if (guid.isLocatorObject())
		object = m_map->findLocatorObject(guid, includeInactiveObjects);
	else if (guid.isCarriedItem())
		object = assertNotNull(m_map->getMyChar())->findCarriedItem(guid, includeInactiveObjects);
	else
		NS_ASSERT_LOG(false, "World: Unhandled object type");

	return object;
}

bool World::sendLogon(AuthSession::LogonConfig const& config)
{
	if (m_currSession && m_currSession->asAuthSession())
	{
		m_currSession->asAuthSession()->sendLogonChallenge(config);
		return true;
	}
	return false;
}

bool World::sendGetRealmList()
{
	if (m_currSession && m_currSession->asAuthSession())
	{
		m_currSession->asAuthSession()->sendGetRealmList();
		return true;
	}

	return false;
}

bool World::sendAuthProof(std::string const& proof, std::string const& playerId, std::string const& originalPlayerId, uint32 requiredCapabilities)
{
	if (m_currSession && m_currSession->asWorldSession())
	{
		m_currSession->asWorldSession()->sendAuthProof(proof, playerId, originalPlayerId, requiredCapabilities);
		return true;
	}


	return false;
}

bool World::resendAuthProof()
{
	if (m_currSession && m_currSession->asWorldSession())
	{
		m_currSession->asWorldSession()->resendAuthProof();
		return true;
	}

	return false;
}

bool World::sendPlayerLogin(WorldSession::PlayerConfig const& config)
{
	if (m_currSession && m_currSession->asWorldSession())
	{
		m_playerConfig = config;
		m_currSession->asWorldSession()->sendPlayerLogin(config);
		return true;
	}


	return false;
}

bool World::sendJoinTheater()
{
	if (m_currSession && m_currSession->asWorldSession())
	{
		m_currSession->asWorldSession()->sendJoinTheater();
		return true;
	}

	return false;
}

bool World::sendSmiley(uint16 code)
{
	if (m_currSession && m_currSession->asWorldSession())
	{
		m_currSession->asWorldSession()->sendSmiley(code);
		return true;
	}

	return false;
}

void World::addMyCharacterListener(MyCharacterListener* listener)
{
	auto it = std::find(m_myCharacterListeners.begin(), m_myCharacterListeners.end(), listener);
	if (it == std::end(m_myCharacterListeners))
		m_myCharacterListeners.push_back(listener);
}

void World::removeMyCharacterListener(MyCharacterListener* listener)
{
	m_myCharacterListeners.erase(std::remove(m_myCharacterListeners.begin(), m_myCharacterListeners.end(), listener), m_myCharacterListeners.end());
}

void World::addMessageListener(MessageListener* listener)
{
	auto it = std::find(m_messageListeners.begin(), m_messageListeners.end(), listener);
	if (it == std::end(m_messageListeners))
		m_messageListeners.push_back(listener);
}

void World::removeMessageListener(MessageListener* listener)
{
	m_messageListeners.erase(std::remove(m_messageListeners.begin(), m_messageListeners.end(), listener), m_messageListeners.end());
}

bool World::sendQueryWorldStatus()
{
	if (m_currSession && m_currSession->asWorldSession())
	{
		m_currSession->asWorldSession()->sendQueryWorldStatus();
		return true;
	}

	return false;
}

bool World::sendQueryTheaterStatusList()
{
	if (m_currSession && m_currSession->asWorldSession())
	{
		m_currSession->asWorldSession()->sendQueryTheaterStatusList();
		return true;
	}

	return false;
}

bool World::sendQueryPlayerStatusList(uint32 theaterId)
{
	if (m_currSession && m_currSession->asWorldSession())
	{
		m_currSession->asWorldSession()->sendQueryPlayerStatusList(theaterId);
		return true;
	}

	return false;
}

void World::addWorldStatusListener(WorldStatusListener* listener)
{
	auto it = std::find(m_worldStatusListeners.begin(), m_worldStatusListeners.end(), listener);
	if (it == std::end(m_worldStatusListeners))
		m_worldStatusListeners.push_back(listener);
}

void World::removeWorldStatusListener(WorldStatusListener* listener)
{
	m_worldStatusListeners.erase(std::remove(m_worldStatusListeners.begin(), m_worldStatusListeners.end(), listener), m_worldStatusListeners.end());
}

bool World::sendGMCommand(std::string const& line)
{
	if (m_currSession && m_currSession->asWorldSession())
	{
		m_currSession->asWorldSession()->sendGMCommand(line);
		return true;
	}

	return false;
}

bool World::sendUseItem(int32 slot, ObjectGuid const& item)
{
	if (m_currSession && m_currSession->asWorldSession())
	{
		m_currSession->asWorldSession()->sendUseItem(slot, item);
		return true;
	}

	return false;
}

void World::addCarriedItemListener(CarriedItemListener* listener)
{
	auto it = std::find(m_carriedItemListeners.begin(), m_carriedItemListeners.end(), listener);
	if (it == std::end(m_carriedItemListeners))
		m_carriedItemListeners.push_back(listener);
}

void World::removeCarriedItemListener(CarriedItemListener* listener)
{
	m_carriedItemListeners.erase(std::remove(m_carriedItemListeners.begin(), m_carriedItemListeners.end(), listener), m_carriedItemListeners.end());
}

void World::addItemApplicationListener(ItemApplicationListener* listener)
{
	auto it = std::find(m_itemApplicationListeners.begin(), m_itemApplicationListeners.end(), listener);
	if (it == std::end(m_itemApplicationListeners))
		m_itemApplicationListeners.push_back(listener);
}

void World::removeItemApplicationListener(ItemApplicationListener* listener)
{
	m_itemApplicationListeners.erase(std::remove(m_itemApplicationListeners.begin(), m_itemApplicationListeners.end(), listener), m_itemApplicationListeners.end());
}

void World::addProjectileListener(ProjectileListener* listener)
{
	auto it = std::find(m_projectileListeners.begin(), m_projectileListeners.end(), listener);
	if (it == std::end(m_projectileListeners))
		m_projectileListeners.push_back(listener);
}

void World::removeProjectileListener(ProjectileListener* listener)
{
	m_projectileListeners.erase(std::remove(m_projectileListeners.begin(), m_projectileListeners.end(), listener), m_projectileListeners.end());
}

void World::setObjectLifecycleListener(ObjectLifecycleListener* listener)
{
	NS_ASSERT(m_map);
	m_map->setObjectLifecycleListener(listener);
}

void World::setLocatorObjectLifecycleListener(LocatorObjectLifecycleListener* listener)
{
	NS_ASSERT(m_map);
	m_map->setLocatorObjectLifecycleListener(listener);
}

void World::setInventoryItemLifecycleListener(InventoryItemLifecycleListener* listener)
{
	NS_ASSERT(m_map);
	MyCharacter* myChar = m_map->getMyChar();
	NS_ASSERT(myChar);
	myChar->setInventoryItemLifecycleListener(listener);
}

void World::setEquipmentItemLifecycleListener(EquipmentItemLifecycleListener* listener)
{
	NS_ASSERT(m_map);
	MyCharacter* myChar = m_map->getMyChar();
	NS_ASSERT(myChar);
	myChar->setEquipmentItemLifecycleListener(listener);
}

void World::notifyLogonSucceeded()
{
	if (m_logonListener)
		m_logonListener->onLogonSucceeded();
}

void World::notifyLogonFailed(LogonResult::ErrorCode errorCode)
{
	if (m_logonListener)
		m_logonListener->onLogonFailed(errorCode);
}

void World::notifyFetchRealmListSucceeded(Realm const& preferredRealm)
{
	if (m_logonListener)
		m_logonListener->onFetchRealmListSucceeded(preferredRealm);
}

void World::notifyFetchRealmlistFailed()
{
	if (m_logonListener)
		m_logonListener->onFetchRealmlistFailed();
}

void World::notifyWorldSessionTimedout()
{
	if (m_worldAuthListener)
		m_worldAuthListener->onWorldSessionTimedout();
}

void World::notifyWorldAuthSucceeded()
{
	if (m_worldAuthListener)
		m_worldAuthListener->onWorldAuthSucceeded();
}

void World::notifyWorldWaitQueue(int32 waitPos)
{
	if (m_worldAuthListener)
		m_worldAuthListener->onWorldWaitQueue(waitPos);
}

void World::notifyWorldSessionExpired()
{
	if (m_worldAuthListener)
		m_worldAuthListener->onWorldSessionExpired();
}

void World::notifyWorldRecvQueueFulL()
{
	if (m_worldAuthListener)
		m_worldAuthListener->onWorldRecvQueueFulL();
}

void World::notifyWorldAuthFailed()
{
	if (m_worldAuthListener)
		m_worldAuthListener->onWorldAuthFailed();
}

void World::notifyNetworkError(NetworkError const& error)
{
	if (m_networkStatusListener)
		m_networkStatusListener->onNetworkError(error);
}

void World::notifyNetworkRestored()
{
	if (m_networkStatusListener)
		m_networkStatusListener->onNetworkRestored();
}

void World::notifyTheaterInfo(TheaterInfo const& info)
{
	for (MyCharacterListener* listen : m_myCharacterListeners)
		listen->onTheaterInfo(info);
}

void World::notifyWaitForPlayers(WaitForPlayers const& waitForPlayers)
{
	for (MyCharacterListener* listen : m_myCharacterListeners)
		listen->onWaitForPlayers(waitForPlayers);
}

void World::notifyCharacterInfo(CharacterInfo const& info)
{
	for (MyCharacterListener* listen : m_myCharacterListeners)
		listen->onCharacterInfo(info);
}

void World::notifyRewardMessage(RewardMessage const& message)
{
	for (MyCharacterListener* listen : m_myCharacterListeners)
		listen->onRewardMessage(message);
}

void World::notifyBattleResult(BattleResult const& result)
{
	for (MyCharacterListener* listen : m_myCharacterListeners)
		listen->onBattleResult(result);
}

void World::notifyItemPickupResult(ItemPickupResult const& result)
{
	for (MyCharacterListener* listen : m_myCharacterListeners)
		listen->onItemPickupResult(result);
}

void World::notifyBattleUpdate(BattleUpdate const& update)
{
	if (m_battleUpdateListener)
		m_battleUpdateListener->onBattleUpdate(update);
}

void World::notifyWorldStatus(WorldStatus const& status)
{
	for (WorldStatusListener* listen : m_worldStatusListeners)
		listen->onWorldStatus(status);
}

void World::notifyTheaterStatusList(TheaterStatusList const& statusList)
{
	for (WorldStatusListener* listen : m_worldStatusListeners)
		listen->onTheaterStatusList(statusList);
}

void World::notifyPlayerStatusList(PlayerStatusList const& statusList)
{
	for (WorldStatusListener* listen : m_worldStatusListeners)
		listen->onPlayerStatusList(statusList);
}

void World::notifyFlashMessage(FlashMessage const& flashMsg)
{
	for (MessageListener* listen : m_messageListeners)
		listen->onFlashMessage(flashMsg);
}

void World::notifyPlayerActionMessage(PlayerActionMessage const& message)
{
	for (MessageListener* listen : m_messageListeners)
		listen->onPlayerActionMessage(message);
}

void World::notifyDeathMessage(DeathMessage const& message)
{
	for (MessageListener* listen : m_messageListeners)
		listen->onDeathMessage(message);
}

void World::notifyItemActionMessage(ItemActionMessage const& message)
{
	for (MessageListener* listen : m_messageListeners)
		listen->onItemActionMessage(message);
}

void World::notifyLaunchResult(LaunchResult const& result)
{
	for (ProjectileListener* listen : m_projectileListeners)
		listen->onLaunchResult(result);
}

void World::notifyItemUseResult(ItemUseResult const& result)
{
	for (CarriedItemListener* listen : m_carriedItemListeners)
		listen->onItemUseResult(result);
}

void World::notifyItemApplicationUpdate(ItemApplicationUpdate const& update)
{
	for (ItemApplicationListener* listen : m_itemApplicationListeners)
		listen->onItemApplicationUpdate(update);
}

void World::notifyItemApplicationUpdateAll(ItemApplicationUpdateAll const& updateAll)
{
	for (ItemApplicationListener* listen : m_itemApplicationListeners)
		listen->onItemApplicationUpdateAll(updateAll);
}

void World::handleMessage(Message const& message)
{
	switch (message.what())
	{
	// Network error
	case SOCKMSG_AUTH_NETWORK_ERROR:
	case SOCKMSG_WORLD_NETWORK_ERROR:
		// Session was not successfully created. World replaced session to send UI notification
		if (!m_currSession)
		{
			NetworkError error;
			message.objectAs(error);

			std::string description = error.getErrorString();
			sAnalytics->logException(message.what() == SOCKMSG_AUTH_NETWORK_ERROR ? "auth" : "world", description.c_str());

			this->notifyNetworkError(error);
		}
		break;
	}
}

void World::removeSession()
{
	if (!m_currSession)
		return;

	if (MyCharacter* myChar = getMyCharacter())
		myChar->setSession(nullptr);

	CC_SAFE_DELETE(m_currSession);

	if (m_isStopping)
	{
		this->removeMySelf();
		m_isStopping = false;
		m_isStopped = true;
		if (m_worldLifecycleListener)
			m_worldLifecycleListener->onWorldStopped(this);
	}
}

void World::removeMySelf()
{
	if (m_map)
	{
		m_map->removeMyChar();
		CC_SAFE_DELETE(m_map);
	}
}

NS_END