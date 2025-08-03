#ifndef __WORLD_OPCODE_HANDLER_H__
#define __WORLD_OPCODE_HANDLER_H__

#include <unordered_map>
#include <sstream>

#include "common/Common.h"
#include "WorldOpcode.h"
#include "game/client/WorldSession.h"

NS_BEGIN

namespace world
{
	struct OpcodeHandler
	{
		void (WorldSession::*handler)(WorldPacket& packet);
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

} //namespace world





NS_END


#endif // __WORLD_OPCODE_HANDLER_H__
