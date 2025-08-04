#include "NTSSocket.h"

#include "protocol/pb/TimeQuery.pb.h"
#include "protocol/pb/TimeResult.pb.h"

#include "common/network/BasicPacket.h"
#include "MessageEnum.h"
#include "TimeService.h"
#include "NTSSocketMgr.h"


NS_BEGIN

#define RECEIVE_TIMEOUT				10000		// Receive timeout time. Unit: milliseconds

NTSSocket::NTSSocket(SocketMgr<NTSSocket, TimeService>* socketMgr, asio::io_service& ioService):
	Socket(ioService), 
	m_socketMgr(socketMgr),
	m_receiveTimeoutTimer(ioService)
{
}


NTSSocket::~NTSSocket()
{
	m_socketMgr = nullptr;
}

void NTSSocket::onReadyToConnect()
{
}

void NTSSocket::onConnected()
{
	m_socketMgr->getService()->getDispatcher()->post(nts::SOCKMSG_CONNECTED);
	this->sendTimeQuery();
}

void NTSSocket::onDisconnected()
{
	asio::error_code ignored;
	m_receiveTimeoutTimer.cancel(ignored);
	m_socketMgr->getService()->getDispatcher()->post(nts::SOCKMSG_DISCONNECTED);
}

void NTSSocket::onError(NetworkError&& error)
{
	asio::error_code ignored;
	m_receiveTimeoutTimer.cancel(ignored);

	Message msg(nts::SOCKMSG_NETWORK_ERROR, std::move(error));
	m_socketMgr->getService()->getDispatcher()->post(msg);
}

void NTSSocket::onReceivedData(NTSPacket&& packet)
{
	asio::error_code ignored;
	m_receiveTimeoutTimer.cancel(ignored);
	try
	{
		switch (packet.getOpcode())
		{
		case nts::SMSG_TIME_RESULT:
			this->handleTimeResult(packet);
			break;
		default:
			break;
		}
	}
	catch (PacketException const&)
	{
		CCLOG("NTSSocket::onReceivedData PacketException occurred while unpacking a packet (opcode: %u)", packet.getOpcode());
	}
}

void NTSSocket::sendPacket(NTSPacket&& packet)
{
	this->queuePacket(std::move(packet));
	this->checkReceiveTimeout();
}

void NTSSocket::sendTimeQuery()
{
	int64 currTime = time_util::getSystemTimeMillis();
	TimeQuery query;
	query.set_transmit_timestamp(currTime);

	NTSPacket packet(nts::CMSG_TIME_QUERY, std::move(query));
	this->sendPacket(std::move(packet));
}

void NTSSocket::handleTimeResult(NTSPacket& packet)
{
	TimeResult result;
	packet.unpack(result);

	// The time the client received the response (t4)
	int64 destTime = time_util::getSystemTimeMillis();

	// The time the client sent the request (t1)
	int64 origTime = result.originate_timestamp();
	// The time the server received the request (t2)
	int64 rcvTime = result.receive_timestamp();
	// The time the server sent the response (t3)
	int64 xmitTime = result.transmit_timestamp();

	// Round-trip delay. delay=(t4-t1)-(t3-t2)
	int32 delay = (int32)((destTime - origTime) - (xmitTime - rcvTime));

	// Local clock offset. offset=((t2-t1)+(t3-t4))/2
	int64 offset = ((rcvTime - origTime) + (xmitTime - destTime)) / 2;

	TimeInfo info;
	info.delay = delay;
	info.offset = offset;
	Message msg(nts::SOCKMSG_TIME_INFO, std::move(info));
	m_socketMgr->getService()->getDispatcher()->post(msg);
}

void NTSSocket::checkReceiveTimeout()
{
	asio::error_code ec;
	m_receiveTimeoutTimer.expires_from_now(std::chrono::milliseconds(RECEIVE_TIMEOUT), ec);
	if (!ec)
	{
		auto self(this->shared_from_this());
		m_receiveTimeoutTimer.async_wait([this, self](asio::error_code const& error) {
			if (!error)
			{
				this->closeSocket();

				NetworkError error(NetworkError::RECV_FAILED, "Receiving data timed out.");
				Message msg(nts::SOCKMSG_NETWORK_ERROR, std::move(error));
				m_socketMgr->getService()->getDispatcher()->post(msg);
			}
		});
	}
}

NS_END