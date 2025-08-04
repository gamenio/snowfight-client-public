#include "WorldSocket.h"

#include "protocol/pb/Pong.pb.h"
#include "protocol/pb/Ping.pb.h"
#include "protocol/pb/TimeSyncReq.pb.h"
#include "protocol/pb/TimeSyncResp.pb.h"

#include "game/client/protocol/WorldOpcode.h"
#include "game/MessageEnum.h"
#include "game/World.h"
#include "WorldSession.h"
#include "WorldSocketMgr.h"


NS_BEGIN


WorldSocket::WorldSocket(SocketMgr<WorldSocket, World>* socketMgr, asio::io_service& ioService) :
	Socket(ioService),
	m_updateTime(0),
	m_session(nullptr),
	m_socketMgr(socketMgr),
	m_pingCounter(0),
	m_pingTime(0),
	m_pingStopped(true),
	m_continuePing(false)
{
}


WorldSocket::~WorldSocket()
{
	m_session = nullptr;
	m_socketMgr = nullptr;
}


void WorldSocket::sendPacket(WorldPacket&& packet)
{
	this->queuePacket(std::move(packet));
}

void WorldSocket::onReadyToConnect()
{

}

void WorldSocket::onConnected()
{
	if (m_session)
		return;
	
	m_session = dynamic_cast<WorldSession*>(m_socketMgr->getService()->getSession());
	if (m_session)
		m_session->setNewSocket(this->shared_from_this());
	else
	{
		m_session = new WorldSession(this->shared_from_this());
		m_socketMgr->getService()->setSession(m_session);
	}
	m_socketMgr->getService()->getDispatcher()->post(SOCKMSG_WORLD_CONNECTED);
}

void WorldSocket::onDisconnected()
{
	m_session = nullptr;
	m_socketMgr->getService()->getDispatcher()->post(SOCKMSG_WORLD_DISCONNECTED);
}

void WorldSocket::onError(NetworkError&& error)
{
	m_session = nullptr;
	Message msg(SOCKMSG_WORLD_NETWORK_ERROR, std::move(error));
	m_socketMgr->getService()->getDispatcher()->post(msg);
}

void WorldSocket::onReceivedData(WorldPacket&& packet)
{
	try
	{
		switch (packet.getOpcode())
		{
		case world::MSG_PONG:
			this->handlePong(packet);
			break;
		case world::SMSG_TIME_SYNC_REQ:
			this->handleTimeSyncReq(packet);
			break;
		default:
			if (m_session)
			{
				if (!m_session->addToRecvQueue(std::move(packet)))
				{
					this->closeSocket();
					m_socketMgr->getService()->getDispatcher()->post(SOCKMSG_WORLD_SESSION_RECV_QUEUE_ERROR);
				}
			}

			break;
		}
	}
	catch (PacketException const&)
	{
		CCLOG("WorldSocket::onReceivedData PacketException occurred while unpacking a packet (opcode: %u)", packet.getOpcode());
	}
}

void WorldSocket::update()
{
	Socket<WorldSocket, WorldPacket>::update();

	if (!this->isOpen())
		return;

	NSTime nowTime = time_util::getUptimeMillis();
	if (m_updateTime <= 0)
		m_updateTime = nowTime;
	float diff = (nowTime - m_updateTime) / 1000.0f;
	m_updateTime = nowTime;
	this->update(diff);
}

void WorldSocket::setPingTimer(float interval)
{
	std::lock_guard<std::mutex> lock(m_pingMutex);
	m_pingTimer.setInterval(interval);
}

void WorldSocket::sendPing()
{
	if (!m_session)
		return;

	Ping ping;

	std::unique_lock<std::mutex> lock(m_pingMutex);
	if(m_pingStopped)
		return;
	ping.set_counter(++m_pingCounter);
	ping.set_latency(m_session->getLatency());

	m_pingTime = time_util::getUptimeMillis();
	m_continuePing = false;

	lock.unlock();

	WorldPacket packet(world::MSG_PING, std::move(ping));
	this->sendPacket(std::move(packet));
}

void WorldSocket::startPing()
{
	std::lock_guard<std::mutex> lock(m_pingMutex);

	if (!m_pingStopped)
		return;
    
    NS_ASSERT_LOG(m_pingTimer.getInterval() > 0, "No interval time set for Ping timer.");

	m_pingTimer.setPassed();
	m_pingCounter = 0;
	m_pingTime = 0;
	m_continuePing = true;
	m_pingStopped = false;
}
void WorldSocket::stopPing()
{
	std::lock_guard<std::mutex> lock(m_pingMutex);

	if(m_pingStopped)
		return;

	m_pingTimer.reset();
	m_pingCounter = 0;
	m_pingTime = 0;
	m_continuePing = false;
	m_pingStopped = true;
}

void WorldSocket::update(float diff)
{
	bool pinging = false;
	std::unique_lock<std::mutex> lock(m_pingMutex);
	if(!m_pingStopped)
	{
		m_pingTimer.update(diff);
		if (m_pingTimer.passed() && m_continuePing)
		{
			pinging = true;
			m_pingTimer.reset();
		}
	}
	lock.unlock();

	if(pinging)
		this->sendPing();
}

void WorldSocket::handlePong(WorldPacket& recvPacket)
{
	if (!m_session)
		return;

	Pong pong;
	recvPacket.unpack(pong);

	std::lock_guard<std::mutex> lock(m_pingMutex);

	if (m_pingCounter != pong.counter())
	{
		CCLOG("Wrong ping counter");
		return;
	}

	NSTime nowTime = time_util::getUptimeMillis();
	NSTime latency = nowTime - m_pingTime;
	m_session->setLatency(latency);
	m_session->resetTimeoutTimer();

	m_continuePing = true;

	//CCLOG("UPDATE LATENCY latency: %dms counter: %d", latency, pong.counter());
}

void WorldSocket::handleTimeSyncReq(WorldPacket& recvPacket)
{
    if(!m_session)
        return;
    
	TimeSyncReq req;
	recvPacket.unpack(req);

	NSTime currTime = time_util::getUptimeMillis();
	//CCLOG("TIME SYNC clienttime: %dms counter: %d", currTime, req.counter());

	// Synchronize the client's current time
	TimeSyncResp resp;
	resp.set_counter(req.counter());
	resp.set_time(currTime);
	recvPacket.setMessage(world::CMSG_TIME_SYNC_RESP, std::move(resp));

	this->sendPacket(std::move(recvPacket));
}

NS_END
