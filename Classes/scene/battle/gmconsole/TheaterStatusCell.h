#ifndef __THEATER_STATUS_CELL_H__
#define __THEATER_STATUS_CELL_H__

#include "game/client/protocol/pb/TheaterStatusList.pb.h"

#include "extensions/cocos-ext.h"

#include "common/Common.h"

USING_NS_CC;
USING_NS_CC_EXT;

NS_BEGIN


struct TheaterStatusField
{
	char const* title;
	float proportion;
};


enum TheaterStatusColumn
{
	THEATERSTATUS_ID,
	THEATERSTATUS_UPDATE_DIFF,
	THEATERSTATUS_MAP_ID,
	THEATERSTATUS_POPULATION_CAP,
	THEATERSTATUS_PLAYER_COUNT,
	THEATERSTATUS_ROBOT_COUNT,
	THEATERSTATUS_ITEM_COUNT,
	THEATERSTATUS_COMBAT_GRADE,
	MAX_THEATERSTATUS_COLUMNS,
};
extern TheaterStatusField sTheaterStatusFields[MAX_THEATERSTATUS_COLUMNS];

class TheaterStatusCell : public TableViewCell
{
public:
	TheaterStatusCell();
	~TheaterStatusCell();

	static TheaterStatusCell* create(cocos2d::Size const& size);
	bool init(cocos2d::Size const& size);

	void setData(TheaterStatus const& data);
	TheaterStatus const& getData() const { return m_data; }

	virtual void onEnter() override;

	void setHighlighted(bool highlight);

private:
	TheaterStatus m_data;
	ui::Scale9Sprite* m_highlightBg;
};

NS_END

#endif // __THEATER_STATUS_CELL_H__