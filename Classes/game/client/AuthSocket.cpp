#include "AuthSocket.h"

#include <google/protobuf/message_lite.h>

#include "common/network/BasicPacket.h"
#include "game/World.h"
#include "AuthSession.h"
#include "AuthSocketMgr.h"


NS_BEGIN

AuthSocket::AuthSocket(SocketMgr<AuthSocket, World>* socketMgr, asio::io_service& ioService):
	Socket(ioService), 
	m_session(nullptr),
	m_socketMgr(socketMgr)
{
}


AuthSocket::~AuthSocket()
{
	m_session = nullptr;
	m_socketMgr = nullptr;
}


void AuthSocket::sendPacket(AuthPacket&& packet)
{
	this->queuePacket(std::move(packet));
}

void AuthSocket::onReadyToConnect()
{
}

void AuthSocket::onConnected()
{
	if (m_session)
		return;

	m_session = new AuthSession(this->shared_from_this());
	m_socketMgr->getService()->setSession(m_session);
	m_socketMgr->getService()->getDispatcher()->post(SOCKMSG_AUTH_CONNECTED);
}

void AuthSocket::onDisconnected()
{
	m_session = nullptr;
	m_socketMgr->getService()->getDispatcher()->post(SOCKMSG_AUTH_DISCONNECTED);
}

void AuthSocket::onError(NetworkError&& error)
{
	m_session = nullptr;
	Message msg(SOCKMSG_AUTH_NETWORK_ERROR, std::move(error));
	m_socketMgr->getService()->getDispatcher()->post(msg);
}

void AuthSocket::onReceivedData(AuthPacket&& packet)
{
	try
	{
		if (m_session)
			m_session->addToRecvQueue(std::move(packet));
	}
	catch (PacketException const&)
	{
		CCLOG("AuthSocket::onReceivedData PacketException occurred while unpacking a packet (opcode: %u)", packet.getOpcode());
	}
}

NS_END