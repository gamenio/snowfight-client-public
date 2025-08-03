#ifndef __NTS_SOCKET_MGR_H__
#define __NTS_SOCKET_MGR_H__

#include "common/Common.h"
#include "common/network/SocketMgr.h"
#include "NTSSocket.h"

NS_BEGIN

class TimeService;

class NTSSocketMgr: public SocketMgr<NTSSocket, TimeService>
{
public:
	NTSSocketMgr(TimeService* service) :
		SocketMgr<NTSSocket, TimeService>(service)
	{ 
	}

	~NTSSocketMgr() 
	{ 
	}
};

NS_END

#endif // __NTS_SOCKET_MGR_H__
