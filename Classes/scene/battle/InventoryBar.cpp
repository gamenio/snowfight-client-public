#include "InventoryBar.h"

#include "game/World.h"

NS_BEGIN

#define CUSTOM_SLOT_MARGIN_LEFT				2
#define FIRSTAID_SLOT_MARGIN_LEFT			56

InventoryBar* InventoryBar::create()
{
	auto ret = new (std::nothrow) InventoryBar();
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

InventoryBar::InventoryBar() :
	m_isEnabled(true)
{
}

InventoryBar::~InventoryBar()
{
}

bool InventoryBar::init()
{
	if (!Node::init())
		return false;

	World* world = World::getInstance();
	world->setInventoryItemLifecycleListener(this);
	world->addCarriedItemListener(this);

	this->setIgnoreAnchorPointForPosition(false);

	this->initSlots();

	return true;
}

void InventoryBar::update(float delta)
{
	for (auto p : m_carriedProps)
	{
		CarriedProp* obj = p.second;
		obj->update(delta);
	}

	for (int32 i = 0; i < INVENTORY_SLOTS_COUNT; ++i)
	{
		InventorySlot* invSlot = m_inventorySlots[i];
		invSlot->update(delta);
	}
}

void InventoryBar::cleanAfterUpdate()
{
	for (auto p : m_carriedProps)
	{
		CarriedProp* prop = p.second;
		if (prop->isActive())
			prop->cleanUpdateMask();
	}
}

void InventoryBar::setEnabled(bool enabled)
{
	if (m_isEnabled == enabled)
		return;

	m_isEnabled = enabled;
	for (int32 i = 0; i < INVENTORY_SLOTS_COUNT; ++i)
	{
		InventorySlot* invSlot = m_inventorySlots[i];
		invSlot->setEnabled(m_isEnabled);
	}
}

void InventoryBar::onInventoryItemDestroyed(ObjectGuid const& guid)
{
	auto it = m_carriedProps.find(guid);
	if (it != m_carriedProps.end())
	{
		CarriedProp* prop = (*it).second;
		InventorySlot* invSlot = this->getInventorySlot(prop->getData()->getSlot());
		NS_ASSERT(invSlot);
		if(invSlot)
			invSlot->removeProp();
		m_carriedProps.erase(it);
	}
}

void InventoryBar::onInventoryItemActivated(DataCarriedItem* data)
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
	InventorySlot* invSlot = this->getInventorySlot(prop->getData()->getSlot());
	NS_ASSERT(invSlot);
	if (invSlot)
	{
		invSlot->setProp(prop);
		invSlot->onPropActivated();
	}
}

void InventoryBar::onInventoryItemInactivated(ObjectGuid const& guid)
{
	CarriedProp* prop = this->findCarriedProp(guid);
	if (prop)
	{
		InventorySlot* invSlot = this->getInventorySlot(prop->getData()->getSlot());
		NS_ASSERT(invSlot);
		if(invSlot)
			invSlot->onPropInactivated();
	}
}

void InventoryBar::onItemUseResult(ItemUseResult const& result)
{
	CarriedProp* prop = this->findCarriedProp(ObjectGuid(result.item()));
	if (prop)
	{
		InventorySlot* invSlot = this->getInventorySlot(prop->getData()->getSlot());
		NS_ASSERT(invSlot);
		if(invSlot)
			invSlot->onItemUseResult(static_cast<ItemUseStatus>(result.status()));
	}
}

void InventoryBar::initSlots()
{
	float nextX = 0;
	float maxHeight = 0;
	for (int32 i = INVENTORY_SLOT_START; i < INVENTORY_SLOT_END; ++i)
	{
		InventorySlot* invSlot = nullptr;
		if (i == INVENTORY_SLOT_FIRST_AID)
		{
			invSlot = InventorySlot::create(InventorySlot::FIRST_AID);
			invSlot->setPosition(nextX + FIRSTAID_SLOT_MARGIN_LEFT, 0);
		}
		else // INVENTORY_SLOT_CUSTOM
		{
			invSlot = InventorySlot::create(InventorySlot::CUSTOM);
			invSlot->setPosition(nextX + CUSTOM_SLOT_MARGIN_LEFT, 0);
		}
		int32 slot = i - INVENTORY_SLOT_START;
		invSlot->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
		m_inventorySlots[slot] = invSlot;
		this->addChild(invSlot);

		nextX = invSlot->getBoundingBox().getMaxX();
		maxHeight = std::max(maxHeight, invSlot->getBoundingBox().size.height);
	}

	this->setContentSize(Size(nextX, maxHeight));
}

CarriedProp* InventoryBar::findCarriedProp(ObjectGuid const& guid) const
{
	auto it = m_carriedProps.find(guid);
	if (it != m_carriedProps.end())
		return (*it).second;

	return nullptr;
}

InventorySlot* InventoryBar::getInventorySlot(int32 slot) const
{
	int32 pos = getInventoryPos(slot);
	if(pos != SLOT_INVALID)
		return m_inventorySlots[pos];

	return nullptr;
}

NS_END
