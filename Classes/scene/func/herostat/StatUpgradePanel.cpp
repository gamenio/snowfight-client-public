//
//  StatUpgradePanel.cpp
//  snowfight
//
//  Created by Luthier on 2020/06/23.
//

#include "StatUpgradePanel.h"

#include "common/utils/MathTools.h"
#include "game/utils/UnitHelper.h"
#include "game/gamecenter/GameCenter.h"
#include "game/store/Store.h"
#include "game/UserPreferences.h"
#include "game/LocaleMgr.h"
#include "game/ObjectMgr.h"
#include "scene/Utils.h"


using namespace cocos2d::ui;

NS_BEGIN

#define CONTENT_SIZE	Size(163, 125)

#define ITEM_HEIGHT						31

#define LISTVIEW_EXTENDED_TOUCH_AREA			45
#define LISTVIEW_CLIPPING_PADDING_LEFTRIGHT		5

// Debug drawer
#if NS_DEBUG

#define DEBUG_LISTVIEW					0

#endif // NS_DEBUG

StatUpgradePanel* StatUpgradePanel::create()
{
	auto ret = new (std::nothrow) StatUpgradePanel();
	if (ret && ret->init())
	{
		ret->autorelease();
	}
	else
	{
		CC_SAFE_DELETE(ret);
	}

	return ret;
}

bool StatUpgradePanel::init()
{
	if (!LayerColor::initWithColor(Color4B(0, 0, 0, 0)))
		return false;

	this->setIgnoreAnchorPointForPosition(false);
	this->setContentSize(CONTENT_SIZE);

	m_nameLabel = Label::createWithSystemFont(sLocaleMgr->getString("statupgrade_panel_name"), DEFAULT_SYSTEM_FONT, 15);
	m_nameLabel->setAnchorPoint(Point::ANCHOR_TOP_LEFT);
	m_nameLabel->setTextColor(Color4B(86, 111, 132, 255));
	Utils::enableBoldForLabel(m_nameLabel);
	m_nameLabel->setPosition(6, this->getContentSize().height);
	this->addChild(m_nameLabel);

	float nextY = m_nameLabel->getBoundingBox().getMinY() - 4;

	ClippingNode* clipper = ClippingNode::create();
	clipper->setPosition(Vec2(this->getContentSize().width / 2, nextY));
	clipper->setContentSize(Size(this->getContentSize().width + LISTVIEW_CLIPPING_PADDING_LEFTRIGHT * 2, nextY));
	clipper->setAnchorPoint(Point::ANCHOR_MIDDLE_TOP);
	DrawNode* stencil = DrawNode::create();
	stencil->drawSolidRect(Vec2(0, 0), clipper->getContentSize(), Color4F::BLACK);
	clipper->setStencil(stencil);
	this->addChild(clipper);

	m_listView = ListView::create();
	m_listView->setClippingEnabled(false);
	m_listView->setSwallowTouches(false);
	m_listView->setScrollBarEnabled(false);
	m_listView->setDirection(ui::ScrollView::Direction::VERTICAL);
	m_listView->setBounceEnabled(true);
	m_listView->setContentSize(Size(clipper->getContentSize().width - LISTVIEW_CLIPPING_PADDING_LEFTRIGHT * 2, clipper->getContentSize().height + LISTVIEW_EXTENDED_TOUCH_AREA * 2));
	m_listView->setAnchorPoint(Point::ANCHOR_MIDDLE);
	m_listView->setPosition(clipper->getContentSize() / 2);
	m_listView->setTopPadding(LISTVIEW_EXTENDED_TOUCH_AREA);
	m_listView->setBottomPadding(LISTVIEW_EXTENDED_TOUCH_AREA);
	m_listView->addEventListener((ui::ListView::ccListViewCallback)CC_CALLBACK_2(StatUpgradePanel::selectedItemEvent, this));
	m_listView->addEventListener((ui::ScrollView::ccScrollViewCallback)CC_CALLBACK_2(StatUpgradePanel::scrollListEvent, this));
	clipper->addChild(m_listView);

#if DEBUG_LISTVIEW

	Rect clipperRect = clipper->getBoundingBox();
	Rect listViewRect = m_listView->getBoundingBox();
	DrawNode* debugDraw = DrawNode::create();
	debugDraw->drawRect(Vec2(clipperRect.origin.x + listViewRect.getMinX(), clipperRect.origin.y + listViewRect.getMinY()),
		Vec2(clipperRect.origin.x + listViewRect.getMaxX(), clipperRect.origin.y + listViewRect.getMaxY()), Color4F::GREEN);
	debugDraw->drawRect(clipperRect.origin, clipperRect.origin + clipperRect.size, Color4F::RED);
	this->addChild(debugDraw);

#endif

	this->updateListItems();

	return true;
}

StatUpgradePanel::StatUpgradePanel() :
	m_template(nullptr),
	m_nameLabel(nullptr),
	m_listView(nullptr)

{

}

StatUpgradePanel::~StatUpgradePanel()
{
	m_template = nullptr;
	m_nameLabel = nullptr;
	m_listView = nullptr;
}

StatUpgradeData StatUpgradePanel::createHealthUpgradeData(HeroTemplate const* tmpl, bool isLocked)
{
	PlayerTemplate const* playerTmpl = sObjectMgr->getPlayerTemplate(tmpl->id);
	StatUpgradeData data;

	data.isLocked = isLocked;
	data.type = StatUpgradeType::STAT_UPGRADE_HEALTH;
	data.maxStage = playerTmpl->getMaxStage();

	data.field1.type = STAT_MAX_HEALTH;
	data.field1.name = sLocaleMgr->getString("statupgrade_field_health");
	data.field1.format = StatField::NUMBER;

	data.field2.type = STAT_HEALTH_REGEN_RATE;
	data.field2.name = sLocaleMgr->getString("statupgrade_field_regen_rate");
	data.field2.format = StatField::PERCENTAGE;

	this->updateHealthUpgradeData(data, tmpl);

	return data;
}

void StatUpgradePanel::updateHealthUpgradeData(StatUpgradeData& data, HeroTemplate const* tmpl)
{
	LocalPlayer* localPlayer = sGameCenter->getLocalPlayer();
	PlayerTemplate const* playerTmpl = sObjectMgr->getPlayerTemplate(tmpl->id);

	data.stage = localPlayer->getStatStage(tmpl->id, STAT_MAX_HEALTH);
	data.price = tmpl->getStatUpgradePrice(MIN(data.maxStage, data.stage + 1), STAT_MAX_HEALTH);

	Value diffVal, currVal, nextVal;

	currVal = playerTmpl->getStageStat(data.stage, STAT_MAX_HEALTH).value;
	nextVal = playerTmpl->getStageStat(MIN(data.maxStage, data.stage + 1), STAT_MAX_HEALTH).value;
	diffVal = nextVal.asInt() - currVal.asInt();
	data.field1.value = diffVal;

	uint8 stage = localPlayer->getStatStage(tmpl->id, STAT_HEALTH_REGEN_RATE);
	currVal = MathTools::roundFloatWithPrecision(playerTmpl->getStageStat(stage, STAT_HEALTH_REGEN_RATE).value.asFloat() / 60, 4); // In seconds
	nextVal = MathTools::roundFloatWithPrecision(playerTmpl->getStageStat(MIN(data.maxStage, stage + 1), STAT_HEALTH_REGEN_RATE).value.asFloat() / 60, 4); // In seconds
	diffVal = nextVal.asFloat() - currVal.asFloat();
	if(diffVal.asFloat() > 0)
		data.field2.value = diffVal;
}

StatUpgradeData StatUpgradePanel::createDamageUpgradeData(HeroTemplate const* tmpl, bool isLocked)
{
	PlayerTemplate const* playerTmpl = sObjectMgr->getPlayerTemplate(tmpl->id);
	StatUpgradeData data;

	data.type = StatUpgradeType::STAT_UPGRADE_DAMAGE;
	data.isLocked = isLocked;
	data.maxStage = playerTmpl->getMaxStage();

	data.field1.type = STAT_DAMAGE;  
	data.field1.name = sLocaleMgr->getString("statupgrade_field_damage");
	data.field1.format = StatField::NUMBER;

	this->updateDamageUpgradeData(data, tmpl);

	return data;
}

void StatUpgradePanel::updateDamageUpgradeData(StatUpgradeData& data, HeroTemplate const* tmpl)
{
	LocalPlayer* localPlayer = sGameCenter->getLocalPlayer();
	data.stage = localPlayer->getStatStage(tmpl->id, STAT_DAMAGE);
	data.price = tmpl->getStatUpgradePrice(MIN(data.maxStage, data.stage + 1), STAT_DAMAGE);

	Value diffVal, currVal, nextVal;

	PlayerTemplate const* playerTmpl = sObjectMgr->getPlayerTemplate(tmpl->id);
	currVal = playerTmpl->getStageStat(data.stage, STAT_DAMAGE).value;
	nextVal = playerTmpl->getStageStat(MIN(data.maxStage, data.stage + 1), STAT_DAMAGE).value;
	diffVal = nextVal.asInt() - currVal.asInt();
	data.field1.value = diffVal;
}

StatUpgradeData StatUpgradePanel::createMoveSpeedUpgradeData(HeroTemplate const* tmpl, bool isLocked)
{
	PlayerTemplate const* playerTmpl = sObjectMgr->getPlayerTemplate(tmpl->id);
	StatUpgradeData data;

	data.type = StatUpgradeType::STAT_UPGRADE_MOVE_SPEED;
	data.isLocked = isLocked;
	data.maxStage = playerTmpl->getMaxStage();

	data.field1.type = STAT_MOVE_SPEED; 
	data.field1.name = sLocaleMgr->getString("statupgrade_field_move_speed"); 
	data.field1.format = StatField::NUMBER;

	this->updateMoveSpeedUpgradeData(data, tmpl);

	return data;
}

void StatUpgradePanel::updateMoveSpeedUpgradeData(StatUpgradeData& data, HeroTemplate const* tmpl)
{
	LocalPlayer* localPlayer = sGameCenter->getLocalPlayer();
	data.stage = localPlayer->getStatStage(tmpl->id, STAT_MOVE_SPEED);
	data.price = tmpl->getStatUpgradePrice(MIN(data.maxStage, data.stage + 1), STAT_MOVE_SPEED);

	Value diffVal, currVal, nextVal;

	PlayerTemplate const* playerTmpl = sObjectMgr->getPlayerTemplate(tmpl->id);
	currVal = playerTmpl->getStageStat(data.stage, STAT_MOVE_SPEED).value;
	nextVal = playerTmpl->getStageStat(MIN(data.maxStage, data.stage + 1), STAT_MOVE_SPEED).value;
	diffVal = nextVal.asInt() - currVal.asInt();
	data.field1.value = diffVal;
}

StatUpgradeData StatUpgradePanel::createAttackRangeUpgradeData(HeroTemplate const* tmpl, bool isLocked)
{
	PlayerTemplate const* playerTmpl = sObjectMgr->getPlayerTemplate(tmpl->id);
	StatUpgradeData data;

	data.type = StatUpgradeType::STAT_UPGRADE_ATTACK_RANGE;
	data.isLocked = isLocked;
	data.maxStage = playerTmpl->getMaxStage();

	data.field1.type = STAT_ATTACK_RANGE; 
	data.field1.name = sLocaleMgr->getString("statupgrade_field_attack_range"); 
	data.field1.format = StatField::NUMBER;

	this->updateAttackRangeUpgradeData(data, tmpl);

	return data;
}

void StatUpgradePanel::updateAttackRangeUpgradeData(StatUpgradeData& data, HeroTemplate const* tmpl)
{
	LocalPlayer* localPlayer = sGameCenter->getLocalPlayer();
	data.stage = localPlayer->getStatStage(tmpl->id, STAT_ATTACK_RANGE);
	data.price = tmpl->getStatUpgradePrice(MIN(data.maxStage, data.stage + 1), STAT_ATTACK_RANGE);

	Value diffVal, currVal, nextVal;

	PlayerTemplate const* playerTmpl = sObjectMgr->getPlayerTemplate(tmpl->id);
	currVal = (int32)playerTmpl->getStageStat(data.stage, STAT_ATTACK_RANGE).value.asFloat();
	nextVal = (int32)playerTmpl->getStageStat(MIN(data.maxStage, data.stage + 1), STAT_ATTACK_RANGE).value.asFloat();
	diffVal = nextVal.asInt() - currVal.asInt();
	data.field1.value = diffVal;
}

void StatUpgradePanel::setLocalPlayerStatStage(StatUpgradeData const& data, uint8 stage)
{
	LocalPlayer* localPlayer = sGameCenter->getLocalPlayer();
	localPlayer->setStatStage(m_template->id, data.field1.type, stage);
	if (!data.field2.isNull())
		localPlayer->setStatStage(m_template->id, data.field2.type, stage);

}

void StatUpgradePanel::updateStatUpgradeData(StatUpgradeData& data)
{
	switch (data.type)
	{
	case StatUpgradeType::STAT_UPGRADE_HEALTH:
		this->updateHealthUpgradeData(data, m_template);
		break;
	case StatUpgradeType::STAT_UPGRADE_DAMAGE:
		this->updateDamageUpgradeData(data, m_template);
		break;
	case StatUpgradeType::STAT_UPGRADE_MOVE_SPEED:
		this->updateMoveSpeedUpgradeData(data, m_template);
		break;
	case StatUpgradeType::STAT_UPGRADE_ATTACK_RANGE:
		this->updateAttackRangeUpgradeData(data, m_template);
		break;
	default:
		NS_ASSERT_LOG(false, "Unsupported upgrade type.");
		break;
	}

}

void StatUpgradePanel::upgradeStat(StatUpgradeItem* item, StatUpgradeData& data)
{
	uint8 nextStage = data.stage + 1;
	if (nextStage > data.maxStage)
		return;

	LocalPlayer* localPlayer = sGameCenter->getLocalPlayer();
	int32 price = data.price;

	if (localPlayer->getProperty() >= price)
	{
		localPlayer->removeMoney(price);
		this->setLocalPlayerStatStage(data, nextStage);
		localPlayer->saveDataAsync();

		this->updateStatUpgradeData(data);

		if (m_recommendedUpgradeTypes.test(data.type))
		{
			data.isRecommended = false;
			m_recommendedUpgradeTypes.reset(data.type);
		}

		item->setData(data);

		if (m_statUpgradedCallback)
			m_statUpgradedCallback(this, UPGRADE_STATE_UPGRADED, data);
	}
	else
	{
		if (m_statUpgradedCallback)
			m_statUpgradedCallback(this, UPGRADE_STATE_NOT_ENOUGH_GOLDS, data);
	}
}

void StatUpgradePanel::setHeroTemplate(HeroTemplate const* tmpl)
{
	m_template = tmpl;

	this->updatePanel();
}

void StatUpgradePanel::updatePanel()
{
	this->reloadData();
	m_listView->jumpToTop();
}

ui::Button* StatUpgradePanel::getUpgradeButton(StatUpgradeType type) const
{
	Vector<Widget*>& items = m_listView->getItems();
	for (auto it = items.begin(); it != items.end(); ++it)
	{
		StatUpgradeItem* item = dynamic_cast<StatUpgradeItem*>(*it);
		if (item->getData().type == type)
		{
			return item->getUpgradeButton();
		}
	}

	return nullptr;
}

void StatUpgradePanel::stopAnimation()
{
	Vector<Widget*>& items = m_listView->getItems();
	for (auto it = items.begin(); it != items.end(); ++it)
	{
		StatUpgradeItem* item = dynamic_cast<StatUpgradeItem*>(*it);
		if(item)
			item->stopAnimation();
	}
}

void StatUpgradePanel::reloadData()
{
	m_dataSet.clear();

	bool isLocked = !sGameCenter->getLocalPlayer()->hasUnlockedHero(m_template->id);

	m_dataSet.emplace_back(this->createHealthUpgradeData(m_template, isLocked));
	m_dataSet.emplace_back(this->createDamageUpgradeData(m_template, isLocked));
	if(this->canUpgradeStat(m_template, STAT_MOVE_SPEED))
		m_dataSet.emplace_back(this->createMoveSpeedUpgradeData(m_template, isLocked));
	if (this->canUpgradeStat(m_template, STAT_ATTACK_RANGE))
		m_dataSet.emplace_back(this->createAttackRangeUpgradeData(m_template, isLocked));

	for (auto& data : m_dataSet)
	{
		if (m_recommendedUpgradeTypes.test(data.type))
			data.isRecommended = true;
	}

	this->updateListItems();
}

bool StatUpgradePanel::canUpgradeStat(HeroTemplate const* tmpl, StatType type) const
{
	bool ret = tmpl->getNextStageUpgradePrice(STAT_STAGE_MIN, type) > 0;
	return ret;
}


void StatUpgradePanel::updateListItems()
{
	Vector<Widget*>& items = m_listView->getItems();

	int32 nData = (int32)m_dataSet.size();
	int32 nItems = (int32)items.size();
	for (int32 i = 0; i < nData || i < nItems; ++i)
	{
		StatUpgradeItem* item = nullptr;
		if (i < nItems)
			item = dynamic_cast<StatUpgradeItem*>(items.at(i));

		if (i < nData)
		{
			if (!item)
			{
				item = dynamic_cast<StatUpgradeItem*>(this->dequeueItem());
				if (!item)
					item = StatUpgradeItem::create(Size(m_listView->getContentSize().width, ITEM_HEIGHT));

				m_listView->pushBackCustomItem(item);
			}
			else
				item->reset();
			StatUpgradeData const& data = m_dataSet[i];
			item->setData(data);
			item->setLastItem(i == nData - 1);
		}
		else
		{
			item->reset();
			m_freedItems.pushBack(item);
			m_listView->removeItem(i);
			nItems = (int32)items.size();
			i--;
		}
	}
}

Widget* StatUpgradePanel::dequeueItem()
{
	Widget *item;

	if (m_freedItems.empty()) {
		item = nullptr;
	}
	else {
		item = m_freedItems.at(0);
		item->retain();
		m_freedItems.erase(0);
		item->autorelease();
	}
	return item;
}

void StatUpgradePanel::selectedItemEvent(Ref* pSender, ListView::EventType type)
{
	if (type == ListView::EventType::ON_SELECTED_ITEM_END)
	{
		ListView* listView = static_cast<ListView*>(pSender);
		auto& items = listView->getItems();
		ssize_t index = listView->getCurSelectedIndex();
		if (index >= 0 && index < items.size())
		{
			StatUpgradeItem* item = dynamic_cast<StatUpgradeItem*>(items.at(index));
			if (item)
			{
				StatUpgradeData& data = m_dataSet[index];
				this->upgradeStat(item, data);
			}
		}
	}
}


void StatUpgradePanel::scrollListEvent(Ref* pSender, ui::ScrollView::EventType type)
{
	if (!m_listScrollCallback)
		return;

	switch (type)
	{
	case ScrollView::EventType::SCROLLING:
		m_listScrollCallback(this, LIST_SCROLLING);
		break;
	case ScrollView::EventType::SCROLLING_BEGAN:
		m_listScrollCallback(this, LIST_SCROLLING_BEGAN);
		break;
	case ScrollView::EventType::AUTOSCROLL_ENDED:
		m_listScrollCallback(this, LIST_SCROLLING_ENDED);
		break;
	default:
		break;
	}
}


NS_END