#include "TheaterStatusCell.h"

#include "game/GameConfig.h"
#if USE_DEBUG_OPTION

#include "scene/Utils.h"

NS_BEGIN

#define FRAMENAME_BG_HIGHLIGHTED				"search_cell_bg_highlighted.png"
#define FRAMENAME_DIVIDER						"divider.png"

#define FIELD_MARGIN_LEFT			    1
#define FIELD_ROW_MARGIN_LEFTRIGHT		10

TheaterStatusField sTheaterStatusFields[MAX_THEATERSTATUS_COLUMNS] = {
	{ "ID",									0.05f	},
	{ "UpdateDiff",							0.13f	},
	{ "MapID",								0.1f	},
	{ "PopulationCap",						0.17f	},
	{ "Players",							0.13f	},
	{ "Bots",								0.13f	},
	{ "Items",								0.13f	},
	{ "CombatGrade",						0.84f	},
};



TheaterStatusCell::TheaterStatusCell() : 
	m_highlightBg(nullptr)
{
}

TheaterStatusCell::~TheaterStatusCell()
{
}


TheaterStatusCell* TheaterStatusCell::create(Size const& size)
{
	TheaterStatusCell *ret = new (std::nothrow) TheaterStatusCell();
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

bool TheaterStatusCell::init(Size const& size)
{
	if (!TableViewCell::init())
		return false;

	Sprite* divider = Sprite::createWithSpriteFrameName(FRAMENAME_DIVIDER);
	divider->setPosition(Point::ZERO);
	divider->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
	divider->setContentSize(Size(size.width, 1));
	this->addChild(divider);

	m_highlightBg = ui::Scale9Sprite::createWithSpriteFrameName(FRAMENAME_BG_HIGHLIGHTED);
	m_highlightBg->setPosition(Point::ZERO);
	m_highlightBg->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
	m_highlightBg->setContentSize(Size(size.width, size.height + 1));
	m_highlightBg->setVisible(false);
	this->addChild(m_highlightBg);

	float rowWidth = size.width - FIELD_ROW_MARGIN_LEFTRIGHT * 2;
	float rowHeight = size.height;

	float nextColX = FIELD_ROW_MARGIN_LEFTRIGHT;
	for (int32 i = 0; i < MAX_THEATERSTATUS_COLUMNS; ++i)
	{
		auto label = Label::createWithSystemFont("", DEFAULT_SYSTEM_FONT, 8);
        label->setTextColor(Color4B(150, 129, 56, 255));
		label->setPosition(Vec2::ZERO);
		label->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
		Utils::enableBoldForLabel(label);
		label->setTag(i);

		float proportion = sTheaterStatusFields[i].proportion;
		float colWidth = rowWidth * proportion;
		label->setPosition(nextColX + FIELD_MARGIN_LEFT, rowHeight / 2);
		nextColX += colWidth;

		this->addChild(label);
	}

	this->setContentSize(size);

	return true;
}

void TheaterStatusCell::setData(TheaterStatus const& data)
{
	m_data = data;

	Label* label = nullptr;

	label = (Label*)this->getChildByTag(THEATERSTATUS_ID);
	label->setString(StringUtils::format("%d", data.id()));

	label = (Label*)this->getChildByTag(THEATERSTATUS_UPDATE_DIFF);
	label->setString(StringUtils::format("%dms", data.update_diff()));

	label = (Label*)this->getChildByTag(THEATERSTATUS_MAP_ID);
	label->setString(StringUtils::format("%d", data.map_id()));

	label = (Label*)this->getChildByTag(THEATERSTATUS_POPULATION_CAP);
	label->setString(StringUtils::format("%d", 0));

	label = (Label*)this->getChildByTag(THEATERSTATUS_PLAYER_COUNT);
	label->setString(StringUtils::format("%d", data.player_count()));

	label = (Label*)this->getChildByTag(THEATERSTATUS_ROBOT_COUNT);
	label->setString(StringUtils::format("%d", data.robot_count()));

	label = (Label*)this->getChildByTag(THEATERSTATUS_ITEM_COUNT);
	label->setString(StringUtils::format("%d", data.item_count()));

	label = (Label*)this->getChildByTag(THEATERSTATUS_COMBAT_GRADE);
	label->setString(StringUtils::format("%d", data.combat_grade()));
}

void TheaterStatusCell::onEnter()
{
	TableViewCell::onEnter();

}

void TheaterStatusCell::setHighlighted(bool highlight)
{
	m_highlightBg->setVisible(highlight);
}


NS_END

#endif // USE_DEBUG_OPTION
