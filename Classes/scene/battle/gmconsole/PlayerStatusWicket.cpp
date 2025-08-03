#include "PlayerStatusWicket.h"

#include "game/GameConfig.h"
#if USE_DEBUG_OPTION

#include "game/World.h"
#include "PlayerStatusCell.h"
#include "scene/Utils.h"


USING_NS_CC_EXT;

NS_BEGIN
#define FRAME_SIZE							Size(460, 270)

#define FIELD_BAR_HEIGHT									15
#define FIELD_BAR_MARGIN_TOP								9
#define FIELD_BAR_MARGIN_LEFTRIGHT							10
#define FIELD_BAR_PADDING_LEFTRIGHT							10

#define CELL_HEIGHT											18

#define REFRESH_INTERVAL									5.0f


PlayerStatusWicket* PlayerStatusWicket::create(Node* owner, uint32 theaterId)
{
	PlayerStatusWicket *ret = new (std::nothrow) PlayerStatusWicket();
	if (ret && ret->init(owner, theaterId))
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

bool PlayerStatusWicket::init(Node* owner, uint32 theaterId)
{
	if (!Wicket::init(owner, FRAME_SIZE, "PLAYER STATUS", true, true))
		return false;

	m_theaterId = theaterId;

	World::getInstance()->addWorldStatusListener(this);

	m_main = Node::create();
	this->setContent(m_main);

	this->addNeutralButton("Refresh", CC_CALLBACK_1(PlayerStatusWicket::buttonRefreshCallback, this));

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
	for (int32 i = 0; i < PLAYERSTATUS_COLUMN_COUNT; ++i)
	{
		Label* label = Label::createWithSystemFont(sPlayerStatusFields[i].title, DEFAULT_SYSTEM_FONT, 8);
		label->setAnchorPoint(Point::ANCHOR_MIDDLE_LEFT);
		label->setTextColor(Color4B(255, 249, 236, 255));
		Utils::enableBoldForLabel(label);

		float proportion = sPlayerStatusFields[i].proportion;
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


PlayerStatusWicket::PlayerStatusWicket() :
	m_main(nullptr),
	m_tableView(nullptr),
	m_fieldBar(nullptr),
	m_theaterId(0),
	m_isRefreshing(false)
{
}

PlayerStatusWicket::~PlayerStatusWicket()
{
	World::getInstance()->removeWorldStatusListener(this);

	m_tableView = nullptr;
	m_fieldBar = nullptr;
}

void PlayerStatusWicket::onEnter()
{
	Wicket::onEnter();

}

void PlayerStatusWicket::update(float delta)
{
}

TableViewCell* PlayerStatusWicket::tableCellAtIndex(TableView* table, ssize_t idx)
{
	PlayerStatusCell* cell = dynamic_cast<PlayerStatusCell*>(table->dequeueCell());
	if (!cell) 
	{
		Size viewSize = table->getViewSize();
		cell = PlayerStatusCell::create(Size(viewSize.width, CELL_HEIGHT));
	}

	PlayerStatus const& data = m_dataSet[idx];
	cell->setData(data);

	return cell;
}

Size PlayerStatusWicket::tableCellSizeForIndex(TableView *table, ssize_t idx)
{
	return Size(0, CELL_HEIGHT);
}

ssize_t PlayerStatusWicket::numberOfCellsInTableView(TableView* table)
{
	return m_dataSet.size();
}

void PlayerStatusWicket::tableCellTouched(TableView* table, TableViewCell* cell)
{
}

void PlayerStatusWicket::scrollViewDidScroll(extension::ScrollView* view)
{

}

void PlayerStatusWicket::refresh()
{
	m_isRefreshing = true;
	World::getInstance()->sendQueryPlayerStatusList(m_theaterId);
}

void PlayerStatusWicket::onPlayerStatusList(PlayerStatusList const& statusList)
{
	m_dataSet.clear();

	auto const& playerList = statusList.result();
	for (auto it = playerList.begin(); it != playerList.end(); ++it)
	{
		PlayerStatus const& player = *it;
		m_dataSet.push_back(player);
	}

	m_tableView->reloadData();
	m_isRefreshing = false;
}

void PlayerStatusWicket::buttonRefreshCallback(Ref* sender)
{
	this->refresh();
}


NS_END

#endif // USE_DEBUG_OPTION
