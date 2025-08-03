#include "game/client/WorldSession.h"

#include "game/client/protocol/pb/QueryWorldStatus.pb.h"
#include "game/client/protocol/pb/QueryTheaterStatusList.pb.h"
#include "game/client/protocol/pb/WorldStatus.pb.h"
#include "game/client/protocol/pb/TheaterStatusList.pb.h"
#include "game/client/protocol/pb/QueryPlayerStatusList.pb.h"
#include "game/client/protocol/pb/PlayerStatusList.pb.h"

#include "game/World.h"

NS_BEGIN

void WorldSession::sendQueryWorldStatus()
{
	QueryWorldStatus message;
	WorldPacket packet(world::CMSG_QUERY_WORLD_STATUS, std::move(message));

	this->sendPacket(std::move(packet));

}

void WorldSession::handleWorldStatus(WorldPacket& recvPacket)
{
	WorldStatus status;
	recvPacket.unpack(status);

	m_world->notifyWorldStatus(status);
}


void WorldSession::sendQueryTheaterStatusList()
{
	QueryTheaterStatusList message;
	WorldPacket packet(world::CMSG_QUERY_THEATER_STATUS_LIST, std::move(message));

	this->sendPacket(std::move(packet));
}

void WorldSession::handleTheaterStatusList(WorldPacket& recvPacket)
{
	TheaterStatusList statusList;
	recvPacket.unpack(statusList);

	m_world->notifyTheaterStatusList(statusList);
}


void WorldSession::sendQueryPlayerStatusList(uint32 theaterId)
{
	QueryPlayerStatusList message;
	message.set_theater_id(theaterId);

	WorldPacket packet(world::CMSG_QUERY_PLAYER_STATUS_LIST, std::move(message));
	this->sendPacket(std::move(packet));
}

void WorldSession::handlePlayerStatusList(WorldPacket& recvPacket)
{
	PlayerStatusList statusList;
	recvPacket.unpack(statusList);

	m_world->notifyPlayerStatusList(statusList);
}

NS_END