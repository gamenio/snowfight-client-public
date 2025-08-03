#include "game/client/WorldSession.h"

#include "game/client/protocol/pb/GMCommand.pb.h"


NS_BEGIN

void WorldSession::sendGMCommand(std::string const& line)
{
	GMCommand command;
	command.set_line(line);
	WorldPacket packet(world::CMSG_GM_COMMAND, std::move(command));

	this->sendPacket(std::move(packet));
}

NS_END
