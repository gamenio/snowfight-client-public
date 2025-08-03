#ifndef __AUTH_SOCKET_MGR_H__
#define __AUTH_SOCKET_MGR_H__

#include "common/Common.h"
#include "common/network/SocketMgr.h"
#include "AuthSocket.h"


NS_BEGIN

class World;

class AuthSocketMgr: public SocketMgr<AuthSocket, World>
{
public:
	AuthSocketMgr(World* world) :
		SocketMgr<AuthSocket, World>(world)
	{ 
	}

	~AuthSocketMgr() 
	{ 
	}
};

NS_END

#endif // __AUTH_SOCKET_MGR_H__
