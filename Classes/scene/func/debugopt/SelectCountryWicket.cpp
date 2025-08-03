#include "SelectCountryWicket.h"

#include "game/LocaleMgr.h"
#include "scene/AssetsLoader.h"

NS_BEGIN

USING_NS_CC_EXT;


#define FRAME_SIZE						Size(280, 260)

#define SEARCHBOX_MARGIN_TOP			8

#define SEARCHBOX_MARGIN_LEFTRIGHT		9
#define SEARCHBOX_BG_HEIGHT				25
#define INPUTBOX_MARGIN_LEFTRIGHT		27

#define CELL_HEIGHT						26

#define FRAMENAME_SEARCHBOX_BG					"searchbox_bg.png"
#define FRAMENAME_SEARCHBOX_CLEAN				"searchbox_clean.png"

#define SELECTED_INDEX_NONE						-1
#define SELECTED_COUNTRYID_NONE					0


SelectCountryWicket* SelectCountryWicket::create(Node* owner)
{
	SelectCountryWicket* ret = new (std::nothrow) SelectCountryWicket();
	if (ret && ret->init(owner))
	{
		ret->autorelease();
		return ret;
	}
	else
	{
		CC_SAFE_DELETE(ret);
		return nullptr;
	}
}

bool SelectCountryWicket::init(Node* owner)
{
	if (!Wicket::init(owner, FRAME_SIZE, "REGION", false, true, Wicket::BUTTON_ALIGNMENT_AVERAGE))
		return false;

	m_main = Node::create();
	this->setContent(m_main);

	Size frameSize = m_main->getBoundingBox().size;

	this->addTickButton(CC_CALLBACK_1(SelectCountryWicket::buttonTickCallback, this));
	this->addCrossButton(CC_CALLBACK_1(SelectCountryWicket::buttonCrossCallback, this));

	ui::Scale9Sprite* searchBoxBg = ui::Scale9Sprite::createWithSpriteFrameName(FRAMENAME_SEARCHBOX_BG);
	searchBoxBg->setPosition(Vec2(SEARCHBOX_MARGIN_LEFTRIGHT, frameSize.height - SEARCHBOX_MARGIN_TOP));
	searchBoxBg->setContentSize(Size(frameSize.width - SEARCHBOX_MARGIN_LEFTRIGHT * 2, SEARCHBOX_BG_HEIGHT));
	searchBoxBg->setAnchorPoint(Point::ANCHOR_TOP_LEFT);
	m_main->addChild(searchBoxBg);

	m_searchBox = InputBox::create(Size(searchBoxBg->getContentSize().width - INPUTBOX_MARGIN_LEFTRIGHT * 2, searchBoxBg->getContentSize().height));
	m_searchBox->setAnchorPoint(Point::ANCHOR_MIDDLE_LEFT);
	m_searchBox->setPosition(Vec2(searchBoxBg->getPositionX() + INPUTBOX_MARGIN_LEFTRIGHT, searchBoxBg->getPositionY() - searchBoxBg->getContentSize().height / 2));
	m_searchBox->setPlaceholderFontColor(Color4B(186, 166, 99, 255));
	m_searchBox->setPlaceholderFont(DEFAULT_SYSTEM_FONT, 12);
	m_searchBox->setPlaceHolder("Search");
	m_searchBox->setTextHorizontalAlignment(TextHAlignment::LEFT);
	m_searchBox->setFontSize(12);
	m_searchBox->setFontColor(Color3B(150, 129, 56));
	m_searchBox->setReturnType(ui::EditBox::KeyboardReturnType::DONE);
	m_searchBox->setInputMode(ui::EditBox::InputMode::SINGLE_LINE);
	m_searchBox->setDelegate(this);
	m_main->addChild(m_searchBox);

	m_cleanInputBtn = Button::create(FRAMENAME_SEARCHBOX_CLEAN, "", "", Widget::TextureResType::PLIST);
	m_cleanInputBtn->setAnchorPoint(Point::ANCHOR_MIDDLE_RIGHT);
	m_cleanInputBtn->setPosition(Vec2(searchBoxBg->getBoundingBox().getMaxX(), searchBoxBg->getPositionY() - searchBoxBg->getContentSize().height / 2));
	m_cleanInputBtn->addClickEventListener(CC_CALLBACK_1(SelectCountryWicket::buttonCleanInputCallback, this));
	m_cleanInputBtn->setVisible(false);
	m_main->addChild(m_cleanInputBtn);


	float tableViewY = searchBoxBg->getBoundingBox().getMinY() - 2;
	m_tableView = TableView::create(this, Size(frameSize.width, tableViewY));
	m_tableView->setIgnoreAnchorPointForPosition(false);
	m_tableView->setAnchorPoint(Point::ANCHOR_MIDDLE_TOP);
	m_tableView->setDirection(extension::ScrollView::Direction::VERTICAL);
	m_tableView->setDelegate(this);
	m_tableView->setVerticalFillOrder(TableView::VerticalFillOrder::TOP_DOWN);
	m_tableView->setPosition(Vec2(frameSize.width / 2, tableViewY));
	m_main->addChild(m_tableView);

	this->loadData();

	return true;
}

SelectCountryWicket::SelectCountryWicket() :
	m_main(nullptr),
	m_searchBox(nullptr),
	m_cleanInputBtn(nullptr),
	m_tableView(nullptr),
	m_selectedIndex(SELECTED_INDEX_NONE),
	m_selectedCountryID(SELECTED_COUNTRYID_NONE),
	m_isFiltering(false)
{
}

SelectCountryWicket::~SelectCountryWicket()
{
	m_main = nullptr;
	m_searchBox = nullptr;
	m_cleanInputBtn = nullptr;
	m_tableView = nullptr;
}

void SelectCountryWicket::onEnter()
{
	Wicket::onEnter();
}

TableViewCell* SelectCountryWicket::tableCellAtIndex(TableView* table, ssize_t idx)
{
	CountryCell* cell = dynamic_cast<CountryCell*>(table->dequeueCell());
	if (!cell)
	{
		Size viewSize = table->getViewSize();
		cell = CountryCell::create(Size(viewSize.width, CELL_HEIGHT));
	}
	CountryInfo const& data = m_filteredDataSet[idx];
	cell->setData(data);
	cell->setSelected(m_selectedCountryID == data.id);

	// CCLOG("tableCellAtIndex %d", idx);

	return cell;
}

Size SelectCountryWicket::tableCellSizeForIndex(TableView *table, ssize_t idx)
{
	return Size(0, CELL_HEIGHT);
}

ssize_t SelectCountryWicket::numberOfCellsInTableView(TableView* table)
{
	return m_filteredDataSet.size();
}

void SelectCountryWicket::tableCellTouched(TableView* table, TableViewCell* cell)
{
	CountryCell* touchedCell = dynamic_cast<CountryCell*>(cell);
	if (!touchedCell)
		return;

	if (m_isFiltering)
	{
		CountryInfo data = touchedCell->getData();

		m_searchBox->setText("");
		m_cleanInputBtn->setVisible(false);
		m_filteredDataSet = m_dataSet;
		m_tableView->reloadData();

		this->setSelectedCountry(data.code, true);

		m_isFiltering = false;
	}
	else
	{
		this->setSelectedIndex((int32)cell->getIdx(), false, false);
	}
}

void SelectCountryWicket::scrollViewDidScroll(extension::ScrollView* view)
{
	
}

void SelectCountryWicket::tableCellHighlight(TableView* table, TableViewCell* cell)
{
	CountryCell* countryCell = dynamic_cast<CountryCell*>(cell);
	if (countryCell)
	{
		countryCell->setHighlighted(true);
	}
}

void SelectCountryWicket::tableCellUnhighlight(TableView* table, TableViewCell* cell)
{
	CountryCell* countryCell = dynamic_cast<CountryCell*>(cell);
	if (countryCell)
	{
		countryCell->setHighlighted(false);
	}
}

void SelectCountryWicket::setCellSelected(int32 index, bool selected)
{
	CountryCell* cell = dynamic_cast<CountryCell*>(m_tableView->cellAtIndex(index));
	if (cell)
		cell->setSelected(selected);
}

void SelectCountryWicket::setSelectedCountry(std::string const& countryCode, bool animated)
{
	if (m_dataSet.empty())
		return;

	int32 foundIndex = SELECTED_INDEX_NONE;
	int32 count = (int32)m_dataSet.size();
	for (int32 i = 0; i < count; ++i)
	{
		CountryInfo const& info = m_dataSet.at(i);
		if (info.code == countryCode)
		{
			foundIndex = i;
			break;
		}
	}

	if (foundIndex != SELECTED_INDEX_NONE)
		this->setSelectedIndex(foundIndex, animated, true);
}

void SelectCountryWicket::setSelectEventListener(SelectCountryCallback const& callback)
{
	m_selectCallback = callback;
}

void SelectCountryWicket::setSelectedIndex(int32 index, bool animated, bool contentOffset)
{
	if (m_dataSet.empty())
		return;

	if(m_selectedIndex != SELECTED_INDEX_NONE)
		this->setCellSelected(m_selectedIndex, false);
	this->setCellSelected(index, true);

	CountryInfo const& data = m_dataSet.at(index);
	m_selectedCountryID = data.id;
	m_selectedIndex = index;

	if (contentOffset)
	{
		Size containerSize = m_tableView->getContainer()->getContentSize();
		if (!containerSize.equals(Size::ZERO))
		{
			auto viewSize = m_tableView->getViewSize();

			float offsetY = 0;
			float h = (m_selectedIndex + 1) * (float)CELL_HEIGHT;
			if (h > viewSize.height)
				offsetY = -(containerSize.height - h);
			else
				offsetY = -(containerSize.height - viewSize.height);
			m_tableView->setContentOffset(Vec2(0, offsetY), animated);
		}
	}
}

void SelectCountryWicket::buttonCrossCallback(Ref* sender)
{
	this->close();
}

void SelectCountryWicket::buttonTickCallback(Ref* sender)
{
	if (m_selectCallback)
	{
		if (m_selectedIndex != SELECTED_INDEX_NONE)
		{
			CountryInfo const& data = m_dataSet.at(m_selectedIndex);
			m_selectCallback(this, data);
		}
	}

	this->close();
}

void SelectCountryWicket::buttonCleanInputCallback(Ref* sender)
{
	m_searchBox->setText("");
	m_cleanInputBtn->setVisible(false);
	m_filteredDataSet = m_dataSet;
	m_tableView->reloadData();
}

void SelectCountryWicket::loadData()
{
	auto& countries = sLocaleMgr->getCountryList();
	CountryInfo countryNone;
	countryNone.name = "<None>";
	countryNone.code = "";
	countryNone.id = -1;
	m_dataSet.push_back(countryNone);
	m_dataSet.insert(m_dataSet.end(), countries.begin(), countries.end());

	m_filteredDataSet = m_dataSet;

	m_tableView->reloadData();
}


void SelectCountryWicket::editBoxTextChanged(ui::EditBox* editBox, std::string const& text)
{
	if (text.empty())
	{
		m_filteredDataSet = m_dataSet;
		m_cleanInputBtn->setVisible(false);
	}
	else
	{
		m_cleanInputBtn->setVisible(true);
		m_isFiltering = true;
		m_filteredDataSet.clear();

		std::string keyword = text;
		std::transform(keyword.begin(), keyword.end(), keyword.begin(), ::toupper);
		std::copy_if(m_dataSet.begin(), m_dataSet.end(), std::back_inserter(m_filteredDataSet), [keyword](CountryInfo const& info)
		{
			std::string nameToUpper = info.name;
			std::transform(nameToUpper.begin(), nameToUpper.end(), nameToUpper.begin(), ::toupper);

			if (nameToUpper.find(keyword) != std::string::npos
				|| info.code.find(keyword) != std::string::npos)
			{
				return true;
			}

			return false;
		});
	}

	m_tableView->reloadData();
}

void SelectCountryWicket::editBoxReturn(ui::EditBox* editBox)
{
}

NS_END

