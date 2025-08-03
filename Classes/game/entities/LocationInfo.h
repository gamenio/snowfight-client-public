#ifndef __LOCATION_INFO_H__
#define __LOCATION_INFO_H__

#include "common/Common.h"
#include "Parcel.h"
#include "DataTypes.h"

NS_BEGIN

class LocationInfo : public Parcel
{
public:
	LocationInfo();
	~LocationInfo();

	LocationInfo(LocationInfo const& right);
	LocationInfo& operator=(LocationInfo const& right);

	std::string GetTypeName() const override { return "LocationInfo"; }
	void Clear() override;

	virtual size_t sizeInBytes() const override;
	bool readFromStream(DataInputStream* input) override;
	virtual void writeToStream(DataOutputStream* output) const override;

	std::string description() const;

	ObjectGuid guid;
	cocos2d::Point position;
	int32 time;

private:
	void copyFrom(LocationInfo const& right);
};


NS_END

#endif // __LOCATION_INFO_H__


