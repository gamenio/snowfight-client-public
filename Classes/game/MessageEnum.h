#ifndef __MESSAGE_ENUM_H__
#define __MESSAGE_ENUM_H__

#include "common/Common.h"


//
// The type of messages sent by the MessageDispatcher.
// Message type naming rules:
// <Module name>MSG_<Type name...>
//

NS_BEGIN

// Socket message type
enum SockMsgType
{
	SOCKMSG_AUTH_CONNECTED,
	SOCKMSG_AUTH_DISCONNECTED,
	SOCKMSG_AUTH_NETWORK_ERROR,
	SOCKMSG_WORLD_CONNECTED,
	SOCKMSG_WORLD_DISCONNECTED,
	SOCKMSG_WORLD_NETWORK_ERROR,
	SOCKMSG_WORLD_SESSION_RECV_QUEUE_ERROR,

};

NS_END


#endif // __MESSAGE_ENUM_H__