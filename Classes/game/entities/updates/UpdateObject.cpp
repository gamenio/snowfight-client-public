#include "UpdateObject.h"

#include "game/entities/DataBasic.h"

NS_BEGIN

UpdateObject::UpdateObject()
{

}

UpdateObject::~UpdateObject()
{

}

bool UpdateObject::readUpdateCreate(UpdateType updateType, DataInputStream* input)
{
	uint32 updateFlags;
	CHECK_READ((Parcel::readUInt32(input, &updateFlags)));

	uint32 _guid;
	CHECK_READ((Parcel::readUInt32(input, &_guid)));

	int _typeId;
	CHECK_READ((Parcel::readEnum(input, &_typeId)));
	DataTypeID typeId = static_cast<DataTypeID>(_typeId);

	DataBasic* data = nullptr;
	void* holder = nullptr;
	if (this->onUpdateCreate)
		data = this->onUpdateCreate(updateFlags, ObjectGuid(_guid), typeId, &holder);

	if (data)
	{
		data->clearFields();
		FieldUpdateMask updateMask;
		updateMask.setCount(data->getReadFieldCount());
		CHECK_READ(updateMask.readMask(input));
		CHECK_READ(data->readFields(updateType, updateFlags, updateMask, input));

		if(this->onObjectCreateFinished)
			this->onObjectCreateFinished(updateFlags, data, holder);
	}
	else
		return false;

	return true;
}


bool UpdateObject::readUpdateValues(UpdateType updateType, DataInputStream* input)
{
	uint32 updateFlags;
	CHECK_READ((Parcel::readUInt32(input, &updateFlags)));

	uint32 _guid;
	CHECK_READ((Parcel::readUInt32(input, &_guid)));

	DataBasic* data = nullptr;
	void* holder = nullptr;
	if(this->onUpdateValues)
		data = this->onUpdateValues(updateFlags, ObjectGuid(_guid), &holder);

	if (data)
	{
		FieldUpdateMask updateMask;
		updateMask.setCount(data->getReadFieldCount());
		CHECK_READ(updateMask.readMask(input));
		CHECK_READ(data->readFields(updateType, updateFlags, updateMask, input));

		if(this->onValuesUpdateFinished)
			this->onValuesUpdateFinished(updateFlags, data, holder);
	}
	else
		return false;

	return true;
}


bool UpdateObject::readOutOfRangeGUIDs(DataInputStream* input)
{
	uint32 count = 0;
	CHECK_READ(Parcel::readUInt32(input, &count));

	std::vector<ObjectGuid> guids;
	for (uint32 i = 0; i < count; ++i)
	{
		uint32 _guid;
		CHECK_READ((Parcel::readUInt32(input, &_guid)));

		guids.emplace_back(_guid);
	}

	if(this->onUpdateOutOfRange)
		this->onUpdateOutOfRange(guids);

	return true;
}

bool UpdateObject::readFromStream(DataInputStream* input)
{
	uint32 blockCount;
	CHECK_READ((Parcel::readUInt32(input, &blockCount)));

	for (uint32 i = 0; i < blockCount; ++i)
	{
		int _updateType;
		CHECK_READ((Parcel::readEnum(input, &_updateType)));
		UpdateType updateType = static_cast<UpdateType>(_updateType);

		switch (updateType)
		{
		case UPDATE_TYPE_OUT_OF_RANGE_OBJECTS:
			CHECK_READ(this->readOutOfRangeGUIDs(input));
			break;
		case UPDATE_TYPE_CREATE:
			CHECK_READ(this->readUpdateCreate(updateType, input));
			break;
		case UPDATE_TYPE_VALUES:
			CHECK_READ(this->readUpdateValues(updateType, input));
			break;
		}
	}

	return true;
}

NS_END


