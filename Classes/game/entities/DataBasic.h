#ifndef __DATA_OBJECT_H__
#define __DATA_OBJECT_H__


#include "common/Common.h"
#include "updates/Updatable.h"
#include "updates/UpdateObject.h"
#include "Parcel.h"
#include "DataTypes.h"

NS_BEGIN

class Object;

class DataPlayer;
class DataRobot;
class DataUnit;
class DataItemBox;
class DataItem;
class DataProjectile;
class DataLocatorObject;
class DataUnitLocator;

class DataBasic: public cocos2d::Ref, public Updatable
{
public:
	DataBasic();
	virtual ~DataBasic() = 0;

	bool isType(uint16 mask) const { return (m_type & mask) != 0; }
	DataTypeID getTypeID() const { return m_typeId; }
	void setTypeID(DataTypeID typeId) { m_typeId = typeId; }

    DataPlayer* asDataPlayer() { if (isType(DATA_TYPEMASK_PLAYER)) return reinterpret_cast<DataPlayer*>(this); else return nullptr; }
    DataRobot* asDataRobot() { if (isType(DATA_TYPEMASK_ROBOT)) return reinterpret_cast<DataRobot*>(this); else return nullptr; }
    DataUnit* asDataUnit() { if (isType(DATA_TYPEMASK_UNIT)) return reinterpret_cast<DataUnit*>(this); else return nullptr; }
	DataItemBox* asDataItemBox() { if (isType(DATA_TYPEMASK_ITEMBOX)) return reinterpret_cast<DataItemBox*>(this); else return nullptr; }
	DataItem* asDataItem() { if (isType(DATA_TYPEMASK_ITEM)) return reinterpret_cast<DataItem*>(this); else return nullptr; }
	DataProjectile* asDataProjectile() { if (isType(DATA_TYPEMASK_PROJECTILE)) return reinterpret_cast<DataProjectile*>(this); else return nullptr; }
	DataLocatorObject* asObjectLocator() { if (isType(DATA_TYPEMASK_LOCATOR_OBJECT)) return reinterpret_cast<DataLocatorObject*>(this); else return nullptr; }
	DataUnitLocator* asUnitLocator() { if (isType(DATA_TYPEMASK_UNIT_LOCATOR)) return reinterpret_cast<DataUnitLocator*>(this); else return nullptr; }

	ObjectGuid const& getGuid() const { return m_guid; }
	virtual void setGuid(ObjectGuid const& guid) { m_guid = guid; }

	virtual void clearFields() = 0;
	virtual uint32 getReadFieldCount() const = 0;
	virtual bool readFields(UpdateType updateType, uint32 updateFlags, FieldUpdateMask const& updateMask, DataInputStream* input) = 0;

protected:
	DataTypeID m_typeId;
	uint16 m_type;
	ObjectGuid m_guid;
};

NS_END

#endif //__DATA_OBJECT_H__