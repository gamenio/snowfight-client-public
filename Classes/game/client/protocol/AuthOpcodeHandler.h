#ifndef __AUTH_OPCODE_HANDLER_H__
#define __AUTH_OPCODE_HANDLER_H__

#include <unordered_map>
#include <sstream>

#include "common/Common.h"
#include "AuthOpcode.h"
#include "game/client/AuthSession.h"

NS_BEGIN

namespace auth
{
	struct OpcodeHandler
	{
		void (AuthSession::*handler)(AuthPacket& packet);
	};

	extern char const* gOpcodeNameTable[NUM_MSG_TYPES];
	extern std::unordered_map<uint16, OpcodeHandler> gOpcodeHandlerTable;

	inline const char* lookupOpcodeName(uint16 id)
	{
		if (id >= NUM_MSG_TYPES)
			return "Received unknown opcode, it's more than max!";
		return gOpcodeNameTable[id];
	}

	inline std::string getOpcodeNameForLogging(uint16 opcode)
	{
		std::ostringstream ss;
		ss << '[' << lookupOpcodeName(opcode) << " 0x" << std::hex << std::uppercase << opcode << std::nouppercase << " (" << std::dec << opcode << ")]";
		return ss.str();
	}

} //namespace auth





NS_END


#endif // __AUTH_OPCODE_HANDLER_H__
