#ifndef __UPDATE_OBJECT_H__
#define __UPDATE_OBJECT_H__

#include <functional>

#include "common/Common.h"
#include "game/entities/Parcel.h"
#include "game/entities/DataTypes.h"

NS_BEGIN

enum UpdateType
{
	UPDATE_TYPE_VALUES,
	UPDATE_TYPE_CREATE,
	UPDATE_TYPE_OUT_OF_RANGE_OBJECTS
};

enum UpdateFlag
{
	UPDATE_FLAG_NONE = 0,
	UPDATE_FLAG_SELF = 1 << 0,
};

class DataBasic;

class UpdateObject : public Parcel
{
public:
	UpdateObject();
	~UpdateObject();

	std::string GetTypeName() const override { return "UpdateObject"; }

	virtual size_t sizeInBytes() const override { return 0; }
	bool readFromStream(DataInputStream* input) override;
	void writeToStream(DataOutputStream* output) const override { }

	std::function<DataBasic*(uint32 /* UpdateFlag */, ObjectGuid const&, void** /* Output data holder */)> onUpdateValues;
	std::function<void(uint32 /* UpdateFlag */, DataBasic*, void* /* Data holder */)> onValuesUpdateFinished;
	std::function<DataBasic*(uint32 /* UpdateFlag */, ObjectGuid const&, DataTypeID, void** /* Output data holder */)> onUpdateCreate;
	std::function<void(uint32 /* UpdateFlag */, DataBasic*, void* /* Data holder */)> onObjectCreateFinished;
	std::function<void(std::vector<ObjectGuid> const&)> onUpdateOutOfRange;

private:
	bool readUpdateCreate(UpdateType updateType, DataInputStream* input);
	bool readUpdateValues(UpdateType updateType, DataInputStream* input);
	bool readOutOfRangeGUIDs(DataInputStream* input);

};


NS_END

#endif //__UPDATE_OBJECT_H__