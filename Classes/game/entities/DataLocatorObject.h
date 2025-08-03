#ifndef __DATA_LOCATOR_OBJECT_H__
#define __DATA_LOCATOR_OBJECT_H__

#include "DataBasic.h"

USING_NS_CC;

NS_BEGIN

class DataLocatorObject: public DataBasic
{
public:
	DataLocatorObject();
	~DataLocatorObject();

	virtual void clearFields() override {}
	virtual bool readFields(UpdateType updateType, uint32 updateFlags, FieldUpdateMask const& updateMask, DataInputStream* input) override { return true; }

	virtual cocos2d::Point const& getPosition() const = 0;
};

NS_END

#endif // __DATA_LOCATOR_OBJECT_H__