#ifndef __DATA_PLAYER_H__
#define __DATA_PLAYER_H__

#include "updates/ObjectUpdateFields.h"
#include "DataUnit.h"
#include "UnitSlotDefines.h"

USING_NS_CC;


NS_BEGIN

// Game controller type
enum ControllerType
{
	CONTROLLER_TYPE_DUAL_STICKS			= 0,
	CONTROLLER_TYPE_TAP					= 1, // Deprecated
	CONTROLLER_TYPE_JOYSTICK			= 2, // Deprecated
	CONTROLLER_TYPE_KEYBOARD_MOUSE		= 3,
};

// Game tutorial process
enum TutorialProcess
{
	TUTORIAL_PROCESS_NONE,
	TUTORIAL_PROCESS_CONTROL_MOVE,
	TUTORIAL_PROCESS_CONTROL_ATTACK,
	TUTORIAL_PROCESS_STAMINA_BAR_DESC,
	TUTORIAL_PROCESS_PICKUP_ALL_ITEMS,
	TUTORIAL_PROCESS_EQUIPMENT_DESC,
	TUTORIAL_PROCESS_CONTROL_SUPER_ATTACK,
	TUTORIAL_PROCESS_USE_ITEM,
	TUTORIAL_PROCESS_FIND_ENEMY,
	TUTORIAL_PROCESS_OBJECTIVE,
	TUTORIAL_PROCESS_COLLECT_MAGIC_BEANS,
	TUTORIAL_PROCESS_SHOW_EQUIPMENT_BAR,
	TUTORIAL_PROCESS_SEND_EMO,
	MAX_TUTORIAL_PROCESSES,
};

// Game suggestion
enum Suggestion
{
	SUGGESTION_NONE						= 0,
	SUGGESTION_BACK_HOME				= 1 << 0,
	SUGGESTION_UPGRADE_STATS			= 1 << 1,
	SUGGESTION_WATCH_AD_FIRST			= 1 << 2,
	SUGGESTION_WATCH_AD_REMINDED		= 1 << 3,
	SUGGESTION_CLAIM_REWARD				= 1 << 4,
};

struct StageStat
{
	static const StageStat Null;

	StageStat()
	{
	}

	bool isNull() const { return value.isNull(); }

	cocos2d::Value value;
};

typedef std::array<StageStat, MAX_STAT_TYPES> PlayerStageStats;

struct PlayerTemplate
{
	StageStat const& getStageStat(uint8 stage, StatType type) const
	{
		if (stage < stageStatsList.size())
			return stageStatsList[stage][type];
		else
			return StageStat::Null;
	}

	StageStat const& getNextStageStat(uint8 currStage, StatType type) const
	{
		uint8 newStage = currStage + 1;
		if (newStage < stageStatsList.size())
			return stageStatsList[newStage][type];
		else
			return StageStat::Null;
	}

	int16 getMaxStage() const
	{
		return (int16)(stageStatsList.size() - 1);
	}

	uint32 id;
	std::vector<PlayerStageStats> stageStatsList;
};

class DataPlayer: public DataUnit
{
public:
	DataPlayer();
	virtual ~DataPlayer();

	void setSelf(bool self) { m_isSelf = self; }
	bool isSelf() const { return m_isSelf; }

	void clearFields() override;
	uint32 getReadFieldCount() const override { return SPLAYER_END; }
	bool readFields(UpdateType updateType, uint32 updateFlags, FieldUpdateMask const& updateMask, DataInputStream* input) override;

	void setViewport(cocos2d::Size const& size) { m_viewport = size; }
	cocos2d::Size const& getViewport() const { return m_viewport; }

	// Is it GM
	bool isGM() const { return m_isGM; }

	void setMaxStamina(int32 maxStamina) override;
	void setStaminaRegenRate(float rate) override;

	int32 getMoney() const { return m_money; }

	// Item in the slot
	ObjectGuid const& getItem(int32 slot) const { return m_items[slot]; }

	// Pick up the target
	ObjectGuid const& getPickupTarget() const { return m_pickupTarget; }

protected:
	bool m_isGM;
	cocos2d::Size m_viewport;

	std::array<ObjectGuid, UNIT_SLOTS_COUNT> m_items;
	int32 m_money;
	ObjectGuid m_pickupTarget;

	bool m_isSelf;
};

NS_END

#endif // __DATA_PLAYER_H__

