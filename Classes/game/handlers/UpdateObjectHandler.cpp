#include "game/client/WorldSession.h"

#include "game/client/protocol/pb/DestroyObject.pb.h"

#include "game/entities/updates/UpdateObject.h"
#include "game/behaviors/MyCharacter.h"
#include "game/behaviors/Player.h"
#include "game/behaviors/Robot.h"
#include "game/behaviors/ItemBox.h"
#include "game/behaviors/Item.h"
#include "game/behaviors/Projectile.h"
#include "game/behaviors/UnitLocator.h"
#include "game/behaviors/CarriedItem.h"
#include "game/World.h"


NS_BEGIN

void WorldSession::handleDestroyObject(WorldPacket& recvPacket)
{
	DestroyObject message;
	recvPacket.unpack(message);

	m_world->removeFromWorld(ObjectGuid(message.guid()), true);
}

void WorldSession::handleUpdateObject(WorldPacket& packet)
{
	UpdateObject update;
	update.onUpdateCreate = std::bind(&WorldSession::onUpdateCreate, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
	update.onObjectCreateFinished = std::bind(&WorldSession::onObjectCreateFinished, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	update.onUpdateValues = std::bind(&WorldSession::onUpdateValues, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	update.onUpdateOutOfRange = std::bind(&WorldSession::onUpdateOutOfRange, this, std::placeholders::_1);
	update.onValuesUpdateFinished = std::bind(&WorldSession::onValuesUpdateFinished, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

	packet.unpack(update);
}

void WorldSession::onObjectCreateFinished(uint32 updateFlags, DataBasic* data, void* holder)
{
	Object* object = static_cast<Object*>(holder);
	m_world->addToWorld(object);

	object->updateObject(UpdateType::UPDATE_TYPE_CREATE, updateFlags, object->getData());
}

void WorldSession::onValuesUpdateFinished(uint32 updateFlags, DataBasic* data, void* holder)
{
	Object* object = static_cast<Object*>(holder);
	object->updateObject(UpdateType::UPDATE_TYPE_VALUES, updateFlags, object->getData());
}

DataBasic* WorldSession::onUpdateValues(uint32 updateFlags, ObjectGuid const& guid, void** outHolder)
{
	Object* object = m_world->findObject(guid);
	NS_ASSERT_LOG(object, "Object is not in world");
	NS_ASSERT_LOG(object->getData(), "Object has no set data");
	*outHolder = object;

	return object->getData();
}

DataBasic* WorldSession::onUpdateCreate(uint32 updateFlags, ObjectGuid const& guid, DataTypeID typeId, void** outHolder)
{
	Object* object = m_world->findObject(guid, true);
	if (object)
	{
		NS_ASSERT_LOG(object->getData(), "Object has no set data");
		*outHolder = object;
		return object->getData();
	}

	switch (typeId)
	{
	case DATA_TYPEID_PLAYER:
	{
		if ((updateFlags & UPDATE_FLAG_SELF) != 0)
		{
			MyCharacter* myChar = new MyCharacter(this);
			myChar->loadData(guid);
			object = myChar;
		}
		else
		{
			Player* player = new Player();
			player->loadData(guid);
			object = player;
		}
		break;
	}
	case DATA_TYPEID_ROBOT:
	{
		Robot* robot = new Robot();
		robot->loadData(guid);
		object = robot;
		break;
	}
	case DATA_TYPEID_ITEMBOX:
	{
		ItemBox* itemBox = new ItemBox();
		itemBox->loadData(guid);
		object = itemBox;
		break;
	}
    case DATA_TYPEID_ITEM:
	{
		Item* item = new Item();
		item->loadData(guid);
		object = item;
		break;
	}
	case DATA_TYPEID_PROJECTILE:
	{
		Projectile* projectile = new Projectile();
		projectile->loadData(guid);
		object = projectile;
		break;
	}
	case DATA_TYPEID_UNIT_LOCATOR:
	{
		UnitLocator* unitLocator = new UnitLocator();
		unitLocator->loadData(guid);
		object = unitLocator;
		break;
	}
	case DATA_TYPEID_CARRIED_ITEM:
	{
		CarriedItem* item = new CarriedItem();
		item->loadData(guid);
		object = item;
		break;
	}
	default:
		NS_ASSERT_LOG(false, "Undefined data type");
		break;
	}

	*outHolder = object;

	return object->getData();
}

void WorldSession::onUpdateOutOfRange(std::vector<ObjectGuid> const& guids)
{
	for (auto guid : guids)
	{
		m_world->removeFromWorld(guid, false);
	}
}

NS_END