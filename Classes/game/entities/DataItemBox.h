#ifndef __DATA_ITEM_BOX_H__
#define __DATA_ITEM_BOX_H__

#include "common/Common.h"
#include "updates/ObjectUpdateFields.h"
#include "DataWorldObject.h"

NS_BEGIN

class DataItemBox: public DataWorldObject
{
public:
	enum Direction
	{
		RIGHT_DOWN,
		LEFT_DOWN,
		MAX_DIRECTIONS
	};

	DataItemBox();
	virtual ~DataItemBox();
    
	void clearFields() override;
	uint32 getReadFieldCount() const override { return SITEMBOX_END; }
    bool readFields(UpdateType updateType, uint32 updateFlags, FieldUpdateMask const& updateMask, DataInputStream* input) override;

	cocos2d::Point const& getLaunchCenter() const { return m_launchCenter; }
	void setLaunchCenter(cocos2d::Point const& center) { m_launchCenter = center; }

	cocos2d::Point const& getPosition() const override { return m_position; }
	uint8 getDirection() const { return m_direction; }

	int32 getHealth() const { return m_health; }
	int32 getMaxHealth() const { return m_maxHealth; }

	bool isLocked() const { return m_isLocked; }
	void setLocked(bool isLocked);
 
private:
	cocos2d::Point m_launchCenter;
	cocos2d::Point m_position;
	uint8 m_direction;

	int32 m_health;
	int32 m_maxHealth;
	bool m_isLocked;
};


NS_END

#endif // __DATA_ITEM_BOX_H__
