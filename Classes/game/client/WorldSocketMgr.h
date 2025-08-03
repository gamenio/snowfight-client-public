#ifndef __WORLD_SOCKET_MGR_H__
#define __WORLD_SOCKET_MGR_H__

#include "common/Common.h"
#include "common/network/SocketMgr.h"
#include "WorldSocket.h"


NS_BEGIN

class World;

class WorldSocketMgr: public SocketMgr<WorldSocket, World>
{
public:
	WorldSocketMgr(World* world):
		SocketMgr<WorldSocket, World>(world)
	{ 
	}

	~WorldSocketMgr()
	{ 
	}
};


NS_END

#endif // __WORLD_SOCKET_MGR_H__
