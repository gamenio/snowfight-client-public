#ifndef __WORLD_LISTENERS_H__
#define __WORLD_LISTENERS_H__

#include "client/protocol/pb/RealmList.pb.h"
#include "client/protocol/pb/TheaterInfo.pb.h"
#include "client/protocol/pb/WaitForPlayers.pb.h"
#include "client/protocol/pb/CharacterInfo.pb.h"
#include "client/protocol/pb/DeathMessage.pb.h"
#include "client/protocol/pb/RewardMessage.pb.h"
#include "client/protocol/pb/BattleResult.pb.h"
#include "client/protocol/pb/LogonResult.pb.h"
#include "client/protocol/pb/BattleUpdate.pb.h"
#include "client/protocol/pb/WorldStatus.pb.h"
#include "client/protocol/pb/TheaterStatusList.pb.h"
#include "client/protocol/pb/PlayerStatusList.pb.h"
#include "client/protocol/pb/FlashMessage.pb.h"
#include "client/protocol/pb/PlayerActionMessage.pb.h"
#include "client/protocol/pb/ItemUseResult.pb.h"
#include "client/protocol/pb/LaunchResult.pb.h"
#include "client/protocol/pb/ItemActionMessage.pb.h"
#include "client/protocol/pb/ItemPickupResult.pb.h"
#include "client/protocol/pb/ItemApplicationUpdate.pb.h"

#include "common/Common.h"
#include "common/network/NetworkError.h"
#include "game/entities/ObjectGuid.h"
#include "game/entities/DataBasic.h"
#include "game/entities/DataCarriedItem.h"
#include "game/entities/DataProjectile.h"


NS_BEGIN

class World;
class AuthSession;
class WorldSession;

class WorldLifecycleListener
{
public:
	virtual void onWorldStarted(World* world) {}
	virtual void onWorldStopped(World* world) {}
};

class LogonListener
{
public:
	virtual void onAuthSessionOpened(AuthSession* session) {}
	virtual void onLogonSucceeded() {}
	virtual void onLogonFailed(LogonResult::ErrorCode errorCode) {}
	virtual void onFetchRealmListSucceeded(Realm const& preferredRealm) {}
	virtual void onFetchRealmlistFailed() {}
};

class WorldAuthListener
{
public:
	virtual void onWorldSessionOpened(WorldSession* session) {}
	virtual void onWorldSessionTimedout() {}
	virtual void onWorldAuthSucceeded() {}
	virtual void onWorldWaitQueue(int32 waitPos) {}
	virtual void onWorldSessionExpired() {}
	virtual void onWorldRecvQueueFulL() {}
	virtual void onWorldAuthFailed() {}
};

class NetworkStatusListener
{
public:
	virtual void onNetworkError(NetworkError const& error) {}
	virtual void onNetworkRestored() {}
};

class WorldInitListener
{
public:
	virtual void onInitSelfCompleted(DataPlayer* myChar) {}
};

class MyCharacterListener
{
public:
	virtual void onTheaterInfo(TheaterInfo const& info) {}
	virtual void onWaitForPlayers(WaitForPlayers const& waitForPlayers) {}
	virtual void onCharacterInfo(CharacterInfo const& info) {}
	virtual void onRewardMessage(RewardMessage const& message) {}
	virtual void onBattleResult(BattleResult const& result) {}
	virtual void onItemPickupResult(ItemPickupResult const& result) {}
};

class BattleUpdateListener
{
public:
	virtual void onBattleUpdate(BattleUpdate const& update) {}
};

class MessageListener
{
public:
	virtual void onFlashMessage(FlashMessage const& flashMsg) {}
	virtual void onPlayerActionMessage(PlayerActionMessage const& message) {}
	virtual void onDeathMessage(DeathMessage const& message) {}
	virtual void onItemActionMessage(ItemActionMessage const& message) {}
};

class WorldStatusListener
{
public:
	virtual void onWorldStatus(WorldStatus const& status) {}
	virtual void onTheaterStatusList(TheaterStatusList const& statusList) {}
	virtual void onPlayerStatusList(PlayerStatusList const& statusList) {}
};

class ObjectLifecycleListener
{
public:
	virtual void onObjectDestroyed(ObjectGuid const& guid) {}
	virtual void onObjectInactivated(ObjectGuid const& guid) {}
	virtual void onObjectActivated(DataBasic* data) {}
};

class LocatorObjectLifecycleListener
{
public:
	virtual void onLocatorObjectDestroyed(ObjectGuid const& guid) {}
	virtual void onLocatorObjectInactivated(ObjectGuid const& guid) {}
	virtual void onLocatorObjectActivated(DataLocatorObject* data) {}
};

class InventoryItemLifecycleListener
{
public:
	virtual void onInventoryItemDestroyed(ObjectGuid const& guid) {}
	virtual void onInventoryItemInactivated(ObjectGuid const& guid) {}
	virtual void onInventoryItemActivated(DataCarriedItem* data) {}
};

class EquipmentItemLifecycleListener
{
public:
	virtual void onEquipmentItemDestroyed(ObjectGuid const& guid) {}
	virtual void onEquipmentItemInactivated(ObjectGuid const& guid) {}
	virtual void onEquipmentItemActivated(DataCarriedItem* data) {}
};

class CarriedItemListener
{
public:
	virtual void onItemUseResult(ItemUseResult const& result) {}
};

class ItemApplicationListener
{
public:
	virtual void onItemApplicationUpdate(ItemApplicationUpdate const& update) {}
	virtual void onItemApplicationUpdateAll(ItemApplicationUpdateAll const& updateAll) {}
};

class ProjectileListener
{
public:
	virtual void onLaunchResult(LaunchResult const& result) {}
};

NS_END


#endif // __WORLD_LISTENERS_H__