#include "WorldSession.h"

#include "protocol/pb/Ping.pb.h"

#include "protocol/WorldOpcodeHandler.h"
#include "game/World.h"
#include "WorldSocketMgr.h"



NS_BEGIN

#define RECV_QUEUE_LIMIT			2000 // Limit on the number of WorldPacket in the receive queue

#define PING_INTERVAL				1.0f // Ping interval time. Unit: seconds
// The session timeout should always be greater than the ping interval time, otherwise it will trigger a session timeout.
#define SESSION_TIMEOUT				10.0f // Session timeout. Unit: seconds

WorldSession::WorldSession(std::shared_ptr<WorldSocket> const& socket) :
	m_socket(socket),
	m_sessionId(0),
	m_authProof(""),
	m_playerId(""),
	m_originalPlayerId(""),
	m_requiredCapabilities(0),
	m_theaterId(0),
	m_world(socket->getSocketMgr()->getService()),
	m_socketMgr(socket->getSocketMgr()),
	m_isPlayerLoggedIn(false),
	m_myCharacter(nullptr),
	m_gmLevel(0),
	m_isClosing(false),
	m_isClosed(false),
	m_latency(0),
	m_isTimeoutEnabled(false)
{
	m_type = SESSION_TYPE_WORLD;

	m_world->getDispatcher()->registerHandler(SOCKMSG_WORLD_CONNECTED, this, &WorldSession::handleMessage);
	m_world->getDispatcher()->registerHandler(SOCKMSG_WORLD_DISCONNECTED, this, &WorldSession::handleMessage);
	m_world->getDispatcher()->registerHandler(SOCKMSG_WORLD_NETWORK_ERROR, this, &WorldSession::handleMessage);
	m_world->getDispatcher()->registerHandler(SOCKMSG_WORLD_SESSION_RECV_QUEUE_ERROR, this, &WorldSession::handleMessage);

	m_socket->setPingTimer(PING_INTERVAL);
	m_timeoutTimer.setDuration(SESSION_TIMEOUT);
}

WorldSession::~WorldSession()
{
	this->close();

	m_world->getDispatcher()->removeHandlersWithTarget(this);

	m_world = nullptr;
	m_socketMgr = nullptr;
}

void WorldSession::setNewSocket(std::shared_ptr<WorldSocket> const& socket)
{
	m_newSocket = socket;
}

void WorldSession::logoutPlayer()
{
	if (m_isClosed || !m_isPlayerLoggedIn)
		return;
    
	m_isPlayerLoggedIn = false;
    m_myCharacter = nullptr;

	WorldPacket packet(world::CMSG_PLAYER_LOGOUT);
	this->sendPacket(std::move(packet));
}

bool WorldSession::addToRecvQueue(WorldPacket&& packet)
{
	if (m_recvQueue.size() < RECV_QUEUE_LIMIT)
	{
		m_recvQueue.add(std::move(packet));
		return true;
	}

	return false;
}

void WorldSession::sendPacket(WorldPacket&& packet)
{
	if (m_isClosed)
		return;

	if(m_socket)
		m_socket->sendPacket(std::move(packet));
}


void WorldSession::closeDelayed()
{
	if (m_isClosed)
		return;

	m_isClosing = true;
}


void WorldSession::close()
{
	if (m_isClosed)
		return;
    
    this->logoutPlayer();

	m_socket = nullptr;
	m_socketMgr->stop();

    m_recvQueue.clear();
	m_isClosed = true;
	m_isClosing = false;
}

void WorldSession::handleMessage(Message const& message)
{
	switch (message.what())
	{
	case SOCKMSG_WORLD_CONNECTED:
	{
		if (this->applyNewSocket())
			m_world->notifyNetworkRestored();
		break;
	}
	case SOCKMSG_WORLD_DISCONNECTED:
		break;
	case SOCKMSG_WORLD_SESSION_RECV_QUEUE_ERROR:
		m_socket = nullptr;
		m_socketMgr->stop();
		m_world->notifyWorldRecvQueueFulL();
		break;
	case SOCKMSG_WORLD_NETWORK_ERROR:
	{
		m_socket = nullptr;
		m_socketMgr->stop();

		NetworkError error;
		message.objectAs(error);

		m_world->notifyNetworkError(error);
		break;
	}
	default:
		break;
	}
}

bool WorldSession::applyNewSocket()
{
	if (!m_newSocket)
		return false;

	m_socket = m_newSocket;
    m_socket->setPingTimer(PING_INTERVAL);

	m_latency = 0;
	m_recvQueue.clear();

	this->resetTimeoutTimer();

	m_newSocket = nullptr;

	return true;
}

void WorldSession::checkTimeout(float delta)
{
	if (!m_isTimeoutEnabled || !m_socket)
		return;

	std::unique_lock<std::mutex> lock(m_timeoutTimerMutex);
	m_timeoutTimer.update(delta);
	bool isTimedOut = m_timeoutTimer.passed();
	lock.unlock();

	if (isTimedOut)
	{
		m_socket = nullptr;
		m_socketMgr->stop();

		m_world->notifyWorldSessionTimedout();
	}

}

bool WorldSession::update(float delta)
{
	this->checkTimeout(delta);

	WorldPacket packet;
	try
	{
		while (m_recvQueue.next(packet))
		{
			if (world::gOpcodeHandlerTable.find(packet.getOpcode()) != world::gOpcodeHandlerTable.end())
			{
				world::OpcodeHandler& opHandler = world::gOpcodeHandlerTable[packet.getOpcode()];
				(this->*opHandler.handler)(packet);
			}
			else
				CCLOG("Received unhandled opcode %s", world::getOpcodeNameForLogging(packet.getOpcode()).c_str());
		}
	}
	catch (PacketException const& ex)
	{
		m_isClosing = true;

		NetworkError error(NetworkError::DECODE_PACKET_FAILED, ex.what(), packet.getOpcode());
		m_world->notifyNetworkError(error);

	}

	if (m_isClosing || m_isClosed)
	{
		this->close();
		return false;
	}

	return true;

}

void WorldSession::resetTimeoutTimer()
{
	std::lock_guard<std::mutex> lock(m_timeoutTimerMutex);
	m_timeoutTimer.reset();
}

void WorldSession::startPing()
{
	if (m_isClosed)
		return;

	if(m_socket)
		m_socket->startPing();
}

void WorldSession::stopPing()
{
	if(m_socket)
		m_socket->stopPing();
}


NS_END