#ifndef __ITEM_BOX_H__
#define __ITEM_BOX_H__

#include "game/entities/DataItemBox.h"
#include "WorldObject.h"

NS_BEGIN

enum OpenState
{
	OPEN_STATE_LOCKED,
	OPEN_STATE_OPENED,
};

class ItemBox : public WorldObject
{
public:
    ItemBox();
    virtual ~ItemBox();

	void activate() override;
	void inactivate() override;

	bool isLocked() const { return m_openState == OPEN_STATE_LOCKED; }
	void setOpenState(OpenState state);
    
	void updateObject(UpdateType updateType, uint32 updateFlags, DataBasic* data) override;
    DataBasic* loadData(ObjectGuid const& guid);
	DataItemBox* getData() const override { return static_cast<DataItemBox*>(m_data); }

private:
	OpenState m_openState;
};


NS_END


#endif // __ITEM_BOX_H__
