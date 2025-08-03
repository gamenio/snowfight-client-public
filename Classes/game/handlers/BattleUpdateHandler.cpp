#include "game/client/WorldSession.h"


#include "game/client/protocol/pb/BattleUpdate.pb.h"

#include "game/behaviors/MyCharacter.h"
#include "game/World.h"

NS_BEGIN

void WorldSession::handleBattleUpdate(WorldPacket& recvPacket)
{
	MyCharacter* myChar = this->getMyCharacter();
	NS_ASSERT_LOG(myChar, "My character was not created.");
	if (!myChar)
		return;

	BattleUpdate update;
	recvPacket.unpack(update);
	m_world->getMap()->setMagicBeanCount(update.magicbean_count());

	m_world->notifyBattleUpdate(update);
}

NS_END



