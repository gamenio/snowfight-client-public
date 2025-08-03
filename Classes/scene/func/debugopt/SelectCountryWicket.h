#ifndef __SELECT_COUNTRY_WICKET_H__
#define __SELECT_COUNTRY_WICKET_H__

#include "extensions/cocos-ext.h"

#include "common/Common.h"
#include "scene/gui/Wicket.h"
#include "scene/gui/SwitchButton.h"
#include "scene/gui/OptionButton.h"
#include "scene/gui/InputBox.h"
#include "CountryCell.h"

USING_NS_CC;
using namespace cocos2d::ui;

NS_BEGIN

typedef std::function<void(Ref*, CountryInfo const&)> SelectCountryCallback;

class SelectCountryWicket : public Wicket, extension::TableViewDataSource, extension::TableViewDelegate, EditBoxDelegate
{
public:
	static SelectCountryWicket* create(Node* owner);

	virtual bool init(Node* owner);

	SelectCountryWicket();
	~SelectCountryWicket();

	void onEnter() override;

	// TableViewDataSource
	virtual extension::TableViewCell* tableCellAtIndex(extension::TableView* table, ssize_t idx) override;
	virtual ssize_t numberOfCellsInTableView(extension::TableView* table) override;
	virtual cocos2d::Size tableCellSizeForIndex(extension::TableView* table, ssize_t idx) override;

	// TableViewDelegate
	virtual void tableCellTouched(extension::TableView* table, extension::TableViewCell* cell) override;
	virtual void scrollViewDidScroll(extension::ScrollView* view) override;
	virtual void tableCellHighlight(extension::TableView* table, extension::TableViewCell* cell) override;
	virtual void tableCellUnhighlight(extension::TableView* table, extension::TableViewCell* cell) override;

	// EditBoxDelegate
	void editBoxTextChanged(EditBox* editBox, std::string const& text) override;
	virtual void editBoxReturn(EditBox* editBox) override;

	void setSelectedCountry(std::string const& countryCode, bool animated = false);
	void setSelectEventListener(SelectCountryCallback const& callback);

private:
	void buttonCrossCallback(Ref* sender);
	void buttonTickCallback(Ref* sender);	
	void buttonCleanInputCallback(Ref* sender);

	void setSelectedIndex(int32 index, bool animated, bool contentOffset);
	void setCellSelected(int32 index, bool selected);

	void loadData();

	Node* m_main;
	InputBox* m_searchBox;
	Button* m_cleanInputBtn;
	extension::TableView* m_tableView;
	std::vector<CountryInfo> m_filteredDataSet;
	std::vector<CountryInfo> m_dataSet;
	int32 m_selectedIndex;
	int32 m_selectedCountryID;
	bool m_isFiltering;

	SelectCountryCallback m_selectCallback;
};

NS_END

#endif // __SELECT_COUNTRY_WICKET_H__