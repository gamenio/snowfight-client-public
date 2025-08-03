#include "NTSOpcodeHandler.h"

NS_BEGIN

namespace nts
{
	char const* gOpcodeNameTable[NUM_MSG_TYPES] = {
		"MSG_NULL",
		"CMSG_TIME_QUERY",
		"SMSG_TIME_RESULT",
	};
}


NS_END