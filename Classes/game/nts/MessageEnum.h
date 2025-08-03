#ifndef __NTS_MESSAGE_ENUM_H__
#define __NTS_MESSAGE_ENUM_H__

#include "common/Common.h"

//
// 消息分发器发送的消息类型。
//
// 消息类型命名规则：
// <模块名>MSG_<类型名...>
//

NS_BEGIN

namespace nts
{

// 套接字消息类型
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