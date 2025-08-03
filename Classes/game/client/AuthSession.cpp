#include "AuthSession.h"

#include "game/client/protocol/AuthOpcodeHandler.h"
#include "game/World.h"
#include "AuthSocketMgr.h"

NS_BEGIN

AuthSession::AuthSession(std::shared_ptr<AuthSocket> const& socket):
	m_socket(socket),
	m_isClosing(false),
	m_isClosed(false),
	m_world(socket->getSocketMgr()->getService()),
	m_socketMgr(socket->getSocketMgr())
{
	m_type = SESSION_TYPE_AUTH;

	m_world->getDispatcher()->registerHandler(SOCKMSG_AUTH_CONNECTED, this, &AuthSession::handleMessage);
	m_world->getDispatcher()->registerHandler(SOCKMSG_AUTH_DISCONNECTED, this, &AuthSession::handleMessage);
	m_world->getDispatcher()->registerHandler(SOCKMSG_AUTH_NETWORK_ERROR, this, &AuthSession::handleMessage);

}


AuthSession::~AuthSession()
{
	this->close();

	m_world->getDispatcher()->removeHandlersWithTarget(this);
	m_world = nullptr;
	m_socketMgr = nullptr;
}

void AuthSession::addToRecvQueue(AuthPacket&& packet)
{
	m_recvQueue.add(std::move(packet));
}

void AuthSession::sendPacket(AuthPacket&& packet)
{
	if (m_isClosed)
		return;

	m_socket->sendPacket(std::move(packet));
}


void AuthSession::closeDelayed()
{
	if (m_isClosed)
		return;

	m_isClosing = true;
}

void AuthSession::close()
{
	if (m_isClosed)
		return;

	m_socket = nullptr;
	m_socketMgr->stop();

    m_recvQueue.clear();
	m_isClosed = true;
    m_isClosing = false;
}


bool AuthSession::update(float delta)
{
	AuthPacket packet;
	try
	{
		while (m_recvQueue.next(packet))
		{
			if (auth::gOpcodeHandlerTable.find(packet.getOpcode()) != auth::gOpcodeHandlerTable.end())
			{
				auth::OpcodeHandler& opHandler = auth::gOpcodeHandlerTable[packet.getOpcode()];
				(this->*opHandler.handler)(packet);
			}
			else
				CCLOG("Received unhandled opcode %s", auth::getOpcodeNameForLogging(packet.getOpcode()).c_str());
		}
	}
	catch (PacketException const& ex)
	{
		m_isClosing = true;

		NetworkError error(NetworkError::DECODE_PACKET_FAILED, ex.what(), packet.getOpcode());
		m_world->notifyNetworkError(error);
	}

	if (m_isClosing || !m_socket || !m_socket->isOpen())
	{
		this->close();

		return false;
	}

	return true;
}


void AuthSession::handleMessage(Message const& message)
{
	switch (message.what())
	{
	case SOCKMSG_AUTH_CONNECTED:
		break;
	case SOCKMSG_AUTH_DISCONNECTED:
		break;
	case SOCKMSG_AUTH_NETWORK_ERROR:
	{
		NetworkError error;
		message.objectAs(error);

		m_world->notifyNetworkError(error);
		break;
	}
	default:
		break;
	}
}

NS_END
