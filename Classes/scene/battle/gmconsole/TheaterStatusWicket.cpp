#include "TheaterStatusWicket.h"

#include "game/GameConfig.h"
#if USE_DEBUG_OPTION

#include "game/World.h"
#include "TheaterStatusCell.h"
#include "PlayerStatusWicket.h"
#include "scene/Utils.h"


USING_NS_CC_EXT;

NS_BEGIN
#define FRAME_SIZE							Size(440, 270)

#define FIELD_BAR_HEIGHT									15
#define FIELD_BAR_MARGIN_TOP								9
#define FIELD_BAR_MARGIN_LEFTRIGHT							10
#define FIELD_BAR_PADDING_LEFTRIGHT							10

#define CELL_HEIGHT											18


TheaterStatusWicket* TheaterStatusWicket::create(Node* owner)
{
	TheaterStatusWicket *ret = new (std::nothrow) TheaterStatusWicket();
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

bool TheaterStatusWicket::init(Node* owner)
{
	if (!Wicket::init(owner, FRAME_SIZE, "THEATER STATUS", true, true))
		return false;

	World::getInstance()->addWorldStatusListener(this);

	m_main = Node::create();
	this->setContent(m_main);

	this->addNeutralButton("Refresh", CC_CALLBACK_1(TheaterStatusWicket::buttonRefreshCallback, this));

	Size frameSize = m_main->getBoundingBox().size;

	// 字段条
	m_fieldBar = Node::create();
	m_fieldBar->setIgnoreAnchorPointForPosition(false);
	m_fieldBar->setPosition(Vec2(FIELD_BAR_MARGIN_LEFTRIGHT, frameSize.height - FIELD_BAR_MARGIN_TOP));
	m_fieldBar->setAnchorPoint(Point::ANCHOR_TOP_LEFT);
	m_fieldBar->setContentSize(Size(frameSize.width - FIELD_BAR_MARGIN_LEFTRIGHT * 2, FIELD_BAR_HEIGHT));
	m_main->addChild(m_fieldBar);

	// 字段条背景
	ui::Scale9Sprite* fieldBarBg = ui::Scale9Sprite::createWithSpriteFrameName("fieldbar_bg.png");
	fieldBarBg->setPosition(Point::ZERO);
	fieldBarBg->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
	fieldBarBg->setContentSize(m_fieldBar->getContentSize());
	m_fieldBar->addChild(fieldBarBg);

	// 字段容器
	Node* fieldContainer = Node::create();
	fieldContainer->setIgnoreAnchorPointForPosition(false);
	fieldContainer->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
	fieldContainer->setPosition(FIELD_BAR_PADDING_LEFTRIGHT, 0);
	fieldContainer->setContentSize(Size(m_fieldBar->getContentSize().width - FIELD_BAR_PADDING_LEFTRIGHT * 2, m_fieldBar->getContentSize().height));
	m_fieldBar->addChild(fieldContainer);
	float nextColX = 0;
	for (int32 i = 0; i < MAX_THEATERSTATUS_COLUMNS; ++i)
	{
		Label* label = Label::createWithSystemFont(sTheaterStatusFields[i].title, DEFAULT_SYSTEM_FONT, 8);
		label->setAnchorPoint(Point::ANCHOR_MIDDLE_LEFT);
		label->setTextColor(Color4B(255, 249, 236, 255));
		Utils::enableBoldForLabel(label);

		float proportion = sTheaterStatusFields[i].proportion;
		float colWidth = fieldContainer->getContentSize().width * proportion;
		label->setPosition(nextColX, FIELD_BAR_HEIGHT / 2);
		nextColX += colWidth;

		fieldContainer->addChild(label);
	}

	// 记录列表
	m_tableView = TableView::create(this, Size(m_fieldBar->getContentSize().width, m_fieldBar->getBoundingBox().getMinY()));
	m_tableView->setIgnoreAnchorPointForPosition(false);
	m_tableView->setAnchorPoint(Point::ANCHOR_TOP_LEFT);
	m_tableView->setDirection(extension::ScrollView::Direction::VERTICAL);
	m_tableView->setDelegate(this);
	m_tableView->setVerticalFillOrder(TableView::VerticalFillOrder::TOP_DOWN);
	m_tableView->setPosition(Vec2(m_fieldBar->getPosition().x, m_fieldBar->getBoundingBox().getMinY()));
	m_main->addChild(m_tableView);


	this->refresh();

	return true;
}


TheaterStatusWicket::TheaterStatusWicket() :
	m_main(nullptr),
	m_tableView(nullptr),
	m_fieldBar(nullptr),
	m_isRefreshing(false)
{
}

TheaterStatusWicket::~TheaterStatusWicket()
{
	World::getInstance()->removeWorldStatusListener(this);

	m_tableView = nullptr;
	m_fieldBar = nullptr;
}

void TheaterStatusWicket::onEnter()
{
	Wicket::onEnter();

}

void TheaterStatusWicket::update(float delta)
{
}

TableViewCell* TheaterStatusWicket::tableCellAtIndex(TableView* table, ssize_t idx)
{
	TheaterStatusCell* cell = dynamic_cast<TheaterStatusCell*>(table->dequeueCell());
	if (!cell) 
	{
		Size viewSize = table->getViewSize();
		cell = TheaterStatusCell::create(Size(viewSize.width, CELL_HEIGHT));
	}

	TheaterStatus const& data = m_dataSet[idx];
	cell->setData(data);

	return cell;
}

Size TheaterStatusWicket::tableCellSizeForIndex(TableView *table, ssize_t idx)
{
	return Size(0, CELL_HEIGHT);
}

ssize_t TheaterStatusWicket::numberOfCellsInTableView(TableView* table)
{
	return m_dataSet.size();
}

void TheaterStatusWicket::tableCellTouched(TableView* table, TableViewCell* cell)
{
	TheaterStatusCell* touchedCell = dynamic_cast<TheaterStatusCell*>(cell);
	if (!touchedCell)
		return;

	TheaterStatus data = touchedCell->getData();
	PlayerStatusWicket* wicket = PlayerStatusWicket::create(this, data.id());
	wicket->show();

}

void TheaterStatusWicket::tableCellHighlight(TableView* table, TableViewCell* cell)
{
	TheaterStatusCell* theaterStatusCell = dynamic_cast<TheaterStatusCell*>(cell);
	if (theaterStatusCell)
	{
		theaterStatusCell->setHighlighted(true);
	}
}

void TheaterStatusWicket::tableCellUnhighlight(TableView* table, TableViewCell* cell)
{
	TheaterStatusCell* theaterStatusCell = dynamic_cast<TheaterStatusCell*>(cell);
	if (theaterStatusCell)
	{
		theaterStatusCell->setHighlighted(false);
	}
}

void TheaterStatusWicket::scrollViewDidScroll(extension::ScrollView* view)
{

}

void TheaterStatusWicket::refresh()
{
	m_isRefreshing = true;
	World::getInstance()->sendQueryTheaterStatusList();
}

void TheaterStatusWicket::onTheaterStatusList(TheaterStatusList const& statusList)
{
	m_dataSet.clear();

	auto const& theaterList = statusList.result();
	for (auto it = theaterList.begin(); it != theaterList.end(); ++it)
	{
		TheaterStatus const& theater = *it;
		m_dataSet.push_back(theater);
	}

	m_tableView->reloadData();
	m_isRefreshing = false;
}

void TheaterStatusWicket::buttonRefreshCallback(Ref* sender)
{
	this->refresh();
}


NS_END

#endif // USE_DEBUG_OPTION
