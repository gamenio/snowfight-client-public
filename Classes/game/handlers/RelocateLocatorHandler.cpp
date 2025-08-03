#include "game/client/WorldSession.h"

#include "game/World.h"
#include "game/entities/LocationInfo.h"
#include "game/behaviors/UnitLocator.h"
#include "game/movement/UnitLocatorMoveSpline.h"

NS_BEGIN

void WorldSession::handleRelocateLocator(WorldPacket& recvPacket)
{
	LocationInfo location;
	recvPacket.unpack(location);

	Object* object = m_world->findObject(location.guid);
	NS_ASSERT_LOG(object != nullptr, "Object is not in world");
	if (!object)
		return;

	//CCLOG("Relocate locator: %s", location.description().c_str());

	UnitLocator* locator = object->asUnitLocator();
	if (locator->isAlive())
		locator->getMoveSpline()->moveBy(location);
	else
		locator->updatePosition(location.position);
}

NS_END