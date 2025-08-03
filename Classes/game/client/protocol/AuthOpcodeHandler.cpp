#include "AuthOpcodeHandler.h"

#include "game/client/AuthSession.h"

NS_BEGIN

namespace auth
{
	char const* gOpcodeNameTable[NUM_MSG_TYPES] = {
		"MSG_NULL",
		"CMSG_LOGON_CHALLENGE",
		"SMSG_LOGON_RESULT",
		"CMSG_GET_REALM_LIST",
		"SMSG_REALM_LIST",

	};

	std::unordered_map<uint16, OpcodeHandler> gOpcodeHandlerTable = {
		{ SMSG_LOGON_RESULT,			{ &AuthSession::handleLogonResult	} },
		{ SMSG_REALM_LIST,				{ &AuthSession::handleRealmList		} },
	};

}




NS_END