#ifndef __WORLD_SOCKET_H__
#define __WORLD_SOCKET_H__

#include "common/Common.h"
#include "common/utils/Timer.h"
#include "common/network/Socket.h"
#include "protocol/WorldOpcode.h"


NS_BEGIN

template<typename T, typename E>class SocketMgr;
class WorldSession;
class World;

typedef BasicPacket<world::NUM_MSG_TYPES> WorldPacket;

//
// 建立与WorldServer的连接， 成功则创建WorldSession。
//
class WorldSocket : public Socket<WorldSocket, WorldPacket>
{
public:
	WorldSocket(SocketMgr<WorldSocket, World>* socketMgr, asio::io_service& ioService);
	~WorldSocket();

	void sendPacket(WorldPacket&& packet);

	void onReadyToConnect() override;
	void onConnected() override;
	void onDisconnected() override;
	void onError(NetworkError&& error) override;
	void onReceivedData(WorldPacket&& packet) override;
	void update() override;

	SocketMgr<WorldSocket, World>* getSocketMgr() const { return m_socketMgr; }

	// Ping
	void setPingTimer(float interval);
	void sendPing();
	void startPing();
	void stopPing();

private:
	void update(float diff);
	// Pong
	void handlePong(WorldPacket& recvPacket);
    
	// 时间同步
	void handleTimeSyncReq(WorldPacket& recvPacket);

	NSTime m_updateTime;
	WorldSession* m_session;
	SocketMgr<WorldSocket, World>* m_socketMgr;

	std::mutex m_pingMutex;
	int32 m_pingCounter;
	NSTime m_pingTime;
	bool m_pingStopped;
	bool m_continuePing;
	IntervalTimer m_pingTimer;
};

NS_END

#endif // __WORLD_SOCKET_H__
