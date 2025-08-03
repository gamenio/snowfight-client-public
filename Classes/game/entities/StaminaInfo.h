#ifndef __STAMINAINFO_H__
#define __STAMINAINFO_H__

#include "common/Common.h"
#include "Parcel.h"
#include "DataTypes.h"

NS_BEGIN

class StaminaInfo: public Parcel
{
public:
	StaminaInfo();
	~StaminaInfo();

	StaminaInfo(StaminaInfo const& right);
	StaminaInfo& operator=(StaminaInfo const& right);

	std::string GetTypeName() const override { return "StaminaInfo"; }
	void Clear() override;

	virtual size_t sizeInBytes() const override;
	bool readFromStream(DataInputStream* input) override;
	virtual void writeToStream(DataOutputStream* output) const override;

	std::string description() const;

	ObjectGuid guid;
	uint32 counter;
	uint32 flags;
	int32 stamina;
	int32 maxStamina;
	float staminaRegenRate;
	int32 chargeStartStamina;
	int32 chargedStamina;
	int32 chargeConsumesStamina;
	uint32 chargeCounter;
	int32 time;
	uint32 attackCounter;
	uint32 consumedStaminaTotal;
	uint32 attackInfoCounter;

private:
	void copyFrom(StaminaInfo const& right);
};

NS_END

#endif // __STAMINAINFO_H__


