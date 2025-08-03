#include "game/client/WorldSession.h"

#include "game/client/protocol/pb/UseItem.pb.h"
#include "game/client/protocol/pb/ItemUseResult.pb.h"
#include "game/client/protocol/pb/ItemCooldownList.pb.h"
#include "game/client/protocol/pb/ItemApplicationUpdate.pb.h"

#include "game/World.h"
#include "game/behaviors/CarriedItem.h"
#include "game/behaviors/MyCharacter.h"

NS_BEGIN

void WorldSession::sendUseItem(int32 slot, ObjectGuid const& item)
{
	UseItem useItem;
	useItem.set_slot(slot);
	useItem.set_item(item.getRawValue());

	WorldPacket packet(world::CMSG_USE_ITEM, std::move(useItem));
	this->sendPacket(std::move(packet));
}

void WorldSession::handleItemUseResult(WorldPacket& recvPacket)
{
	MyCharacter* myChar = this->getMyCharacter();
	if (!myChar)
		return;

	ItemUseResult result;
	recvPacket.unpack(result);

	ObjectGuid itemGuid(result.item());
	Object* obj = m_world->findObject(itemGuid);
	if (!obj)
		return;

	CarriedItem* item = obj->asCarriedItem();
	if (result.status() == ITEM_USE_STATUS_OK)
		myChar->getItemCooldownProcesser()->startCooldown(item->getData()->getItemId(), item->getData()->getCooldownDuration());

	m_world->notifyItemUseResult(result);
}

void WorldSession::handleItemApplicationUpdate(WorldPacket& recvPacket)
{
	ItemApplicationUpdate update;
	recvPacket.unpack(update);

	Object* obj = m_world->findObject(ObjectGuid(update.target()));
	if (!obj)
		return;

	m_world->notifyItemApplicationUpdate(update);
}

void WorldSession::handleItemApplicationUpdateAll(WorldPacket& recvPacket)
{
	ItemApplicationUpdateAll updateAll;
	recvPacket.unpack(updateAll);

	Object* obj = m_world->findObject(ObjectGuid(updateAll.target()));
	if (!obj)
		return;

	m_world->notifyItemApplicationUpdateAll(updateAll);
}

void WorldSession::handleItemCooldownList(WorldPacket& recvPacket)
{
	MyCharacter* myChar = this->getMyCharacter();
	if (!myChar)
		return;

	ItemCooldownList cooldownList;
	recvPacket.unpack(cooldownList);

	for (int32 i = 0; i < cooldownList.result_size(); ++i)
	{
		ItemCooldownInfo const& info = cooldownList.result(i);
		if (info.remaining_time() <= 0)
			continue;

		myChar->getItemCooldownProcesser()->startCooldown(info.item_id(), info.duration(), info.remaining_time());
	}
}

NS_END