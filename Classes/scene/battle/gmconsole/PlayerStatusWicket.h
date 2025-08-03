#ifndef __PLAYER_STATUS_WICKET_H__
#define __PLAYER_STATUS_WICKET_H__

#include "extensions/cocos-ext.h"

#include "common/Common.h"
#include "common/utils/Timer.h"
#include "game/WorldListeners.h"
#include "scene/gui/Wicket.h"

USING_NS_CC;

NS_BEGIN

class PlayerStatusWicket : public Wicket, extension::TableViewDataSource, 
										  extension::TableViewDelegate, 
										  WorldStatusListener
{
public:
	static PlayerStatusWicket* create(Node* owner, uint32 theaterId);

	virtual bool init(Node* owner, uint32 theaterId);

	PlayerStatusWicket();
	~PlayerStatusWicket();

	virtual void onEnter() override;
	virtual void update(float delta) override;

	// TableViewDataSource
	virtual extension::TableViewCell* tableCellAtIndex(extension::TableView* table, ssize_t idx) override;
	virtual ssize_t numberOfCellsInTableView(extension::TableView* table) override;
	cocos2d::Size tableCellSizeForIndex(extension::TableView* table, ssize_t idx) override;

	// TableViewDelegate
	virtual void tableCellTouched(extension::TableView* table, extension::TableViewCell* cell) override;
	virtual void scrollViewDidScroll(extension::ScrollView* view) override;

private:
	void refresh();
	void onPlayerStatusList(PlayerStatusList const& statusList) override;
	void buttonRefreshCallback(Ref* sender);

	Node* m_main;
	extension::TableView* m_tableView;
	Node* m_fieldBar;

	std::vector<PlayerStatus> m_dataSet;
	uint32 m_theaterId;
	bool m_isRefreshing;
};


NS_END

#endif // __PLAYER_STATUS_WICKET_H__
