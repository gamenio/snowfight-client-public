#include "UnitLocator.h"

#include "game/movement/UnitLocatorMoveSpline.h"


NS_BEGIN

UnitLocator::UnitLocator() :
	m_moveSpline(nullptr)
{
	m_type |= TypeMask::TYPEMASK_UNIT_LOCATOR;
	m_typeId = TypeID::TYPEID_UNIT_LOCATOR;

	m_moveSpline = new UnitLocatorMoveSpline(this);
}

UnitLocator::~UnitLocator()
{
	CC_SAFE_DELETE(m_moveSpline);
}

void UnitLocator::update(float delta)
{
	LocatorObject::update(delta);

	if (!this->isAlive() || !this->isInWorld())
		return;

	if (m_moveSpline)
		m_moveSpline->update(delta);
}

bool UnitLocator::updatePosition(Point const& newPosition)
{
	this->getData()->setPosition(newPosition);
	return true;
}

bool UnitLocator::isAlive() const
{
	if (this->getData())
		return this->getData()->isAlive();

	return true;
}

void UnitLocator::stopMoving()
{
	if (this->getMoveSpline())
		this->getMoveSpline()->stop(!this->isAlive());
}

void UnitLocator::activate()
{
	if (this->isActivated())
		return;

	LocatorObject::activate();
}

void UnitLocator::inactivate()
{
	if (!this->isActivated())
		return;

	this->stopMoving();

	LocatorObject::inactivate();
}

void UnitLocator::updateObject(UpdateType updateType, uint32 updateFlags, DataBasic* data)
{
	LocatorObject::updateObject(updateType, updateFlags, data);

	DataUnitLocator* unitLocator = data->asUnitLocator();
	if (!unitLocator->isAlive())
		this->stopMoving();

	if (updateType == UPDATE_TYPE_CREATE)
		this->getMap()->activateLocatorObject(this);
}

DataBasic* UnitLocator::loadData(ObjectGuid const& guid)
{
	DataUnitLocator* data = new DataUnitLocator();
	data->autorelease();
	data->setGuid(guid);
	this->setData(data);

	return data;
}


NS_END