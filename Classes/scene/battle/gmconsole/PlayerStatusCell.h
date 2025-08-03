#ifndef __PLAYER_STATUS_CELL_H__
#define __PLAYER_STATUS_CELL_H__

#include "game/client/protocol/pb/PlayerStatusList.pb.h"

#include "extensions/cocos-ext.h"

#include "common/Common.h"

USING_NS_CC;
USING_NS_CC_EXT;

NS_BEGIN


struct PlayerStatusField
{
	char const* title;
	float proportion;
};


enum PlayerStatusColumn
{
	PLAYERSTATUS_ID,
	PLAYERSTATUS_NAME,
	PLAYERSTATUS_ATTACKS,
	PLAYERSTATUS_LATENCY,
	PLAYERSTATUS_VIEWPORT,
	PLAYERSTATUS_LANG,
	PLAYERSTATUS_COUNTRY,
	PLAYERSTATUS_COLUMN_COUNT
};

extern PlayerStatusField sPlayerStatusFields[PLAYERSTATUS_COLUMN_COUNT];

class PlayerStatusCell : public TableViewCell
{
public:
	PlayerStatusCell();
	~PlayerStatusCell();

	static PlayerStatusCell* create(cocos2d::Size const& size);
	bool init(cocos2d::Size const& size);

	void setData(PlayerStatus const& data);

	virtual void onEnter() override;

private:
};

NS_END

#endif // __PLAYER_STATUS_CELL_H__