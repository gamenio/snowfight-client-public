#include "Item.h"

#include "game/behaviors/ObjectShapes.h"
#include "game/utils/UnitHelper.h"
#include "game/behaviors/Unit.h"
#include "game/ObjectMgr.h"
#include "MyCharacter.h"

NS_BEGIN

Item::Item() :
	m_isAvailabilityChanged(false)
{
    m_type |= TypeMask::TYPEMASK_ITEM;
    m_typeId = TypeID::TYPEID_ITEM;
}

Item::~Item()
{
    
}

void Item::activate()
{
	if (this->isActivated())
		return;

	WorldObject::activate();

	this->updateAvailability();
}

void Item::inactivate()
{
	if (!this->isActivated())
		return;

	WorldObject::inactivate();
}

void Item::update(float delta)
{
	if (!this->isInWorld())
		return;

	this->availabilityChange();
}

void Item::updateAvailability()
{
	m_isAvailabilityChanged = true;
}

DataBasic* Item::loadData(ObjectGuid const& guid)
{
    DataItem* data = new DataItem();
    data->autorelease();
    data->setGuid(guid);
    
	data->setObjectRadiusInMap(ITEM_OBJECT_RADIUS_IN_MAP);
    this->setData(data);
    
    return data;
}

void Item::updateObject(UpdateType updateType, uint32 updateFlags, DataBasic* data)
{
	NS_ASSERT(updateType == UPDATE_TYPE_CREATE);

	DataItem* dItem = data->asDataItem();
	//CCLOG("Item::updateObject dropDuration: %d dropElapsed: %d", dItem->getDropDuration(), dItem->getDropElapsed());
	WorldObject* holder = this->getMap()->findObject(dItem->getHolder());
	if (holder && holder->isType(TYPEMASK_UNIT)
		&& dItem->getDropElapsed() <= 0)
	{
		Unit* unit = holder->asUnit();
		dItem->setHolderOrigin(unit->getData()->getPosition());
	}

	if (dItem->getDropElapsed() < dItem->getDropDuration())
	{
		Vec2 launchPos = UnitHelper::computeLaunchPosition(dItem->getMapData(), dItem->getHolderOrigin(), dItem->getLaunchCenter(), dItem->getLaunchRadiusInMap(), dItem->getPosition());
		TrajectoryGenerator trajGenerator(TRAJECTORY_TYPE_ITEM, launchPos, dItem->getPosition());
		trajGenerator.compute();
		BezierCurveConfig const& config = trajGenerator.getBezierCurveConfig();
		dItem->setTrajectory(config);
	}

	this->getMap()->activateObject(this);
}

void Item::availabilityChange()
{
	if (!m_isAvailabilityChanged)
		return;

	MyCharacter* myChar = this->getMap()->getMyChar();
	NS_ASSERT(myChar);

	ItemTemplate const* tmpl = sObjectMgr->getItemTemplate(this->getData()->getItemId());
	NS_ASSERT(tmpl);
	if (myChar->canStoreItem(tmpl) == PICKUP_STATUS_OK)
		this->getData()->setAvailable(true);
	else
		this->getData()->setAvailable(false);

	m_isAvailabilityChanged = false;
}

NS_END
