#include "CountryCell.h"

#include "scene/Utils.h"

NS_BEGIN

USING_NS_CC_EXT;

#define CELL_MARGIN_LEFTRIGHT		10

#define FRAMENAME_BG_HIGHLIGHTED				"search_cell_bg_highlighted.png"
#define FRAMENAME_DIVIDER						"divider.png"
#define FRAMENAME_TICK							"search_cell_tick.png"

CountryCell::CountryCell() :
	m_highlightBg(nullptr),
	m_nameLabel(nullptr),
	m_dividerSp(nullptr),
	m_tickSp(nullptr)
{
}

CountryCell::~CountryCell()
{
	m_highlightBg = nullptr;
	m_nameLabel = nullptr;
	m_dividerSp = nullptr;
	m_tickSp = nullptr;
}

CountryCell* CountryCell::create(Size const& size)
{
	CountryCell* ret = new (std::nothrow) CountryCell();
	if (ret && ret->init(size))
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

bool CountryCell::init(Size const& size)
{
	if (!TableViewCell::init())
		return false;

	m_dividerSp = Sprite::createWithSpriteFrameName(FRAMENAME_DIVIDER);
	m_dividerSp->setPosition(Vec2(size.width / 2, 0));
	m_dividerSp->setAnchorPoint(Point::ANCHOR_MIDDLE_BOTTOM);
	m_dividerSp->setContentSize(Size(size.width - CELL_MARGIN_LEFTRIGHT * 2, 1));
	this->addChild(m_dividerSp);

	m_highlightBg = ui::Scale9Sprite::createWithSpriteFrameName(FRAMENAME_BG_HIGHLIGHTED);
	m_highlightBg->setPosition(Point::ZERO);
	m_highlightBg->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
	m_highlightBg->setContentSize(Size(size.width, size.height + 1));
	m_highlightBg->setVisible(false);
	this->addChild(m_highlightBg);

	float nameLabelX = CELL_MARGIN_LEFTRIGHT;
	m_nameLabel = Label::createWithSystemFont("", DEFAULT_SYSTEM_FONT, 11);
	m_nameLabel->setTextColor(Color4B(150, 129, 56, 255));
	m_nameLabel->setPosition(Vec2(nameLabelX, size.height / 2));
	m_nameLabel->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
	Utils::enableBoldForLabel(m_nameLabel);
	m_nameLabel->enableWrap(false);
	m_nameLabel->setWidth(size.width - nameLabelX - CELL_MARGIN_LEFTRIGHT);
	this->addChild(m_nameLabel);

	m_tickSp = Sprite::createWithSpriteFrameName(FRAMENAME_TICK);
	m_tickSp->setPosition(Vec2(size.width - CELL_MARGIN_LEFTRIGHT - 3, size.height / 2));
	m_tickSp->setAnchorPoint(Point::ANCHOR_MIDDLE_RIGHT);
	m_tickSp->setVisible(false);
	this->addChild(m_tickSp);

	this->setContentSize(size);

	return true;
}

void CountryCell::setData(CountryInfo const& data)
{
	m_data = data;
	m_nameLabel->setString(data.name);
}

void CountryCell::setHighlighted(bool highlight)
{
	m_highlightBg->setVisible(highlight);
}

void CountryCell::setSelected(bool selected)
{
	m_tickSp->setVisible(selected);
}

void CountryCell::onEnter()
{
	TableViewCell::onEnter();
}


NS_END

