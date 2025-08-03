#ifndef __NTS_OPCODE_HANDLER_H__
#define __NTS_OPCODE_HANDLER_H__

#include <unordered_map>
#include <sstream>

#include "common/Common.h"
#include "NTSOpcode.h"

NS_BEGIN

namespace nts
{
	extern char const* gOpcodeNameTable[NUM_MSG_TYPES];

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

} // namespace nts



NS_END


#endif // __NTS_OPCODE_HANDLER_H__
