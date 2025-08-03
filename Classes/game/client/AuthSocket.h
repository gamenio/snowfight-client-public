#ifndef __AUTH_SOCKET_H__
#define __AUTH_SOCKET_H__

#include <google/protobuf/message_lite.h>

#include "common/Common.h"
#include "common/network/Socket.h"
#include "protocol/AuthOpcode.h"

NS_BEGIN

template<typename T, typename E>class SocketMgr;
class AuthSession;
class World;

typedef BasicPacket<auth::NUM_MSG_TYPES> AuthPacket;

//
// 创建AuthSocket，建立与AuthServer的连接
// Socket握手成功后创建AuthSession并关联到MultWorld，之后发起验证请求
//
class AuthSocket: public Socket<AuthSocket, AuthPacket>
{
public:
	AuthSocket(SocketMgr<AuthSocket, World>* socketMgr, asio::io_service& ioService);
	~AuthSocket();

	void sendPacket(AuthPacket&& packet);

	void onReadyToConnect() override;
	void onConnected() override;
	void onDisconnected() override;
	void onError(NetworkError&& error) override;
	void onReceivedData(AuthPacket&& packet) override;

	SocketMgr<AuthSocket, World>* getSocketMgr() const { return m_socketMgr; }

private:
	AuthSession* m_session;
	SocketMgr<AuthSocket, World>* m_socketMgr;
};

NS_END

#endif // __AUTH_SOCKET_H__

