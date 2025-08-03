#include "EquipmentBar.h"

#include "game/World.h"

NS_BEGIN

#define ACTION_MOVE_TO_POSITION				1
#define MOVE_DURATION						0.3f

#define EQUIPMENT_SLOT_SIZE					Size(38, 38)

#define SCHEDULE_KEY_HIDE_DELAYED			"HideDelayed"
#define HIDE_DELAY							3.0f

EquipmentBar* EquipmentBar::create()
{
	auto ret = new (std::nothrow) EquipmentBar();
	if (ret && ret->init())
	{
		ret->autorelease();
	}
	else
	{
		CC_SAFE_DELETE(ret);
	}

	return ret;
}

EquipmentBar::EquipmentBar() :
	m_isShown(false)
{
}

EquipmentBar::~EquipmentBar()
{
}

bool EquipmentBar::init()
{
	if (!Node::init())
		return false;

	World* world = World::getInstance();
	world->setEquipmentItemLifecycleListener(this);
	world->addCarriedItemListener(this);

	this->setIgnoreAnchorPointForPosition(false);

	this->initSlots();

	return true;
}

void EquipmentBar::update(float delta)
{
	for (auto p : m_carriedProps)
	{
		CarriedProp* obj = p.second;
		obj->update(delta);
	}

	for (int32 i = 0; i < EQUIPMENT_SLOTS_COUNT; ++i)
	{
		EquipmentSlot* equipSlot = m_equipmentSlots[i];
		equipSlot->update(delta);
	}
}

void EquipmentBar::cleanAfterUpdate()
{
	for (auto p : m_carriedProps)
	{
		CarriedProp* prop = p.second;
		if (prop->isActive())
			prop->cleanUpdateMask();
	}
}

void EquipmentBar::show(Point const& moveToPosition)
{
	if (m_isShown)
		return;

	std::array<Point, EquipmentSlot::MAX_SLOT_TYPES> slotPositions;
	slotPositions[EquipmentSlot::HAT] = moveToPosition;
	slotPositions[EquipmentSlot::GLOVES].x = slotPositions[EquipmentSlot::HAT].x - EQUIPMENT_SLOT_SIZE.width / 2 - 1 - EQUIPMENT_SLOT_SIZE.width / 2;
	slotPositions[EquipmentSlot::GLOVES].y = slotPositions[EquipmentSlot::HAT].y + EQUIPMENT_SLOT_SIZE.height / 2 - 7 - EQUIPMENT_SLOT_SIZE.height / 2;
	slotPositions[EquipmentSlot::SNOWBALL_MAKER].x = slotPositions[EquipmentSlot::GLOVES].x - EQUIPMENT_SLOT_SIZE.width / 2 + 2 - EQUIPMENT_SLOT_SIZE.width / 2;
	slotPositions[EquipmentSlot::SNOWBALL_MAKER].y = slotPositions[EquipmentSlot::GLOVES].y + EQUIPMENT_SLOT_SIZE.height / 2 - 22 - EQUIPMENT_SLOT_SIZE.height / 2;
	slotPositions[EquipmentSlot::JACKET].x = -slotPositions[EquipmentSlot::GLOVES].x;
	slotPositions[EquipmentSlot::JACKET].y = slotPositions[EquipmentSlot::GLOVES].y;
	slotPositions[EquipmentSlot::SHOES].x = -slotPositions[EquipmentSlot::SNOWBALL_MAKER].x;
	slotPositions[EquipmentSlot::SHOES].y = slotPositions[EquipmentSlot::SNOWBALL_MAKER].y;

	auto const& children = this->getChildren();
	for (ssize_t i = 0; i < children.size(); ++i)
	{
		EquipmentSlot* equipSlot = dynamic_cast<EquipmentSlot*>(children.at(i));
		if(!equipSlot)
			continue;

		if (Action* action = equipSlot->getActionByTag(ACTION_MOVE_TO_POSITION))
			equipSlot->stopAction(action);

		MoveTo* move = MoveTo::create(MOVE_DURATION, slotPositions[equipSlot->getType()]);
		ScaleTo* scale = ScaleTo::create(MOVE_DURATION, 1.0f);
		FadeTo* fadeTo = FadeTo::create(MOVE_DURATION, 255);
		ActionInterval* spawn = EaseQuadraticActionOut::create(Spawn::create(move, scale, fadeTo, nullptr));
		Sequence* action = Sequence::create(Show::create(), spawn, nullptr);
		action->setTag(ACTION_MOVE_TO_POSITION);
		equipSlot->runAction(action);
	}

	this->scheduleOnce([this](float dt) {
		this->hide();
	}, HIDE_DELAY, SCHEDULE_KEY_HIDE_DELAYED);

	m_isShown = true;
}

void EquipmentBar::hide()
{
	if (!m_isShown)
		return;

	this->unschedule(SCHEDULE_KEY_HIDE_DELAYED);

	auto const& children = this->getChildren();
	for (ssize_t i = 0; i < children.size(); ++i)
	{
		EquipmentSlot* equipSlot = dynamic_cast<EquipmentSlot*>(children.at(i));
		if (!equipSlot)
			continue;

		if (Action* action = equipSlot->getActionByTag(ACTION_MOVE_TO_POSITION))
			equipSlot->stopAction(action);

		MoveTo* move = MoveTo::create(MOVE_DURATION, Point::ZERO);
		ScaleTo* scale = ScaleTo::create(MOVE_DURATION, 0.0f);
		FadeTo* fadeTo = FadeTo::create(MOVE_DURATION, 0);
		ActionInterval* spawn = EaseQuadraticActionIn::create(Spawn::create(move, scale, fadeTo, nullptr));
		Sequence* action = Sequence::create(spawn, Hide::create(), nullptr);
		action->setTag(ACTION_MOVE_TO_POSITION);
		equipSlot->runAction(action);
	}

	m_isShown = false;
}

void EquipmentBar::onEquipmentItemDestroyed(ObjectGuid const& guid)
{
	auto it = m_carriedProps.find(guid);
	if (it != m_carriedProps.end())
	{
		CarriedProp* prop = (*it).second;
		EquipmentSlot* equipSlot = this->getEquipmentSlot(prop->getData()->getSlot());
		NS_ASSERT(equipSlot);
		if(equipSlot)
			equipSlot->removeProp();
		m_carriedProps.erase(it);
	}
}

void EquipmentBar::onEquipmentItemActivated(DataCarriedItem* data)
{
	NS_ASSERT(!data->getGuid().isEmpty());

	CarriedProp* prop;
	auto it = m_carriedProps.find(data->getGuid());
	if (it != m_carriedProps.end())
		prop = (*it).second;
	else
	{
		prop = CarriedProp::createWithData(data);
		m_carriedProps.insert(data->getGuid(), prop);
	}
	EquipmentSlot* equipSlot = this->getEquipmentSlot(prop->getData()->getSlot());
	NS_ASSERT(equipSlot);
	if (equipSlot)
	{
		equipSlot->setProp(prop);
		equipSlot->onPropActivated();
	}
}

void EquipmentBar::onEquipmentItemInactivated(ObjectGuid const& guid)
{
	CarriedProp* prop = this->findCarriedProp(guid);
	if (prop)
	{
		EquipmentSlot* equipSlot = this->getEquipmentSlot(prop->getData()->getSlot());
		NS_ASSERT(equipSlot);
		if(equipSlot)
			equipSlot->onPropInactivated();
	}
}

void EquipmentBar::initSlots()
{
	EquipmentSlot* equipSlot = nullptr;

	equipSlot = EquipmentSlot::create(EquipmentSlot::HAT);
	m_equipmentSlots[getEquipmentPos(EQUIPMENT_SLOT_HAT)] = equipSlot;
	this->addChild(equipSlot);

	equipSlot = EquipmentSlot::create(EquipmentSlot::GLOVES);
	m_equipmentSlots[getEquipmentPos(EQUIPMENT_SLOT_GLOVES)] = equipSlot;
	this->addChild(equipSlot);

	equipSlot = EquipmentSlot::create(EquipmentSlot::SNOWBALL_MAKER);
	m_equipmentSlots[getEquipmentPos(EQUIPMENT_SLOT_SNOWBALL_MAKER)] = equipSlot;
	this->addChild(equipSlot);

	equipSlot = EquipmentSlot::create(EquipmentSlot::JACKET);
	m_equipmentSlots[getEquipmentPos(EQUIPMENT_SLOT_JACKET)] = equipSlot;
	this->addChild(equipSlot);

	equipSlot = EquipmentSlot::create(EquipmentSlot::SHOES);
	m_equipmentSlots[getEquipmentPos(EQUIPMENT_SLOT_SHOES)] = equipSlot;
	this->addChild(equipSlot);

	auto const& children = this->getChildren();
	for (ssize_t i = 0; i < children.size(); ++i)
	{
		EquipmentSlot* equipSlot = dynamic_cast<EquipmentSlot*>(children.at(i));
		if (!equipSlot)
			continue;

		equipSlot->setAnchorPoint(Point::ANCHOR_MIDDLE);
		equipSlot->setContentSize(EQUIPMENT_SLOT_SIZE);
		equipSlot->setScale(0.0f);
		equipSlot->setOpacity(0);
		equipSlot->setVisible(false);
		equipSlot->setPosition(Point::ZERO);
	}
}

CarriedProp* EquipmentBar::findCarriedProp(ObjectGuid const& guid) const
{
	auto it = m_carriedProps.find(guid);
	if (it != m_carriedProps.end())
		return (*it).second;

	return nullptr;
}

EquipmentSlot* EquipmentBar::getEquipmentSlot(int32 slot) const
{
	int32 pos = getEquipmentPos(slot);
	if(pos != SLOT_INVALID)
		return m_equipmentSlots[pos];

	return nullptr;
}

void EquipmentBar::setEquipmentSlot(int32 slot, EquipmentSlot* equipmentSlot)
{
	int32 pos = getEquipmentPos(slot);
	m_equipmentSlots[pos] = equipmentSlot;
}

NS_END
