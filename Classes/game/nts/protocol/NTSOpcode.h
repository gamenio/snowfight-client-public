#ifndef __NTS_OPCODE_H__
#define __NTS_OPCODE_H__


#include "common/Common.h"

NS_BEGIN

namespace nts
{
	enum Opcode
	{
		MSG_NULL						= 0x0000,
		CMSG_TIME_QUERY					= 0x0001,
		SMSG_TIME_RESULT				= 0x0002,
		NUM_MSG_TYPES
	};


} // namespace nts


NS_END


#endif // __NTS_OPCODE_H__
