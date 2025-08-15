#ifndef __NTS_MESSAGE_ENUM_H__
#define __NTS_MESSAGE_ENUM_H__

#include "common/Common.h"

//
// The type of message sent by the message dispatcher
//
// Message type naming rules:
// <module name>MSG_<type name... >
//

NS_BEGIN

namespace nts
{

// Socket message type
enum SockMsgType
{
	SOCKMSG_CONNECTED				= 0,
	SOCKMSG_DISCONNECTED,
	SOCKMSG_NETWORK_ERROR,
	SOCKMSG_TIME_INFO,
};

} // namespace nts

NS_END

#endif // __NTS_MESSAGE_ENUM_H__