#include "PlayerStatusCell.h"

#include "game/GameConfig.h"
#if USE_DEBUG_OPTION

#include "game/entities/ObjectGuid.h"
#include "game/LocaleMgr.h"
#include "scene/Utils.h"

NS_BEGIN

#define FIELD_MARGIN_LEFT			    1
#define FIELD_ROW_MARGIN_LEFTRIGHT		10

PlayerStatusField sPlayerStatusFields[PLAYERSTATUS_COLUMN_COUNT] = {
	{ "ID",								0.2f	},
	{ "Name",							0.25f	},
	{ "Attacks",						0.1f	},
	{ "Latency",						0.1f	},
	{ "Viewport",						0.15f	},
	{ "Lang",							0.1f	},
	{ "Country",						0.1f	}
};

PlayerStatusCell::PlayerStatusCell()
{
}

PlayerStatusCell::~PlayerStatusCell()
{
}


PlayerStatusCell* PlayerStatusCell::create(Size const& size)
{
	PlayerStatusCell *ret = new (std::nothrow) PlayerStatusCell();
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

bool PlayerStatusCell::init(Size const& size)
{
	if (!TableViewCell::init())
		return false;

	float rowWidth = size.width - FIELD_ROW_MARGIN_LEFTRIGHT * 2;
	float rowHeight = size.height;

	float nextColX = FIELD_ROW_MARGIN_LEFTRIGHT;
	for (int32 i = 0; i < PLAYERSTATUS_COLUMN_COUNT; ++i)
	{
		auto label = Label::createWithSystemFont("", DEFAULT_SYSTEM_FONT, 8);
        label->setTextColor(Color4B(150, 129, 56, 255));
		label->setPosition(Vec2::ZERO);
		label->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
		Utils::enableBoldForLabel(label);
		label->setTag(i);

		float proportion = sPlayerStatusFields[i].proportion;
		float colWidth = rowWidth * proportion;
		label->setPosition(nextColX + FIELD_MARGIN_LEFT, rowHeight / 2);
		nextColX += colWidth;

		this->addChild(label);
	}

	Sprite* divider = Sprite::createWithSpriteFrameName("divider.png");
	divider->setPosition(Point::ZERO);
	divider->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
	divider->setContentSize(Size(size.width, 1));
	this->addChild(divider);

	this->setContentSize(size);

	return true;
}

void PlayerStatusCell::setData(PlayerStatus const& data)
{
	Label* label = nullptr;

	label = (Label*)this->getChildByTag(PLAYERSTATUS_ID);
	label->setString(StringUtils::format("%d", ObjectGuid(data.guid()).getCounter()));

	label = (Label*)this->getChildByTag(PLAYERSTATUS_NAME);
	if(data.gm_level() > 0)
		label->setString(StringUtils::format("(*)%s", data.name().c_str()));
	else
		label->setString(data.name());

	label = (Label*)this->getChildByTag(PLAYERSTATUS_ATTACKS);
	label->setString(StringUtils::format("%d", data.attack_total()));

	label = (Label*)this->getChildByTag(PLAYERSTATUS_LATENCY);
	label->setString(StringUtils::format("%dms", data.latency()));

	label = (Label*)this->getChildByTag(PLAYERSTATUS_VIEWPORT);
	label->setString(StringUtils::format("%dX%d", (int32)data.viewport_width(), (int32)data.viewport_height()));

	label = (Label*)this->getChildByTag(PLAYERSTATUS_LANG);
	label->setString(sLocaleMgr->getLangTagByType(static_cast<LangType>(data.lang())));

	label = (Label*)this->getChildByTag(PLAYERSTATUS_COUNTRY);
	label->setString(data.country());
}

void PlayerStatusCell::onEnter()
{
	TableViewCell::onEnter();

}


NS_END

#endif // USE_DEBUG_OPTION
