#include "game/client/WorldSession.h"


#include "game/client/protocol/pb/FlashMessage.pb.h"
#include "game/client/protocol/pb/PlayerActionMessage.pb.h"
#include "game/client/protocol/pb/DeathMessage.pb.h"
#include "game/client/protocol/pb/ItemActionMessage.pb.h"

#include "game/World.h"

NS_BEGIN

void WorldSession::handleFlashMessage(WorldPacket& recvPacket)
{
	FlashMessage flashMsg;
	recvPacket.unpack(flashMsg);

	m_world->notifyFlashMessage(flashMsg);
}

void WorldSession::handlePlayerActionMessage(WorldPacket& recvPacket)
{
	PlayerActionMessage message;
	recvPacket.unpack(message);

	m_world->notifyPlayerActionMessage(message);
}

void WorldSession::handleDeathMessage(WorldPacket& recvPacket)
{
	DeathMessage message;
	recvPacket.unpack(message);

	m_world->notifyDeathMessage(message);
}

void WorldSession::handleItemActionMessage(WorldPacket& recvPacket)
{
	ItemActionMessage message;
	recvPacket.unpack(message);

	m_world->notifyItemActionMessage(message);
}

NS_END



