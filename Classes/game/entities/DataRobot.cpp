#include "DataRobot.h"

#include "game/LocaleMgr.h"

NS_BEGIN

DataRobot::DataRobot():
	m_target(ObjectGuid::EMPTY),
	m_aiActionType(0),
	m_aiActionState(0)
{
	m_type |= DATA_TYPEMASK_ROBOT;
	m_typeId = DataTypeID::DATA_TYPEID_ROBOT;

	m_updateMask.setCount(CROBOT_END);
}


DataRobot::~DataRobot()
{
}

void DataRobot::clearFields()
{
	m_target.clear();
	m_aiActionType = 0;
	m_aiActionState = 0;

	DataUnit::clearFields();
}

bool DataRobot::readFields(UpdateType updateType, uint32 updateFlags, FieldUpdateMask const& updateMask, DataInputStream* input)
{
	CHECK_READ(DataUnit::readFields(updateType, updateFlags, updateMask, input));

	if (Updatable::isNeedReadField(updateMask, SROBOT_FIELD_TARGET))
	{
		uint32 _guid;
		CHECK_READ((Parcel::readUInt32(input, &_guid)));
		m_target = ObjectGuid(_guid);
		this->setUpdatedField(CROBOT_FIELD_TARGET);
	}

	if (Updatable::isNeedReadField(updateMask, SROBOT_FIELD_AIACTION_TYPE))
	{
		CHECK_READ((Parcel::readUInt32(input, &m_aiActionType)));
		this->setUpdatedField(CROBOT_FIELD_AIACTION_TYPE);
	}

	if (Updatable::isNeedReadField(updateMask, SROBOT_FIELD_AIACTION_STATE))
	{
		CHECK_READ((Parcel::readUInt32(input, &m_aiActionState)));
		this->setUpdatedField(CROBOT_FIELD_AIACTION_STATE);
	}

	return true;
}

std::string DataRobot::getAIActionTypeName() const
{
	std::string name = "None";
	switch (m_aiActionType)
	{
	case AI_ACTION_TYPE_EXPLORE:
		name = "Explore";
		break;
	case AI_ACTION_TYPE_SEEK:
		name = "Seek";
		break;
	case AI_ACTION_TYPE_UNLOCK:
		name = "Unlock";
		break;
	case AI_ACTION_TYPE_COLLECT:
		name = "Collect";
		break;
	case AI_ACTION_TYPE_COMBAT:
		name = "Combat";
		break;
	case AI_ACTION_TYPE_UNLOCK_DIRECTLY:
		name = "UnlockDirectly";
		break;
	case AI_ACTION_TYPE_HIDE:
		name = "Hide";
		break;
	case AI_ACTION_TYPE_COLLECT_DIRECTLY:
		name = "CollectDirectly";
		break;
	}

	return name;
}

std::string DataRobot::getAIActionStateName() const
{
	std::string name = "None";
	switch (m_aiActionType)
	{
	case AI_ACTION_TYPE_COMBAT:
		if (m_aiActionState == COMBAT_STATE_CHASE)
			name = "Chase";
		else
			name = "Escape";
		break;
	case AI_ACTION_TYPE_UNLOCK:
	case AI_ACTION_TYPE_UNLOCK_DIRECTLY:
		if (m_aiActionState == UNLOCK_STATE_CHASE)
			name = "Chase";
		else
			name = "GoBack";
		break;
	case AI_ACTION_TYPE_COLLECT:
	case AI_ACTION_TYPE_COLLECT_DIRECTLY:
		if (m_aiActionState == COLLECTION_STATE_COLLECT)
			name = "Collect";
		else
			name = "Combat";
		break;
	}

	return name;
}

NS_END
