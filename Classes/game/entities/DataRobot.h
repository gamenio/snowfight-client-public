#ifndef __DATA_ROBOT_H__
#define __DATA_ROBOT_H__

#include "common/Common.h"
#include "updates/ObjectUpdateFields.h"
#include "DataUnit.h"

NS_BEGIN

// AI动作类型。
enum AIActionType
{
	AI_ACTION_TYPE_NONE,
	AI_ACTION_TYPE_EXPLORE,
	AI_ACTION_TYPE_SEEK,
	AI_ACTION_TYPE_UNLOCK,
	AI_ACTION_TYPE_COLLECT,
	AI_ACTION_TYPE_COMBAT,
	AI_ACTION_TYPE_UNLOCK_DIRECTLY,
	AI_ACTION_TYPE_HIDE,
	AI_ACTION_TYPE_COLLECT_DIRECTLY,
};

enum CombatState
{
	COMBAT_STATE_CHASE,
	COMBAT_STATE_ESCAPE,
};

enum UnlockState
{
	UNLOCK_STATE_CHASE,
	UNLOCK_STATE_GO_BACK,
};

enum CollectionState
{
	COLLECTION_STATE_COLLECT,
	COLLECTION_STATE_COMBAT,
};

class DataRobot: public DataUnit
{
public:
	DataRobot();
	virtual ~DataRobot();

	void clearFields() override;
	uint32 getReadFieldCount() const override { return SROBOT_END; }
	bool readFields(UpdateType updateType, uint32 updateFlags, FieldUpdateMask const& updateMask, DataInputStream* input) override;

	ObjectGuid const& getTarget() const { return m_target; }

	// AI动作类型
	uint32 getAIActionType() const { return m_aiActionType; }
	std::string getAIActionTypeName() const;
	// AI动作状态
	uint32 getAIActionState() const { return m_aiActionState; }
	std::string getAIActionStateName() const;

private:
	ObjectGuid m_target;
	uint32 m_aiActionType;
	uint32 m_aiActionState;
};


NS_END

#endif // __DATA_ROBOT_H__
