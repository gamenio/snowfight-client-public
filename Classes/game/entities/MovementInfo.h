#ifndef __MOVEMENTINFO_H__
#define __MOVEMENTINFO_H__

#include "common/Common.h"
#include "Parcel.h"
#include "DataTypes.h"

NS_BEGIN

class MovementInfo : public Parcel
{
public:
	MovementInfo();
	~MovementInfo();

	MovementInfo(MovementInfo const& right);
	MovementInfo& operator=(MovementInfo const& right);

	std::string GetTypeName() const override { return "MovementInfo"; }
	void Clear() override;

	virtual size_t sizeInBytes() const override;
	bool readFromStream(DataInputStream* input) override;
	virtual void writeToStream(DataOutputStream* output) const override;

	std::string description() const;

	ObjectGuid guid;
	uint32 counter;
	uint32 flags;
	cocos2d::Point position;
	float orientation;
	int32 time;

private:
	void copyFrom(MovementInfo const& right);
};


NS_END

#endif //__MOVEMENTINFO_H__


