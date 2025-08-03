#ifndef __NTS_SOCKET_H__
#define __NTS_SOCKET_H__

#include <google/protobuf/message_lite.h>

#include "common/Common.h"
#include "common/network/Socket.h"
#include "protocol/NTSOpcode.h"

NS_BEGIN

struct TimeInfo
{
	TimeInfo() :
		offset(0),
		delay(0)
	{
	}

	int64 offset;
	int32 delay;
};

class TimeService;
template<typename T, typename E>class SocketMgr;

typedef BasicPacket<nts::NUM_MSG_TYPES> NTSPacket;

//
// 创建NTSSocket，建立与NTSServer的连接
//
class NTSSocket: public Socket<NTSSocket, NTSPacket>
{
public:
	NTSSocket(SocketMgr<NTSSocket, TimeService>* socketMgr, asio::io_service& ioService);
	~NTSSocket();

	void onReadyToConnect() override;
	void onConnected() override;
	void onDisconnected() override;
	void onError(NetworkError&& error) override;
	void onReceivedData(NTSPacket&& packet) override;

	SocketMgr<NTSSocket, TimeService>* getSocketMgr() const { return m_socketMgr; }

private:
	void sendPacket(NTSPacket&& packet);

	void sendTimeQuery();
	void handleTimeResult(NTSPacket& packet);

	void checkReceiveTimeout();

	SocketMgr<NTSSocket, TimeService>* m_socketMgr;
	asio::steady_timer m_receiveTimeoutTimer;
};

NS_END

#endif // __NTS_SOCKET_H__

