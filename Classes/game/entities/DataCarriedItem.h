#ifndef __DATA_CARRIED_ITEM_H__
#define __DATA_CARRIED_ITEM_H__

#include "common/Common.h"
#include "updates/ObjectUpdateFields.h"
#include "DataBasic.h"
#include "DataItem.h"

NS_BEGIN

enum ItemUseStatus
{
	ITEM_USE_STATUS_OK,
	ITEM_USE_STATUS_FAILED,
};

class DataCarriedItem: public DataBasic
{
public:
	DataCarriedItem();
	virtual ~DataCarriedItem();

	uint32 getReadFieldCount() const override { return SCARRIEDITEM_END; }

	uint32 getItemId() const { return m_itemId; }

	uint8 getLevel() const { return m_level; }
	int32 getCount() const { return m_count; }
	int32 getStackable() const { return m_stackable; }

	void setSlot(int32 slot) { m_slot = slot; }
	int32 getSlot() const { return m_slot; }

	// Item cooldown time. Unit: milliseconds
	int32 getCooldownDuration() const { return m_cooldownDuration; }
    
	void clearFields() override;
    bool readFields(UpdateType updateType, uint32 updateFlags, FieldUpdateMask const& updateMask, DataInputStream* input) override;

private:
	uint32 m_itemId;
	uint8 m_level;
	int32 m_count;
	int32 m_stackable;
	int32 m_slot;
	int32 m_cooldownDuration;
};


NS_END

#endif // __DATA_CARRIED_ITEM_H__
