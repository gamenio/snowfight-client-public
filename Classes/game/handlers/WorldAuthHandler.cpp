#include "game/client/WorldSession.h"

#include "game/client/protocol/pb/AuthChallenge.pb.h"
#include "game/client/protocol/pb/AuthVerdict.pb.h"
#include "game/client/protocol/pb/AuthProof.pb.h"

#include "game/World.h"

NS_BEGIN

void WorldSession::handleAuthChallenge(WorldPacket& recvPacket)
{
	AuthChallenge message;
	recvPacket.unpack(message);
	CCLOG("handleAuthChallenge: %d", message.auth_seed());

}

void WorldSession::handleAuthVerdict(WorldPacket& recvPacket)
{
	AuthVerdict message;
	recvPacket.unpack(message);
	switch (message.result())
	{
	case AuthVerdict::AUTH_OK:
		this->setSessionId(message.session_id());
		this->setGMLevel((uint8)message.gm_level());
		this->startPing();

		m_world->notifyWorldAuthSucceeded();
		break;
	case AuthVerdict::AUTH_WAIT_QUEUE:
	{
		this->startPing();
		m_world->notifyWorldWaitQueue(message.wait_pos());
		break;
	}
	case AuthVerdict::AUTH_SESSION_EXPIRED:
		this->close();
		m_world->notifyWorldSessionExpired();
		break;
	default: // AuthVerdict::AUTH_FAILED
		this->close();
		m_world->notifyWorldAuthFailed();
		break;
	}
}

void WorldSession::sendAuthProof(std::string const& proof, std::string const& playerId, std::string const& originalPlayerId, uint32 requiredCapabilities)
{
	m_authProof = proof;
	m_playerId = playerId;
	m_originalPlayerId = originalPlayerId;
	m_requiredCapabilities = requiredCapabilities;

	AuthProof message;
	message.set_proof(proof);
	message.set_session_id(this->getSessionId());
	message.set_required_capabilities(requiredCapabilities);
	message.set_playerid(playerId);
	message.set_original_playerid(originalPlayerId);

	WorldPacket packet(world::CMSG_AUTH_PROOF, std::move(message));
	this->sendPacket(std::move(packet));
}

void WorldSession::resendAuthProof()
{
	NS_ASSERT(!m_authProof.empty());
	NS_ASSERT(!m_playerId.empty());
	this->sendAuthProof(m_authProof, m_playerId, m_originalPlayerId, m_requiredCapabilities);
}



NS_END