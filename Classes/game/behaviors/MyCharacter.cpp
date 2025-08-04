#include "MyCharacter.h"

#include "game/client/protocol/pb/AttackInfo.pb.h"

#include "common/utils/Timer.h"
#include "common/utils/MathTools.h"
#include "game/utils/UnitHelper.h"
#include "game/entities/updates/ObjectUpdateFields.h"
#include "game/entities/DataPlayer.h"
#include "game/behaviors/SharedUnitDefines.h"
#include "game/movement/MoveSegmentImpl.h"
#include "game/behaviors/ObjectShapes.h"
#include "game/client/protocol/WorldOpcode.h"
#include "game/client/protocol/WorldOpcodeHandler.h"
#include "game/movement/MyMoveSpline.h"
#include "game/movement/UnitMoveSpline.h"
#include "game/movement/MyMovementGenerator.h"
#include "game/combat/MyStaminaUpdater.h"
#include "game/World.h"
#include "game/ObjectMgr.h"
#include "game/behaviors/CarriedItem.h"
#include "Player.h"
#include "ItemBox.h"
#include "Item.h"


NS_BEGIN

#define ATTACK_DELAY						100		// Attack delay time. Unit: milliseconds
#define FACING_DIRECTION_LOCK_DURATION      0.5f	// Facing direction lock duration. Unit: seconds
#define HAND_DOWN_DELAY						0.5f	// Hand-down time delay. Unit: seconds

MyCharacter::MyCharacter(WorldSession* session) :
	m_session(session),
	m_isMoveEnabled(false),
	m_isAttackTimerEnabled(false),
	m_direction(0),
	m_attackFlags(0),
	m_attackInfoCounter(0),
	m_isHandDownTimerEnabled(false),
	m_isFacingDirectionLocked(false),
	m_isItemAvailabilitiesChanged(false),
	m_itemCooldownProcesser(new ItemCooldownProcesser(this)),
	m_inventoryItemLifecycleListener(nullptr),
	m_equipmentItemLifecycleListener(nullptr)
{
	m_type |= TYPEMASK_MYCHARACTER;
	m_typeId = TypeID::TYPEID_MYCHARACTER;

	m_facingDirectionUnlockTimer.setDuration(FACING_DIRECTION_LOCK_DURATION);
}


MyCharacter::~MyCharacter()
{
	this->removeAllCarriedItems();

	CC_SAFE_DELETE(m_itemCooldownProcesser);

    m_session = nullptr;
	m_inventoryItemLifecycleListener = nullptr;
	m_equipmentItemLifecycleListener = nullptr;
}

void MyCharacter::cleanupBeforeDelete()
{
	this->removeAllCarriedItems();
	m_itemCooldownProcesser->removeAll();

	Unit::cleanupBeforeDelete();
}

void MyCharacter::activate()
{
	if (this->isActivated())
		return;

	this->activateCarriedItemsInSlots();

	Unit::activate();
}

void MyCharacter::inactivate()
{
	if (!this->isActivated())
		return;

	this->inactivateAllCarriedItems();

	Unit::inactivate();
}

void MyCharacter::setDeathState(DeathState state)
{
	Unit::setDeathState(state);
	switch (state)
	{
	case DEATH_STATE_DEAD:
		this->resetMoveSyncState();
		this->resetStaminaSyncState();
		this->getData()->setStamina(0);
		this->getData()->resetAttackCounter();
		this->getData()->resetConsumedStaminaTotal();
		this->unlockFacingDirection();
		this->stopHandDownTimer();

		m_itemCooldownProcesser->removeAll();
		break;
	case DEATH_STATE_ALIVE:
		break;
	}
}

void MyCharacter::charge()
{
	if (!this->isAlive())
		return;

	if (this->getStaminaUpdater()->isInCharge())
		return;

	if (m_isHandDownTimerEnabled)
		this->stopHandDownTimer();
	else
	{
		this->getData()->addMovementFlag(MOVEMENT_FLAG_HANDUP);
		this->sendMoveSync();
	}
	NS_ASSERT(this->getData()->hasMovementFlag(MOVEMENT_FLAG_HANDUP));

	this->getStaminaUpdater()->charge();
	this->stopAttackTimer();
}

void MyCharacter::chargeStop()
{
	if (!this->getStaminaUpdater()->isInCharge())
		return;

	NS_ASSERT(this->getData()->hasMovementFlag(MOVEMENT_FLAG_HANDUP));

	this->getStaminaUpdater()->chargeStop();
	this->stopAttackTimer();
}

bool MyCharacter::isInCharge() const
{
	return this->getStaminaUpdater()->isInCharge();
}

void MyCharacter::setFullStamina()
{
	this->getData()->setStamina(this->getData()->getMaxStamina());
}

void MyCharacter::buildStaminaInfo(StaminaInfo& stamina) const
{
	stamina.guid = this->getData()->getGuid();
	stamina.time = time_util::getUptimeMillis();
	stamina.counter = this->getData()->getStaminaCounter();
	stamina.stamina = this->getData()->getStamina();
	stamina.flags = this->getData()->getStaminaFlags();
	stamina.attackCounter = this->getData()->getAttackCounter();
	stamina.consumedStaminaTotal = this->getData()->getConsumedStaminaTotal();
	stamina.attackInfoCounter = m_attackInfoCounter;
}

void MyCharacter::resetStaminaSyncState()
{
	MyStaminaUpdater* staminaUpdater = this->getStaminaUpdater();
	if(staminaUpdater)
		staminaUpdater->resetStaminaSyncState();
}

void MyCharacter::startRegenStamina()
{
    MyStaminaUpdater* staminaUpdater = this->getStaminaUpdater();
    if(staminaUpdater)
        staminaUpdater->startRegenStamina();
}

void MyCharacter::addCarriedItem(CarriedItem* item)
{
	if (!item->isInWorld())
	{
		m_carriedItems[item->getData()->getGuid()] = item;
		item->addToWorld();
	}
}

void MyCharacter::removeCarriedItem(ObjectGuid const& guid, bool cleanup)
{
	CarriedItem* item = nullptr;

	auto it = m_carriedItems.find(guid);
	if (it != m_carriedItems.end())
		item = (*it).second;

	NS_ASSERT_LOG(item != nullptr, "Removed CarriedItem does not exist in the map");

	if (cleanup)
	{
		this->notifyCarriedItemDestroyed(item);
		item->removeFromWorld();

		this->removeIfExistsInInactiveCarriedItems(guid);
		m_carriedItems.erase(it);
		CC_SAFE_DELETE(item);

		m_isItemAvailabilitiesChanged = true;
	}
	else
	{
		this->notifyCarriedItemInactivated(item);
		item->inactivate();

		m_inactiveCarriedItems[guid] = item;
		m_carriedItems.erase(it);
	}
}

CarriedItem* MyCharacter::findCarriedItem(ObjectGuid const& guid, bool includeInactiveItems) const
{
	CarriedItem* result = nullptr;

	auto it = m_carriedItems.find(guid);
	if (it != m_carriedItems.end())
	{
		CarriedItem* item = (*it).second;
		result = item;
	}
	else if (includeInactiveItems)
	{
		auto it = m_inactiveCarriedItems.find(guid);
		if (it != m_inactiveCarriedItems.end())
			result = (*it).second;
	}
	return result;
}

void MyCharacter::sendAttackInfo(float direction, uint32 flags)
{
	if (!this->getSession())
		return;

	NS_ASSERT(this->getData()->hasMovementFlag(MOVEMENT_FLAG_HANDUP));
	++m_attackInfoCounter;

	AttackInfo info;
	info.set_launcher(this->getData()->getGuid().getRawValue());
	info.set_direction(direction);
	info.set_flags(flags);
	info.set_counter(m_attackInfoCounter);
	info.set_movement_counter(this->getData()->getMovementCounter());

	WorldPacket packet(world::CMSG_ATTACK, std::move(info));
	this->getSession()->sendPacket(std::move(packet));
}

void MyCharacter::updateAttackTimer(float delta)
{
	if (!m_isAttackTimerEnabled)
		return;

	m_attackTimer.update(delta);
	if (m_attackTimer.passed())
	{
		if (this->getStaminaUpdater()->isInCharge())
			this->getStaminaUpdater()->prepareForChargedAttack();
		this->sendAttackInfo(m_direction, m_attackFlags);

		this->stopAttackTimer();
	}
}

void MyCharacter::attackDelayed(float direction, uint32 flags, int32 delay)
{
	m_attackTimer.setDuration(delay / 1000.f);
	m_direction = direction;
	m_attackFlags = flags;
	m_isAttackTimerEnabled = true;
}

void MyCharacter::stopAttackTimer()
{
	m_attackTimer.reset();
	m_direction = 0;
	m_attackFlags = ATTACK_FLAG_NONE;
	m_isAttackTimerEnabled = false;
}

void MyCharacter::updateHandDownTimer(float delta)
{
	if (!m_isHandDownTimerEnabled)
		return;

	m_handDownTimer.update(delta);
	if (m_handDownTimer.passed())
	{
		NS_ASSERT(!this->getStaminaUpdater()->isInCharge());
		NS_ASSERT(!m_isAttackTimerEnabled);
		NS_ASSERT(this->getData()->hasMovementFlag(MOVEMENT_FLAG_HANDUP));

		this->getData()->clearMovementFlag(MOVEMENT_FLAG_HANDUP);
		this->sendMoveSync();

		m_handDownTimer.reset();
		m_isHandDownTimerEnabled = false;
	}

}

void MyCharacter::updateFacingDirectionUnlockTimer(float delta)
{
	if (m_isFacingDirectionLocked)
	{
		m_facingDirectionUnlockTimer.update(delta);
		if (m_facingDirectionUnlockTimer.passed())
		{
			this->unlockFacingDirection();
		}
	}
}

PickupStatus MyCharacter::canStoreItemInInventoryCustomSlots(ItemTemplate const* itemTemplate) const
{
	PickupStatus status = PICKUP_STATUS_INVENTORY_CUSTOM_SLOTS_FULL;
	int32 pos = SLOT_INVALID;
	for (int32 i = INVENTORY_SLOT_CUSTOM_START; i < INVENTORY_SLOT_CUSTOM_END; ++i)
	{
		CarriedItem* item = this->findCarriedItem(this->getData()->getItem(i));
		if (item)
		{
			if (item->getData()->getItemId() == itemTemplate->id)
			{
				status = item->canBeMergedPartlyWith(itemTemplate);
				if (status == PICKUP_STATUS_OK)
					pos = i;
				else
					pos = SLOT_INVALID;
				break;
			}
		}
		else
		{
			if (pos == SLOT_INVALID)
				pos = i;
		}
	}

	if (pos != SLOT_INVALID)
		return PICKUP_STATUS_OK;

	return status;
}

PickupStatus MyCharacter::canStoreStackableItemInSpecificSlot(int32 slot, ItemTemplate const* itemTemplate) const
{
	NS_ASSERT(itemTemplate->stackable != ITEM_STACK_NON_STACKABLE);

	PickupStatus status;
	CarriedItem* existingItem = this->findCarriedItem(this->getData()->getItem(slot));
	if (!existingItem)
		status = PICKUP_STATUS_OK;
	else
		status = existingItem->canBeMergedPartlyWith(itemTemplate);

	return status;
}

PickupStatus MyCharacter::canStoreItemInEquipmentSlot(ItemTemplate const* itemTemplate) const
{
	PickupStatus status = PICKUP_STATUS_FORBIDDEN;
	int32 equipSlot = SLOT_INVALID;
	switch (itemTemplate->itemSubClass)
	{
	case ITEM_SUBCLASS_HAT:
		equipSlot = EQUIPMENT_SLOT_HAT;
		break;
	case ITEM_SUBCLASS_JACKET:
		equipSlot = EQUIPMENT_SLOT_JACKET;
		break;
	case ITEM_SUBCLASS_GLOVES:
		equipSlot = EQUIPMENT_SLOT_GLOVES;
		break;
	case ITEM_SUBCLASS_SNOWBALL_MAKER:
		equipSlot = EQUIPMENT_SLOT_SNOWBALL_MAKER;
		break;
	case ITEM_SUBCLASS_SHOES:
		equipSlot = EQUIPMENT_SLOT_SHOES;
		break;
	default:
		break;
	}

	if (equipSlot != SLOT_INVALID)
	{
		CarriedItem* existingItem = this->findCarriedItem(this->getData()->getItem(equipSlot));
		if (!existingItem)
			status = PICKUP_STATUS_OK;
		else
		{
			if (existingItem->getData()->getItemId() == itemTemplate->id)
				status = PICKUP_STATUS_ITEM_IS_EQUIPPED;
			else if (itemTemplate->level > 0 && existingItem->getData()->getLevel() > itemTemplate->level)
				status = PICKUP_STATUS_LEVEL_LOWER_THAN_EXISTING_EQUIP;
			else
				status = PICKUP_STATUS_OK;
		}
	}

	return status;
}


void MyCharacter::updateItemAvailabilitiesOnMap()
{
	if (!m_isItemAvailabilitiesChanged)
		return;

	auto const& objects = m_map->getObjects();
	for (auto it = objects.begin(); it != objects.end(); ++it)
	{
		Item* item = (*it).second->asItem();
		if(item)
			item->updateAvailability();
	}

	m_isItemAvailabilitiesChanged = false;
}

void MyCharacter::activateCarriedItem(ObjectGuid const& guid, int32 slot)
{
	CarriedItem* item = nullptr;
	auto it = m_carriedItems.find(guid);
	if (it == m_carriedItems.end())
	{
		item = this->removeIfExistsInInactiveCarriedItems(guid);
		if (item)
			m_carriedItems[guid] = item;
	}
	else
		item = (*it).second;
	if (item)
	{
		if (item->isInWorld() && !item->isActivated())
		{
			item->getData()->setSlot(slot);
			item->activate();
			this->notifyCarriedItemActivated(item);
			m_isItemAvailabilitiesChanged = true;
		}
	}
}

void MyCharacter::activateCarriedItemsInSlots()
{
	for (int32 i = 0; i < UNIT_SLOTS_COUNT; ++i)
	{
		ObjectGuid const& guid = this->getData()->getItem(i);
		if(!guid.isEmpty())
			this->activateCarriedItem(guid, i);
	}
}

void MyCharacter::removeAllCarriedItems()
{
	for (auto it = m_carriedItems.begin(); it != m_carriedItems.end(); )
	{
		CarriedItem* item = (*it).second;
		item->removeFromWorld();

		it = m_carriedItems.erase(it);
		CC_SAFE_DELETE(item);
	}

	for (auto it = m_inactiveCarriedItems.begin(); it != m_inactiveCarriedItems.end();)
	{
		CarriedItem* item = (*it).second;
		item->removeFromWorld();

		it = m_inactiveCarriedItems.erase(it);
		CC_SAFE_DELETE(item);
	}
}

void MyCharacter::inactivateAllCarriedItems()
{
	for (auto it = m_carriedItems.begin(); it != m_carriedItems.end(); )
	{
		CarriedItem* item = (*it).second;
		if (!item->isActivated())
		{
			++it;
			continue;
		}

		this->notifyCarriedItemInactivated(item);
		item->inactivate();
		item->getData()->setSlot(SLOT_INVALID);

		m_inactiveCarriedItems[item->getData()->getGuid()] = item;
		it = m_carriedItems.erase(it);
	}
}

void MyCharacter::updateCarriedItems(float delta)
{
	for (auto it = m_carriedItems.begin(); it != m_carriedItems.end(); ++it)
	{
		CarriedItem* item = (*it).second;
		item->update(delta);

		if (item->getData()->hasUpdatedField(CCARRIEDITEM_FIELD_COUNT))
			m_isItemAvailabilitiesChanged = true;
	}

	for (int32 i = 0; i < UNIT_SLOTS_COUNT; ++i)
	{
		if (this->getData()->hasUpdatedField(CPLAYER_FIELD_ITEM_HEAD + i))
		{
			ObjectGuid const& guid = this->getData()->getItem(i);
			if (!guid.isEmpty())
				this->activateCarriedItem(guid, i);
		}
	}
}

void MyCharacter::clearInactiveCarriedItems()
{
	for (auto it = m_inactiveCarriedItems.begin(); it != m_inactiveCarriedItems.end();)
	{
		CarriedItem* item = (*it).second;
		if (item->canRemoveFromWorld())
		{
			this->notifyCarriedItemDestroyed(item);
			item->removeFromWorld();

			it = m_inactiveCarriedItems.erase(it);
			CC_SAFE_DELETE(item);
		}
		else
			++it;
	}
}

CarriedItem* MyCharacter::removeIfExistsInInactiveCarriedItems(ObjectGuid const& guid)
{
	CarriedItem* item = nullptr;
	auto it = m_inactiveCarriedItems.find(guid);
	if (it != m_inactiveCarriedItems.end())
	{
		item = (*it).second;
		m_inactiveCarriedItems.erase(it);
	}
	return item;
}

void MyCharacter::notifyCarriedItemDestroyed(CarriedItem* item)
{
	int32 slot = item->getData()->getSlot();
	NS_ASSERT(slot != SLOT_INVALID);
	if (slot >= INVENTORY_SLOT_START && slot < INVENTORY_SLOT_END)
	{
		if (m_inventoryItemLifecycleListener)
			m_inventoryItemLifecycleListener->onInventoryItemDestroyed(item->getData()->getGuid());
	}
	else if (slot >= EQUIPMENT_SLOT_START && slot < EQUIPMENT_SLOT_END)
	{
		if (m_equipmentItemLifecycleListener)
			m_equipmentItemLifecycleListener->onEquipmentItemDestroyed(item->getData()->getGuid());
	}
}

void MyCharacter::notifyCarriedItemActivated(CarriedItem* item)
{
	int32 slot = item->getData()->getSlot();
	NS_ASSERT(slot != SLOT_INVALID);
	if (slot >= INVENTORY_SLOT_START && slot < INVENTORY_SLOT_END)
	{
		if (m_inventoryItemLifecycleListener)
			m_inventoryItemLifecycleListener->onInventoryItemActivated(item->getData());
	}
	else if (slot >= EQUIPMENT_SLOT_START && slot < EQUIPMENT_SLOT_END)
	{
		if (m_equipmentItemLifecycleListener)
			m_equipmentItemLifecycleListener->onEquipmentItemActivated(item->getData());
	}
}

void MyCharacter::notifyCarriedItemInactivated(CarriedItem* item)
{
	int32 slot = item->getData()->getSlot();
	NS_ASSERT(slot != SLOT_INVALID);
	if (slot >= INVENTORY_SLOT_START && slot < INVENTORY_SLOT_END)
	{
		if (m_inventoryItemLifecycleListener)
			m_inventoryItemLifecycleListener->onInventoryItemInactivated(item->getData()->getGuid());
	}
	else if (slot >= EQUIPMENT_SLOT_START && slot < EQUIPMENT_SLOT_END)
	{
		if (m_equipmentItemLifecycleListener)
			m_equipmentItemLifecycleListener->onEquipmentItemInactivated(item->getData()->getGuid());
	}
}

void MyCharacter::sendMoveTurn()
{
	if (!this->getSession())
		return;

	MovementInfo movement;
	this->buildMovementInfo(movement);
	WorldPacket packet(world::MSG_MOVE_TURN, std::move(movement));
	this->getSession()->sendPacket(std::move(packet));
}

void MyCharacter::sendMoveSync()
{
	if (!this->getSession())
		return;

	MovementInfo movement;
	this->buildMovementInfo(movement);
	WorldPacket packet(world::MSG_MOVE_SYNC, std::move(movement));
	this->getSession()->sendPacket(std::move(packet));
}

PickupStatus MyCharacter::canStoreItem(ItemTemplate const* itemTemplate) const
{
	PickupStatus status = PICKUP_STATUS_FORBIDDEN;
	switch (itemTemplate->itemClass)
	{
	case ITEM_CLASS_CONSUMABLE:
		if (itemTemplate->itemSubClass == ITEM_SUBCLASS_FIRST_AID)
			status = this->canStoreStackableItemInSpecificSlot(INVENTORY_SLOT_FIRST_AID, itemTemplate);
		else if (itemTemplate->itemSubClass == ITEM_SUBCLASS_CONSUMABLE_OTHER)
			status = this->canStoreItemInInventoryCustomSlots(itemTemplate);
		break;
	case ITEM_CLASS_EQUIPMENT:
		status = this->canStoreItemInEquipmentSlot(itemTemplate);
		break;
	case ITEM_CLASS_GOLD:
	case ITEM_CLASS_MAGIC_BEAN:
		status = PICKUP_STATUS_OK;
		break;
	}

	return status;
}

bool MyCharacter::moveBy(float direction)
{
	if (!this->isAlive())
		return false;

	if (m_isMoveEnabled)
	{
		MyMovementGenerator* generator = this->getMovementGenerator();
		generator->moveBy(direction);
		return true;
	}
	else
	{
		this->setMoveTurnAngle(direction);
		return false;
	}
}

void MyCharacter::moveStop()
{
	this->stopMoving();
}

void MyCharacter::setMoveEnabled(bool enabled)
{
	if (m_isMoveEnabled != enabled)
	{
		if (!enabled)
			this->stopMoving();

		m_isMoveEnabled = enabled;
	}
}

void MyCharacter::stopMoving()
{
	MyMovementGenerator* movementGenerator = getMovementGenerator();
	if (movementGenerator)
		movementGenerator->finish();
}

void MyCharacter::resetMoveSyncState()
{
    MyMoveSpline* moveSpline = this->getMoveSpline();
    if(moveSpline)
        moveSpline->resetSyncState();
}

void MyCharacter::setMoveTurnAngle(float rad)
{
	if (this->getData()->expectFacingToAngle(rad))
		this->sendMoveTurn();
}

void MyCharacter::buildMovementInfo(MovementInfo& movement) const
{
	movement.guid = this->getData()->getGuid();
	movement.counter = this->getData()->getMovementCounter();
	movement.position = this->getData()->getPosition();
	movement.flags = this->getData()->getMovementFlags();
	movement.orientation = this->getData()->getOrientation();
	movement.time = time_util::getUptimeMillis();
}

void MyCharacter::lockFacingDirection()
{
	this->getMoveSpline()->setMoveTurnEnabled(false);
	m_facingDirectionUnlockTimer.reset();
	m_isFacingDirectionLocked = true;
}

void MyCharacter::unlockFacingDirection()
{
	this->getMoveSpline()->setMoveTurnEnabled(true);
	m_isFacingDirectionLocked = false;
	m_facingDirectionUnlockTimer.reset();
}

bool MyCharacter::canHandDown() const
{
	if (this->getData()->hasMovementFlag(MOVEMENT_FLAG_HANDUP)
		&& !this->getStaminaUpdater()->isInCharge()
		&& !this->isAttackTimerEnabled())
	{
		return true;
	}

	return false;
}

void MyCharacter::startHandDownTimer()
{
	NS_ASSERT(this->getData()->hasMovementFlag(MOVEMENT_FLAG_HANDUP));

	m_handDownTimer.setDuration(HAND_DOWN_DELAY);
	m_isHandDownTimerEnabled = true;
}

void MyCharacter::stopHandDownTimer()
{
	m_handDownTimer.reset();
	m_isHandDownTimerEnabled = false;
}

bool MyCharacter::attack(float direction)
{
	if (!this->isAlive())
		return false;

	if (m_isAttackTimerEnabled && !m_attackTimer.passed())
		return false;

	// Can you attack
	if (!this->getStaminaUpdater()->canAttack())
	{
		if (this->getStaminaUpdater()->isInCharge())
			this->chargeStop();

		return false;
	}

	this->addUnitState(UNIT_STATE_ATTACKING);

	DataPlayer* myChar = this->getData();
	uint32 attackFlags = ATTACK_FLAG_NONE;
	if (direction != FLT_MAX)
	{
		if (!this->getStaminaUpdater()->isInCharge())
			attackFlags = ATTACK_FLAG_ALL_OUT;
	}
	else
	{
		Point targetPos;
		if (this->getClosestTargetPosition(targetPos))
			direction = MathTools::computeAngleInRadians(myChar->getPosition(), targetPos);
		else
			direction = this->getData()->getOrientation();
	}

	if (this->getStaminaUpdater()->isInCharge())
	{
		float delay = ATTACK_DELAY;
		// The charge has started
		if (this->getStaminaUpdater()->getChargeStartTime() > 0)
		{
			int32 elapsed = time_util::getUptimeMillis() - this->getStaminaUpdater()->getChargeStartTime();
			if (elapsed < ATTACK_DELAY)
				delay = ATTACK_DELAY - elapsed;
			else
				delay = 0.f;
		}

		// Is it need to delay the attack
		if (delay > 0)
			this->attackDelayed(direction, attackFlags, delay);
		else
		{
			this->getStaminaUpdater()->prepareForChargedAttack();
			this->sendAttackInfo(direction, attackFlags);
		}
	}
	else
	{
		if (!m_isHandDownTimerEnabled)
		{
			if (!this->getData()->hasMovementFlag(MOVEMENT_FLAG_HANDUP))
			{
				this->getData()->addMovementFlag(MOVEMENT_FLAG_HANDUP);
				this->sendMoveSync();
			}
		}
		else
			this->stopHandDownTimer();

		NS_ASSERT(this->getData()->hasMovementFlag(MOVEMENT_FLAG_HANDUP));

		this->attackDelayed(direction, attackFlags, ATTACK_DELAY);
	}

	return true;
}

bool MyCharacter::attackStop()
{
	this->stopAttackTimer();
	this->getData()->clearMovementFlag(MOVEMENT_FLAG_HANDUP);

	m_attackInfoCounter = 0;

	return Unit::attackStop();
}

void MyCharacter::combatStop()
{
	this->chargeStop();

    Unit::combatStop();
}

bool MyCharacter::getClosestTargetPosition(Point& position)
{
	auto const& objects = this->getMap()->getObjects();
	std::multimap<float, WorldObject*, std::less<float>> orderedTargets;

	for (auto p : objects)
	{
		WorldObject* obj = p.second;
		if (obj == this)
			continue;

		WorldObject* target = nullptr;
		if (Unit* unit = obj->asUnit())
		{
			if (unit->isAlive())
				target = unit;
		}
		else if (ItemBox* itemBox = obj->asItemBox())
		{
			if (itemBox->isLocked())
				target = itemBox;
		}

		if (target)
		{
			float dx = target->getData()->getPosition().x - this->getData()->getPosition().x;
			float dy = target->getData()->getPosition().y - this->getData()->getPosition().y;
			float d = dx * dx + dy * dy;
			orderedTargets.emplace(d, target);
		}
	}

	auto it = orderedTargets.begin();
	if (it != orderedTargets.end())
	{
		WorldObject* obj = (*it).second;
		position = obj->getData()->getPosition();
		if (Unit* unit = obj->asUnit())
		{
			UnitMoveSpline* moveSpline = static_cast<UnitMoveSpline*>(unit->getMoveSpline());
			MoveSegment<Unit>* segment = moveSpline->getMoveSegment();
			if (segment)
			{
				float d = segment->getEndPosition().getDistance(unit->getData()->getPosition());
				d = d / 2;
				float rad = MathTools::computeAngleInRadians(unit->getData()->getPosition(), segment->getEndPosition());
				float dx = std::cos(rad) * d;
				float dy = std::sin(rad) * d;
				position = unit->getData()->getPosition() + Vec2(dx, dy);
			}
		}
		return true;
	}
	return false;
}

DataBasic* MyCharacter::loadData(ObjectGuid const& guid)
{
	NS_ASSERT(this->getSession() != nullptr);

	DataPlayer* data = new DataPlayer();
	data->autorelease();
	data->setGuid(guid);
	data->setSelf(true);

	data->setObjectSize(UNIT_OBJECT_SIZE);
	data->setAnchorPoint(UNIT_ANCHOR_POINT);
	data->setObjectRadiusInMap(UNIT_OBJECT_RADIUS_IN_MAP);
	data->setLaunchCenter(UNIT_LAUNCH_CENTER);
	data->setLaunchRadiusInMap(UNIT_LAUNCH_RADIUS_IN_MAP);
	data->setViewport(this->getSession()->getWorld()->getPlayerConfig().winSize);

	this->setData(data);
	this->getSession()->setMyCharacter(this);

	auto const& playerConfig = this->getSession()->getWorld()->getPlayerConfig();
	this->setMoveEnabled(playerConfig.isMoveEnabled);

	m_movementGenerator = new MyMovementGenerator(this);
	m_moveSpline = new MyMoveSpline(this);
	m_staminaUpdater = new MyStaminaUpdater(this);

	return data;
}

void MyCharacter::updateObject(UpdateType updateType, uint32 updateFlags, DataBasic* data)
{
	Unit::updateObject(updateType, updateFlags, data);

	//DataPlayer* dPlayer = data->asDataPlayer();

	if (updateType == UPDATE_TYPE_CREATE)
	{
		this->resetMoveSyncState();
		this->resetStaminaSyncState();
		if(this->isAlive())
			this->startRegenStamina();

		this->getMap()->activateObject(this);
	}
	else if (updateType == UPDATE_TYPE_VALUES)
	{
	}
}

void MyCharacter::update(float delta)
{
	Unit::update(delta);

	if(!this->isInWorld())
		return;

	this->updateCarriedItems(delta);
	this->clearInactiveCarriedItems();

	if (this->isAlive())
	{
		m_itemCooldownProcesser->update(delta);
		this->updateAttackTimer(delta);
		this->updateHandDownTimer(delta);
		this->updateFacingDirectionUnlockTimer(delta);
		this->updateItemAvailabilitiesOnMap();
	}
}

NS_END
