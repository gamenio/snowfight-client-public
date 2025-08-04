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
// Create AuthSocket to establish a connection with AuthServer.
// After the socket handshake is successful, create an AuthSession and associate it with World, 
// then initiate an authorization request.
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

