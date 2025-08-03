#ifndef __THEATER_STATUS_WICKET_H__
#define __THEATER_STATUS_WICKET_H__

#include "extensions/cocos-ext.h"

#include "common/Common.h"
#include "common/utils/Timer.h"
#include "scene/gui/Wicket.h"
#include "game/WorldListeners.h"

USING_NS_CC;

NS_BEGIN

class TheaterStatusWicket : public Wicket, extension::TableViewDataSource, 
										   extension::TableViewDelegate, 
										   WorldStatusListener
{
public:
	static TheaterStatusWicket* create(Node* owner);

	virtual bool init(Node* owner);

	TheaterStatusWicket();
	~TheaterStatusWicket();

	virtual void onEnter() override;
	virtual void update(float delta) override;

	// TableViewDataSource
	virtual extension::TableViewCell* tableCellAtIndex(extension::TableView* table, ssize_t idx) override;
	virtual ssize_t numberOfCellsInTableView(extension::TableView* table) override;
	cocos2d::Size tableCellSizeForIndex(extension::TableView* table, ssize_t idx) override;

	// TableViewDelegate
	virtual void tableCellTouched(extension::TableView* table, extension::TableViewCell* cell) override;
	virtual void scrollViewDidScroll(extension::ScrollView* view) override;
	virtual void tableCellHighlight(extension::TableView* table, extension::TableViewCell* cell) override;
	virtual void tableCellUnhighlight(extension::TableView* table, extension::TableViewCell* cell) override;

private:
	void refresh();
	void onTheaterStatusList(TheaterStatusList const& statusList) override;
	void buttonRefreshCallback(Ref* sender);

	Node* m_main;
	extension::TableView* m_tableView;
	Node* m_fieldBar;

	std::vector<TheaterStatus> m_dataSet;
	bool m_isRefreshing;
};


NS_END

#endif // __THEATER_STATUS_WICKET_H__
