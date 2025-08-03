#ifndef __DATA_UNIT_LOCATOR_H__
#define __DATA_UNIT_LOCATOR_H__

#include "DataLocatorObject.h"
#include "LocationInfo.h"
#include "updates/ObjectUpdateFields.h"

NS_BEGIN

class DataUnitLocator: public DataLocatorObject
{
public:
	DataUnitLocator();
	~DataUnitLocator();

	void clearFields() override;
	uint32 getReadFieldCount() const override { return SUNIT_LOCATOR_END; }
	bool readFields(UpdateType updateType, uint32 updateFlags, FieldUpdateMask const& updateMask, DataInputStream* input) override;

	uint32 getDisplayId() const { return m_displayId; }

	cocos2d::Point const& getPosition() const override { return m_locationInfo.position; }
	void setPosition(cocos2d::Point const& position) { m_locationInfo.position = position; }
	LocationInfo const& getLocationInfo() const { return m_locationInfo; }

	bool isAlive() const { return m_isAlive; }
	int32 getMoveSpeed() const { return m_moveSpeed; }

private:
	LocationInfo m_locationInfo;
	uint32 m_displayId;
	bool m_isAlive;
	int32 m_moveSpeed;
};

NS_END

#endif // __DATA_UNIT_LOCATOR_H__