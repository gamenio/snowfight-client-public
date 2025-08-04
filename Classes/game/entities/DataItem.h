#ifndef __DATA_ITEM_H__
#define __DATA_ITEM_H__

#include "common/Common.h"
#include "game/utils/TrajectoryGenerator.h"
#include "updates/ObjectUpdateFields.h"
#include "DataWorldObject.h"

NS_BEGIN

#define ITEM_PARABOLA_DURATION					500		// Duration of parabola motion of an item. Unit: milliseconds

// The number of items that can be stacked
#define ITEM_STACK_UNLIMITED					-1		// Unlimited
#define ITEM_STACK_NON_STACKABLE				0		// Non-stackable

enum PickupStatus
{
	PICKUP_STATUS_OK,
	PICKUP_STATUS_ITEM_CANT_STACK,
	PICKUP_STATUS_ITEM_STACK_LIMIT_EXCEEDED,
	PICKUP_STATUS_INVENTORY_CUSTOM_SLOTS_FULL,
	PICKUP_STATUS_ITEM_IS_EQUIPPED,
	PICKUP_STATUS_LEVEL_LOWER_THAN_EXISTING_EQUIP,
	PICKUP_STATUS_FORBIDDEN,
};

enum ItemClass
{
	ITEM_CLASS_MAGIC_BEAN						= 0,
	ITEM_CLASS_EQUIPMENT						= 1,
	ITEM_CLASS_CONSUMABLE						= 2,
	ITEM_CLASS_GOLD								= 3,
};

enum ItemSubClassEquipment
{
	ITEM_SUBCLASS_HAT							= 0,
	ITEM_SUBCLASS_JACKET						= 1,
	ITEM_SUBCLASS_GLOVES						= 2,
	ITEM_SUBCLASS_SNOWBALL_MAKER				= 3,
	ITEM_SUBCLASS_SHOES							= 4,
};

enum ItemSubClassConsumable
{
	ITEM_SUBCLASS_FIRST_AID						= 0,
	ITEM_SUBCLASS_CONSUMABLE_OTHER				= 1,
};

#define ITEM_APPLICATION_NONE					0
#define ITEM_EFFECT_NONE						0
#define ITEM_STAT_NONE							0

enum ItemVisualID
{
	ITEM_VISUAL_NONE							= 0,
	ITEM_VISUAL_STAMINA_CYCLIC_ROLLING			= 1,
	ITEM_VISUAL_SHIELD							= 2,
	ITEM_VISUAL_HIGHLIGHT_HIDING_SPOTS			= 3,
	ITEM_VISUAL_ATTACK_STICK_CHARGE_ENABLE		= 4,
	ITEM_VISUAL_HEALING							= 5,
};

struct ItemStat
{
	uint32 type;
	int32 value;
};

#define MAX_ITEM_STATS				2

struct ItemLocale
{
	std::string name;
	std::string description;
};

struct ItemTemplate
{
	uint32 id;
	uint32 displayId;
	uint32 itemClass;
	uint32 itemSubClass;
	int32 stackable;
	uint8 level;
	uint32 appId;
	std::vector<ItemStat> itemStats;
};

struct ItemEffect
{
	uint32 type;
	int32 value;
};

#define MAX_ITEM_EFFECTS			2

struct ItemApplicationTemplate
{
	uint32 id;
	uint32 flags;
	uint32 visualId;
	int32 duration;	
	int32 recoveryTime;
	std::vector<ItemEffect> effects;
};

class DataItem: public DataWorldObject
{
public:
	DataItem();
	virtual ~DataItem();
    
	void clearFields() override;
	uint32 getReadFieldCount() const override { return SITEM_END; }
    bool readFields(UpdateType updateType, uint32 updateFlags, FieldUpdateMask const& updateMask, DataInputStream* input) override;

	uint32 getItemId() const { return m_itemId; }
	cocos2d::Point const& getPosition() const override { return m_position; }
	int32 getCount() const { return m_count; }

	ObjectGuid const& getHolder() const { return m_holder; }
	cocos2d::Point const& getHolderOrigin() const { return m_holderOrigin; }
	void setHolderOrigin(cocos2d::Point origin) { m_holderOrigin = origin; }

	cocos2d::Point const& getLaunchCenter() const { return m_launchCenter; }
	void setLaunchCenter(cocos2d::Point const& center) { m_launchCenter = center; }
	float getLaunchRadiusInMap() const { return m_launchRadiusInMap; };
	void setLaunchRadiusInMap(float radius) { m_launchRadiusInMap = radius; }

	// Drop duration. Unit: milliseconds
	int32 getDropDuration() const { return m_dropDuration; }
	// Drop elapsed time. Unit: milliseconds
	int32 getDropElapsed() const { return m_dropElapsed; }

	void setTrajectory(BezierCurveConfig const& trajectory) { m_trajectory = trajectory; }
	BezierCurveConfig const& getTrajectory() const { return m_trajectory; }

	void setAvailable(bool isAvailable);
	bool isAvailable() const { return m_isAvailable; }
 
private:
	uint32 m_itemId;
	cocos2d::Point m_position;
	int32 m_count;

	ObjectGuid m_holder;
	cocos2d::Point m_holderOrigin;
	cocos2d::Point m_launchCenter;
	float m_launchRadiusInMap;

	int32 m_dropDuration;
	int32 m_dropElapsed;

	BezierCurveConfig m_trajectory;

	bool m_isAvailable;
};


NS_END

#endif // __DATA_ITEM_H__
