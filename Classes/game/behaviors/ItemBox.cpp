#include "ItemBox.h"

#include "game/behaviors/ObjectShapes.h"
#include "game/tiles/TileCoord.h"

NS_BEGIN

ItemBox::ItemBox():
	m_openState(OPEN_STATE_LOCKED)
{
    m_type |= TypeMask::TYPEMASK_ITEMBOX;
    m_typeId = TypeID::TYPEID_ITEMBOX;
}

ItemBox::~ItemBox()
{
    
}

void ItemBox::activate()
{
	if (this->isActivated())
		return;

	WorldObject::activate();

	TileCoord currCoord(m_map->getMapData()->getMapSize(), this->getData()->getPosition());
	m_map->setTileClosed(currCoord);
}

void ItemBox::inactivate()
{
	if (!this->isActivated())
		return;

	TileCoord currCoord(m_map->getMapData()->getMapSize(), this->getData()->getPosition());
	m_map->setTileClosed(currCoord, false);

	WorldObject::inactivate();
}

void ItemBox::setOpenState(OpenState state)
{
	m_openState = state;

	switch (state)
	{
	case OPEN_STATE_LOCKED:
		break;
	case OPEN_STATE_OPENED:
		break;
	}
}

void ItemBox::updateObject(UpdateType updateType, uint32 updateFlags, DataBasic* data)
{
	WorldObject::updateObject(updateType, updateFlags, data);
	DataItemBox* dItemBox = data->asDataItemBox();
	if (!dItemBox->isLocked())
	{
		if (this->isLocked())
		{
			this->setOpenState(OPEN_STATE_OPENED);
		}
	}
	else
	{
		if (!this->isLocked())
		{
			this->setOpenState(OPEN_STATE_LOCKED);
		}
	}

	if(updateType == UPDATE_TYPE_CREATE)
		this->getMap()->activateObject(this);
}

DataBasic* ItemBox::loadData(ObjectGuid const& guid)
{
    DataItemBox* data = new DataItemBox();
    data->autorelease();
    data->setGuid(guid);
    
    data->setObjectSize(ITEMBOX_OBJECT_SIZE);
    data->setAnchorPoint(ITEMBOX_ANCHOR_POINT);
	data->setObjectRadiusInMap(ITEMBOX_OBJECT_RADIUS_IN_MAP);
	data->setLaunchCenter(ITEMBOX_LAUNCH_CENTER);
    this->setData(data);
    
    return data;
}


NS_END
