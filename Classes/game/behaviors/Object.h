#ifndef __OBJECT_H__
#define __OBJECT_H__

#include "common/Common.h"
#include "game/entities/updates/UpdateObject.h"

NS_BEGIN

enum TypeID
{
	TYPEID_OBJECT           = 0,
	TYPEID_WORLDOBJECT,
	TYPEID_UNIT,
	TYPEID_PLAYER,
	TYPEID_ROBOT,
	TYPEID_ITEMBOX,
	TYPEID_ITEM,
	TYPEID_CARRIED_ITEM,
	TYPEID_MYCHARACTER,
	TYPEID_PROJECTILE,
	TYPEID_LOCATOR_OBJECT,
	TYPEID_UNIT_LOCATOR,
};

enum TypeMask
{
	TYPEMASK_NONE				= 0,
	TYPEMASK_OBJECT				= 1 << 0,
	TYPEMASK_WORLDOBJECT		= 1 << 1,
	TYPEMASK_UNIT				= 1 << 2,
	TYPEMASK_PLAYER				= 1 << 3,
	TYPEMASK_ROBOT				= 1 << 4,
	TYPEMASK_ITEMBOX			= 1 << 5,
	TYPEMASK_ITEM				= 1 << 6,
	TYPEMASK_CARRIED_ITEM		= 1 << 7,
	TYPEMASK_MYCHARACTER		= 1 << 8,
	TYPEMASK_PROJECTILE			= 1 << 9,
	TYPEMASK_LOCATOR_OBJECT		= 1 << 10,
	TYPEMASK_UNIT_LOCATOR		= 1 << 11,
};

class DataBasic;

class Player;
class Robot;
class Unit;
class MyCharacter;
class ItemBox;
class Item;
class CarriedItem;
class Projectile;
class UnitLocator;

class Object
{
public:
	Object();
	virtual ~Object();

	virtual void setData(DataBasic* data);
    virtual DataBasic* getData() { return m_data; }
    virtual DataBasic const* getData() const { return m_data; }
    
	bool isType(uint16 mask) const { return (m_type & mask) != 0; }
	TypeID getTypeID() const { return m_typeId; }
	void setTypeID(TypeID typeId) { m_typeId = typeId; }

	MyCharacter* asMyCharacter() { if (isType(TYPEMASK_MYCHARACTER)) return reinterpret_cast<MyCharacter*>(this); else return nullptr; }
	Player* asPlayer() { if (isType(TYPEMASK_PLAYER)) return reinterpret_cast<Player*>(this); else return nullptr; }
	Robot* asRobot() { if (isType(TYPEMASK_ROBOT)) return reinterpret_cast<Robot*>(this); else return nullptr; }
	Unit* asUnit() { if (isType(TYPEMASK_UNIT)) return reinterpret_cast<Unit*>(this); else return nullptr; }
	ItemBox* asItemBox() { if (isType(TYPEMASK_ITEMBOX)) return reinterpret_cast<ItemBox*>(this); else return nullptr; }
	Item* asItem() { if (isType(TYPEMASK_ITEM)) return reinterpret_cast<Item*>(this); else return nullptr; }
	CarriedItem* asCarriedItem() { if (isType(TYPEMASK_CARRIED_ITEM)) return reinterpret_cast<CarriedItem*>(this); else return nullptr; }
	Projectile* asProjectile() { if (isType(TYPEMASK_PROJECTILE)) return reinterpret_cast<Projectile*>(this); else return nullptr; }
	UnitLocator* asUnitLocator() { if (isType(TYPEMASK_UNIT_LOCATOR)) return reinterpret_cast<UnitLocator*>(this); else return nullptr; }

	virtual void addToWorld();
	virtual void removeFromWorld();
	virtual bool canRemoveFromWorld() const;
	bool isInWorld() const { return m_isInWorld; }

	virtual void activate();
	virtual void inactivate();
	bool isActivated() const { return m_isActivated; }

	virtual void updateObject(UpdateType updateType, uint32 updateFlags, DataBasic* data) {}

protected:
	TypeID m_typeId;
	uint16 m_type;

	DataBasic* m_data;

	bool m_isInWorld;
	bool m_isActivated;
	NSTime m_inactiveTime;
};


NS_END

#endif // __OBJECT_H__