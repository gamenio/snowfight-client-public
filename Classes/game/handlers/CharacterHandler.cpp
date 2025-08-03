#include "game/client/WorldSession.h"

#include "game/client/protocol/pb/PlayerLogin.pb.h"
#include "game/client/protocol/pb/TheaterInfo.pb.h"
#include "game/client/protocol/pb/JoinTheater.pb.h"
#include "game/client/protocol/pb/QueryCharacterInfo.pb.h"
#include "game/client/protocol/pb/CharacterInfo.pb.h"
#include "game/client/protocol/pb/SmileyChat.pb.h"
#include "game/client/protocol/pb/GMCommand.pb.h"
#include "game/client/protocol/pb/RewardMessage.pb.h"
#include "game/client/protocol/pb/WaitForPlayers.pb.h"
#include "game/client/protocol/pb/BattleResult.pb.h"
#include "game/client/protocol/pb/ItemPickupResult.pb.h"

#include "common/utils/Timer.h"
#include "game/behaviors/MyCharacter.h"
#include "game/World.h"
#include "game/entities/updates/UpdateObject.h"


NS_BEGIN

void WorldSession::handleTheaterInfo(WorldPacket& recvPacket)
{
	TheaterInfo info;
	recvPacket.unpack(info);

	m_theaterId = info.theater_id();

	m_world->notifyTheaterInfo(info);
}

void WorldSession::handleWaitForPlayers(WorldPacket& recvPacket)
{
	WaitForPlayers wait;
	recvPacket.unpack(wait);

	m_world->notifyWaitForPlayers(wait);
}

void WorldSession::sendPlayerLogin(PlayerConfig const& config)
{
	PlayerLogin message;
	message.set_screen_width(uint32(config.winSize.width));
	message.set_screen_height(uint32(config.winSize.height));
	message.set_lang(config.lang);
	message.set_country(config.country);
	message.set_char_name(config.charName);
	message.set_char_id(config.charId);
	message.set_total_kills(config.totalKills);
	message.set_property(config.property);
	message.set_level(config.level);
	message.set_experience(config.experience);
	int32 nStatTypes = (int32)config.statStageList.size();
	for (int32 i = 0; i < nStatTypes; ++i)
	{
		if (config.statStageList[i] > 0)
		{
			PlayerLogin::StatStage* ss = message.add_stat_stage_list();
			ss->set_stage(config.statStageList[i]);
			ss->set_type(i);
		}
	}
	message.set_controller_type(config.controllerType);
	message.set_trainee(config.isTrainee);
	message.set_reward_stage(config.rewardStage);
	message.set_daily_reward_days(config.dailyRewardDays);
	
	WorldPacket packet(world::CMSG_PLAYER_LOGIN, std::move(message));

	this->sendPacket(std::move(packet));
	m_isPlayerLoggedIn = true;
}

void WorldSession::sendJoinTheater()
{
	JoinTheater join;
	WorldPacket packet(world::CMSG_JOIN_THEATER, std::move(join));

	this->sendPacket(std::move(packet));
}

void WorldSession::sendQueryCharacterInfo(Unit* unit)
{
	QueryCharacterInfo query;
	query.set_guid(unit->getData()->getGuid().getRawValue());
	WorldPacket packet(world::CMSG_QUERY_CHARACTER_INFO, std::move(query));

	this->sendPacket(std::move(packet));
}

void WorldSession::handleCharacterInfo(WorldPacket& recvPacket)
{
	CharacterInfo info;
	recvPacket.unpack(info);

	WorldObject* obj = m_world->getMap()->findObject(ObjectGuid(info.guid()));
	if (!obj)
		return;

	DataUnit* dataUnit = obj->asUnit()->getData();
	dataUnit->setName(info.name());

	if (obj->asMyCharacter())
		m_world->notifyCharacterInfo(info);
}

void WorldSession::handleTransport(WorldPacket& recvPacket)
{
	MyCharacter* myChar = this->getMyCharacter();
	if (!myChar)
		return;

	MovementInfo movement;
	recvPacket.unpack(movement);
	NS_ASSERT_LOG(myChar->getData()->getGuid() == movement.guid, "MovementInfo does not belong to my character.");

	myChar->stopMoving();
	myChar->resetMoveSyncState();
	myChar->combatStop();
	myChar->unlockFacingDirection();
	myChar->stopHandDownTimer();
	myChar->getData()->setMovementInfo(movement);
}

void WorldSession::handleRewardMessage(WorldPacket& recvPacket)
{
	RewardMessage message;
	recvPacket.unpack(message);

	m_world->notifyRewardMessage(message);
}

void WorldSession::handleBattleResult(WorldPacket& recvPacket)
{
	BattleResult result;
	recvPacket.unpack(result);

	m_world->notifyBattleResult(result);
}

void WorldSession::handleModifyStamina(WorldPacket& recvPacket)
{
	MyCharacter* myChar = this->getMyCharacter();
	if (!myChar)
		return;

	StaminaInfo stamina;
	recvPacket.unpack(stamina);
	NS_ASSERT_LOG(myChar->getData()->getGuid() == stamina.guid, "StaminaInfo does not belong to my character.");

	myChar->getData()->setStaminaInfo(stamina);

	myChar->resetStaminaSyncState();
	if (myChar->isInCharge())
		myChar->getStaminaUpdater()->chargeUpdate(stamina);
	else
		myChar->startRegenStamina();
}

void WorldSession::handleItemPickupResult(WorldPacket& recvPacket)
{
	ItemPickupResult result;
	recvPacket.unpack(result);

	m_world->notifyItemPickupResult(result);
}

void WorldSession::sendSmiley(uint16 code)
{
    SmileyChat smileyChat;
    smileyChat.set_code(code);
    WorldPacket packet(world::CMSG_SMILEY_CHAT, std::move(smileyChat));
    
    this->sendPacket(std::move(packet));
}

NS_END
