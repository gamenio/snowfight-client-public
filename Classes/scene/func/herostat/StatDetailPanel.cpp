//
// StatDetailPanel.cpp
// snowfight
//
// Created by Luthier on 2019/5/9.
//

#include "StatDetailPanel.h"

#include "common/utils/MathTools.h"
#include "game/LocaleMgr.h"
#include "game/gamecenter/GameCenter.h"
#include "game/utils/UnitHelper.h"
#include "game/ObjectMgr.h"
#include "scene/SoundMgr.h"
#include "scene/Utils.h"


using namespace cocos2d::ui;

NS_BEGIN

#define FRAMENAME_STATDETAIL_RED_BAR_BG					"statdetail_red_bar_bg.png"
#define FRAMENAME_STATDETAIL_RED_BAR_FG					"statdetail_red_bar_fg.png"
#define FRAMENAME_STATDETAIL_GREEN_BAR_BG				"statdetail_green_bar_bg.png"
#define FRAMENAME_STATDETAIL_GREEN_BAR_FG				"statdetail_green_bar_fg.png"
#define FRAMENAME_STATDETAIL_BLUE_BAR_BG				"statdetail_blue_bar_bg.png"
#define FRAMENAME_STATDETAIL_BLUE_BAR_FG				"statdetail_blue_bar_fg.png"
#define FRAMENAME_STATDETAIL_YELLOW_BAR_BG				"statdetail_yellow_bar_bg.png"
#define FRAMENAME_STATDETAIL_YELLOW_BAR_FG				"statdetail_yellow_bar_fg.png"
#define FRAMENAME_STATDETAIL_GROUP_BG					"statdetail_group_bg.png"


#define ACTION_TAG_PROGRESS					1

#define FULL_PERCENTAGE							100
#define PROGRESS_PER_SECOND						150 // 每秒百分比进度

#define GROUP_PADDING_LEFTRIGHT		6
#define GROUP_BG_TOP				4

#define CONTENT_SIZE					Size(163, 125)
#define NORMAL_GROUP_MARGIN_TOP			6
#define NORMAL_GROUP_SIZE				Size(80, 40)
#define MISC_GROUP_SIZE					Size(CONTENT_SIZE.width, 56)
#define STATS_BAR_MARGIN_TOP			5
#define STATS_BAR_HEIGHT				6
#define REGEN_RATE_MARGIN_TOP			6
#define ATTACK_RANGE_ITEM_MARGIN_TOP	9

#define SCHEDULE_KEY_COUNTING_TICK		"CountingTick"


StatDetailPanel* StatDetailPanel::create()
{
    auto ret = new (std::nothrow) StatDetailPanel();
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

bool StatDetailPanel::init()
{
    if (!LayerColor::initWithColor(Color4B(0, 0, 0, 0)))
        return false;
    
	this->setContentSize(CONTENT_SIZE);
	this->setIgnoreAnchorPointForPosition(false);

	m_nameLabel = Label::createWithSystemFont("Name", DEFAULT_SYSTEM_FONT, 15);
	m_nameLabel->setAnchorPoint(Point::ANCHOR_TOP_LEFT);
	m_nameLabel->setTextColor(Color4B(86, 111, 132, 255));
	Utils::enableBoldForLabel(m_nameLabel);
	m_nameLabel->setPosition(6, this->getContentSize().height);
	this->addChild(m_nameLabel);

	this->initHealthGroup();
	this->initStaminaGroup();
	this->initMiscGroup();

    return true;
}

void StatDetailPanel::setHeroTemplate(HeroTemplate const* tmpl)
{
	m_template = tmpl;
	this->setHeroName(tmpl->name);

	// 最大属性值
	PlayerTemplate const* playerTmpl = sObjectMgr->getPlayerTemplate(tmpl->id);
	uint8 maxStage = playerTmpl->getMaxStage();
	m_statValues.maxHealth = playerTmpl->getStageStat(maxStage, STAT_MAX_HEALTH).value.asInt();
	float maxHealthRegenRate = playerTmpl->getStageStat(maxStage, STAT_HEALTH_REGEN_RATE).value.asFloat();
	m_statValues.maxHealthRegenRate = MathTools::roundFloatWithPrecision(maxHealthRegenRate / 60, 4); // 以秒为单位
	m_statValues.maxStamina = playerTmpl->getStageStat(maxStage, STAT_MAX_STAMINA).value.asInt();
	m_statValues.maxStaminaRegenRate = playerTmpl->getStageStat(maxStage, STAT_STAMINA_REGEN_RATE).value.asFloat();
	m_statValues.maxDamage = playerTmpl->getStageStat(maxStage, STAT_DAMAGE).value.asInt();
	m_statValues.maxMoveSpeed = playerTmpl->getStageStat(maxStage, STAT_MOVE_SPEED).value.asInt();
	m_statValues.maxAttackRange = (int32)playerTmpl->getStageStat(maxStage, STAT_ATTACK_RANGE).value.asFloat();

	this->updateStatValues();

	this->stopProgressAnimations();
	this->initProgressAnimations();
	this->startProgressAnimations();
}

void StatDetailPanel::updateStats()
{
	this->updateStatValues();

	this->stopProgressAnimations();
	this->startProgressAnimations();
}

void StatDetailPanel::stopAnimation()
{
	this->stopProgressAnimations();
}

void StatDetailPanel::updateStatValues()
{
	LocalPlayer* localPlayer = sGameCenter->getLocalPlayer();

	// 当前属性值
	PlayerTemplate const* playerTmpl = sObjectMgr->getPlayerTemplate(m_template->id);
	m_statValues.health = playerTmpl->getStageStat(localPlayer->getStatStage(m_template->id, STAT_MAX_HEALTH), STAT_MAX_HEALTH).value.asInt();
	float healthRegenRate = playerTmpl->getStageStat(localPlayer->getStatStage(m_template->id, STAT_HEALTH_REGEN_RATE), STAT_HEALTH_REGEN_RATE).value.asFloat();
	m_statValues.healthRegenRate = MathTools::roundFloatWithPrecision(healthRegenRate / 60, 4);
	m_statValues.stamina = playerTmpl->getStageStat(localPlayer->getStatStage(m_template->id, STAT_MAX_STAMINA), STAT_MAX_STAMINA).value.asInt();
	m_statValues.staminaRegenRate = playerTmpl->getStageStat(localPlayer->getStatStage(m_template->id, STAT_STAMINA_REGEN_RATE), STAT_STAMINA_REGEN_RATE).value.asFloat();
	m_statValues.damage = playerTmpl->getStageStat(localPlayer->getStatStage(m_template->id, STAT_DAMAGE), STAT_DAMAGE).value.asInt();
	m_statValues.moveSpeed = playerTmpl->getStageStat(localPlayer->getStatStage(m_template->id, STAT_MOVE_SPEED), STAT_MOVE_SPEED).value.asInt();
	m_statValues.attackRange = (int32)playerTmpl->getStageStat(localPlayer->getStatStage(m_template->id, STAT_ATTACK_RANGE), STAT_ATTACK_RANGE).value.asFloat();

}

void StatDetailPanel::initProgressAnimations()
{
	m_healthProg->setPercentage(0);
	m_staminaProg->setPercentage(0);
	m_damageProg->setPercentage(0);
	m_moveSpeedProg->setPercentage(0);
	m_attackRangeProg->setPercentage(0);

	m_healthRegenRateLabel->setValue(0);
	m_staminaRegenRateLabel->setValue(0);

	m_healthValueLabel->setValue(0);
	m_healthValueLabel->setMaximum(m_statValues.maxHealth);
	m_staminaValueLabel->setValue(0);
	m_staminaValueLabel->setMaximum(m_statValues.maxStamina);
	m_damageValueLabel->setValue(0);
	m_damageValueLabel->setMaximum(m_statValues.maxDamage);
	m_moveSpeedValueLabel->setValue(0);
	m_moveSpeedValueLabel->setMaximum(m_statValues.maxMoveSpeed);
	m_attackRangeValueLabel->setValue(0);
	m_attackRangeValueLabel->setMaximum(m_statValues.maxAttackRange);
}


void StatDetailPanel::startProgressAnimations()
{
	float duration = 0.f;

	if (m_healthValueLabel->getValue() < m_statValues.health)
	{
		float dur = this->startProgressActions(m_statValues.health, m_statValues.maxHealth, m_healthValueLabel, m_healthProg);
		duration = MAX(dur, duration);
	}

	if (m_healthRegenRateLabel->getValue() < m_statValues.healthRegenRate)
	{
		float dur = this->startProgressAction(m_statValues.healthRegenRate, m_statValues.maxHealthRegenRate, m_healthRegenRateLabel);
		duration = MAX(dur, duration);
	}

	if (m_staminaValueLabel->getValue() < m_statValues.stamina)
	{
		float dur = this->startProgressActions(m_statValues.stamina, m_statValues.maxStamina, m_staminaValueLabel, m_staminaProg);
		duration = MAX(dur, duration);
	}

	if (m_staminaRegenRateLabel->getValue() < m_statValues.staminaRegenRate)
	{
		float dur = this->startProgressAction(m_statValues.staminaRegenRate, m_statValues.maxStaminaRegenRate, m_staminaRegenRateLabel);
		duration = MAX(dur, duration);
	}

	if (m_damageValueLabel->getValue() < m_statValues.damage)
	{
		float dur = this->startProgressActions(m_statValues.damage, m_statValues.maxDamage, m_damageValueLabel, m_damageProg);
		duration = MAX(dur, duration);
	}

	if (m_moveSpeedValueLabel->getValue() < m_statValues.moveSpeed)
	{
		float dur = this->startProgressActions(m_statValues.moveSpeed, m_statValues.maxMoveSpeed, m_moveSpeedValueLabel, m_moveSpeedProg);
		duration = MAX(dur, duration);
	}

	if (m_attackRangeValueLabel->getValue() < m_statValues.attackRange)
	{
		float dur = this->startProgressActions(m_statValues.attackRange, m_statValues.maxAttackRange, m_attackRangeValueLabel, m_attackRangeProg);
		duration = MAX(dur, duration);
	}

	if(duration > 0.f)
		this->playCountingTick(duration);
}

void StatDetailPanel::stopProgressAnimations()
{
	this->stopProgressActions(m_healthValueLabel, m_healthProg);
	this->stopProgressAction(m_healthRegenRateLabel);
	this->stopProgressActions(m_staminaValueLabel, m_staminaProg);
	this->stopProgressAction(m_staminaRegenRateLabel);
	this->stopProgressActions(m_damageValueLabel, m_damageProg);
	this->stopProgressActions(m_moveSpeedValueLabel, m_moveSpeedProg);
	this->stopProgressActions(m_attackRangeValueLabel, m_attackRangeProg);

	this->stopCountingTick();
}

StatDetailPanel::StatDetailPanel() :
	m_template(nullptr),
	m_nameLabel(nullptr),
	m_healthGroup(nullptr),
	m_healthValueLabel(nullptr),
	m_healthProg(nullptr),
	m_healthRegenRateLabel(nullptr),
	m_staminaGroup(nullptr),
	m_staminaValueLabel(nullptr),
	m_staminaProg(nullptr),
	m_staminaRegenRateLabel(nullptr),
	m_miscGroup(nullptr),
	m_damageValueLabel(nullptr),
	m_damageProg(nullptr),
	m_moveSpeedValueLabel(nullptr),
	m_moveSpeedProg(nullptr),
	m_attackRangeValueLabel(nullptr),
	m_attackRangeProg(nullptr)
{
    
}

StatDetailPanel::~StatDetailPanel()
{
	m_template = nullptr;
	m_nameLabel = nullptr;

	m_healthGroup = nullptr;
	m_healthValueLabel = nullptr;
	m_healthProg = nullptr;
	m_healthRegenRateLabel = nullptr;

	m_staminaGroup = nullptr;
	m_staminaValueLabel = nullptr;
	m_staminaProg = nullptr;
	m_staminaRegenRateLabel = nullptr;

	m_miscGroup = nullptr;
	m_damageValueLabel = nullptr;
	m_damageProg = nullptr;
	m_moveSpeedValueLabel = nullptr;
	m_moveSpeedProg = nullptr;
	m_attackRangeValueLabel = nullptr;
	m_attackRangeProg = nullptr;
}

void StatDetailPanel::initHealthGroup()
{
	m_healthGroup = Node::create();
	m_healthGroup->setAnchorPoint(Point::ANCHOR_TOP_LEFT);
	m_healthGroup->setIgnoreAnchorPointForPosition(false);
	m_healthGroup->setContentSize(NORMAL_GROUP_SIZE);
	m_healthGroup->setPosition(0, m_nameLabel->getBoundingBox().getMinY() - NORMAL_GROUP_MARGIN_TOP);
	this->addChild(m_healthGroup);

	Scale9Sprite* groupBg = Scale9Sprite::createWithSpriteFrameName(FRAMENAME_STATDETAIL_GROUP_BG);
	groupBg->setAnchorPoint(Point::ANCHOR_TOP_LEFT);
	groupBg->setPosition(0, m_healthGroup->getContentSize().height - GROUP_BG_TOP);
	groupBg->setContentSize(Size(m_healthGroup->getContentSize().width, groupBg->getPosition().y));
	m_healthGroup->addChild(groupBg);

	Label* title = Label::createWithSystemFont(sLocaleMgr->getString("statdetail_title_health"), DEFAULT_SYSTEM_FONT, 8);
	title->setAnchorPoint(Point::ANCHOR_TOP_LEFT);
	title->setPosition(GROUP_PADDING_LEFTRIGHT, m_healthGroup->getContentSize().height);
	title->setTextColor(Color4B(86, 111, 132, 255));
	Utils::enableBoldForLabel(title);
	m_healthGroup->addChild(title);

	m_healthValueLabel = StatValueLabel::create(StatValueLabel::NUMBER);
	m_healthValueLabel->setAnchorPoint(Point::ANCHOR_BOTTOM_RIGHT);
	m_healthValueLabel->setPosition(m_healthGroup->getContentSize().width - GROUP_PADDING_LEFTRIGHT, title->getBoundingBox().getMinY());
	m_healthGroup->addChild(m_healthValueLabel);

	Sprite* progBg = Sprite::createWithSpriteFrameName(FRAMENAME_STATDETAIL_RED_BAR_BG);
	progBg->setContentSize(Size(m_healthGroup->getContentSize().width - GROUP_PADDING_LEFTRIGHT * 2, STATS_BAR_HEIGHT));
	progBg->setAnchorPoint(Point::ANCHOR_TOP_LEFT);
	progBg->setPosition(GROUP_PADDING_LEFTRIGHT, title->getBoundingBox().getMinY() - STATS_BAR_MARGIN_TOP);
	m_healthGroup->addChild(progBg);

	Sprite* progFg = Sprite::createWithSpriteFrameName(FRAMENAME_STATDETAIL_RED_BAR_FG);
	progFg->setContentSize(progBg->getContentSize());
	m_healthProg = ProgressTimer::create(progFg);
	m_healthProg->setAnchorPoint(progBg->getAnchorPoint());
	m_healthProg->setPosition(progBg->getPosition());
	m_healthProg->setType(ProgressTimer::Type::BAR);
	m_healthProg->setBarChangeRate(Vec2(1, 0));
	m_healthProg->setMidpoint(Vec2(0, 0));
	m_healthGroup->addChild(m_healthProg);

	Label* regenRateTitle = Label::createWithSystemFont(sLocaleMgr->getString("statdetail_title_regen_rate"), DEFAULT_SYSTEM_FONT, 7);
	regenRateTitle->setAnchorPoint(Point::ANCHOR_TOP_LEFT);
	regenRateTitle->setPosition(GROUP_PADDING_LEFTRIGHT, progBg->getBoundingBox().getMinY() - REGEN_RATE_MARGIN_TOP);
	regenRateTitle->setTextColor(Color4B(86, 111, 132, 255));
	Utils::enableBoldForLabel(regenRateTitle);
	m_healthGroup->addChild(regenRateTitle);

	m_healthRegenRateLabel = StatValueLabel::create(StatValueLabel::PERCENTAGE);
	m_healthRegenRateLabel->setAnchorPoint(Point::ANCHOR_BOTTOM_RIGHT);
	m_healthRegenRateLabel->setPosition(m_healthGroup->getContentSize().width - GROUP_PADDING_LEFTRIGHT, regenRateTitle->getBoundingBox().getMinY());
	m_healthRegenRateLabel->setPrecision(2);
	m_healthGroup->addChild(m_healthRegenRateLabel);
}

void StatDetailPanel::initStaminaGroup()
{
	m_staminaGroup = Node::create();
	m_staminaGroup->setAnchorPoint(Point::ANCHOR_TOP_RIGHT);
	m_staminaGroup->setIgnoreAnchorPointForPosition(false);
	m_staminaGroup->setContentSize(NORMAL_GROUP_SIZE);
	m_staminaGroup->setPosition(this->getContentSize().width, m_nameLabel->getBoundingBox().getMinY() - NORMAL_GROUP_MARGIN_TOP);
	this->addChild(m_staminaGroup);

	Scale9Sprite* groupBg = Scale9Sprite::createWithSpriteFrameName(FRAMENAME_STATDETAIL_GROUP_BG);
	groupBg->setAnchorPoint(Point::ANCHOR_TOP_LEFT);
	groupBg->setPosition(0, m_healthGroup->getContentSize().height - GROUP_BG_TOP);
	groupBg->setContentSize(Size(m_staminaGroup->getContentSize().width, groupBg->getPosition().y));
	m_staminaGroup->addChild(groupBg);

	Label* title = Label::createWithSystemFont(sLocaleMgr->getString("statdetail_title_stamina"), DEFAULT_SYSTEM_FONT, 8);
	title->setAnchorPoint(Point::ANCHOR_TOP_LEFT);
	title->setPosition(GROUP_PADDING_LEFTRIGHT, m_healthGroup->getContentSize().height);
	title->setTextColor(Color4B(86, 111, 132, 255));
	Utils::enableBoldForLabel(title);
	m_staminaGroup->addChild(title);

	m_staminaValueLabel = StatValueLabel::create(StatValueLabel::NUMBER);
	m_staminaValueLabel->setAnchorPoint(Point::ANCHOR_BOTTOM_RIGHT);
	m_staminaValueLabel->setPosition(m_healthGroup->getContentSize().width - GROUP_PADDING_LEFTRIGHT, title->getBoundingBox().getMinY());
	m_staminaGroup->addChild(m_staminaValueLabel);

	Sprite* progBg = Sprite::createWithSpriteFrameName(FRAMENAME_STATDETAIL_GREEN_BAR_BG);
	progBg->setContentSize(Size(m_staminaGroup->getContentSize().width - GROUP_PADDING_LEFTRIGHT * 2, STATS_BAR_HEIGHT));
	progBg->setAnchorPoint(Point::ANCHOR_TOP_LEFT);
	progBg->setPosition(GROUP_PADDING_LEFTRIGHT, title->getBoundingBox().getMinY() - STATS_BAR_MARGIN_TOP);
	m_staminaGroup->addChild(progBg);

	Sprite* progFg = Sprite::createWithSpriteFrameName(FRAMENAME_STATDETAIL_GREEN_BAR_FG);
	progFg->setContentSize(progBg->getContentSize());
	m_staminaProg = ProgressTimer::create(progFg);
	m_staminaProg->setAnchorPoint(progBg->getAnchorPoint());
	m_staminaProg->setPosition(progBg->getPosition());
	m_staminaProg->setType(ProgressTimer::Type::BAR);
	m_staminaProg->setBarChangeRate(Vec2(1, 0));
	m_staminaProg->setMidpoint(Vec2(0, 0));
	m_staminaGroup->addChild(m_staminaProg);

	Label* regenRateTitle = Label::createWithSystemFont(sLocaleMgr->getString("statdetail_title_regen_rate"), DEFAULT_SYSTEM_FONT, 7);
	regenRateTitle->setAnchorPoint(Point::ANCHOR_TOP_LEFT);
	regenRateTitle->setPosition(GROUP_PADDING_LEFTRIGHT, progBg->getBoundingBox().getMinY() - REGEN_RATE_MARGIN_TOP);
	regenRateTitle->setTextColor(Color4B(86, 111, 132, 255));
	Utils::enableBoldForLabel(regenRateTitle);
	m_staminaGroup->addChild(regenRateTitle);

	m_staminaRegenRateLabel = StatValueLabel::create(StatValueLabel::PERCENTAGE);
	m_staminaRegenRateLabel->setAnchorPoint(Point::ANCHOR_BOTTOM_RIGHT);
	m_staminaRegenRateLabel->setPosition(m_staminaGroup->getContentSize().width - GROUP_PADDING_LEFTRIGHT, regenRateTitle->getBoundingBox().getMinY());
	m_staminaGroup->addChild(m_staminaRegenRateLabel);
}

void StatDetailPanel::initMiscGroup()
{
	m_miscGroup = Node::create();
	m_miscGroup->setAnchorPoint(Point::ANCHOR_BOTTOM_RIGHT);
	m_miscGroup->setIgnoreAnchorPointForPosition(false);
	m_miscGroup->setContentSize(MISC_GROUP_SIZE);
	m_miscGroup->setPosition(this->getContentSize().width, 0);
	this->addChild(m_miscGroup);

	Scale9Sprite* groupBg = Scale9Sprite::createWithSpriteFrameName(FRAMENAME_STATDETAIL_GROUP_BG);
	groupBg->setAnchorPoint(Point::ANCHOR_TOP_LEFT);
	groupBg->setPosition(0, m_miscGroup->getContentSize().height - GROUP_BG_TOP);
	groupBg->setContentSize(Size(m_miscGroup->getContentSize().width, groupBg->getPosition().y));
	m_miscGroup->addChild(groupBg);


	this->initDamageItem();
	this->initMoveSpeedItem();
	this->initAttackRangeItem();

}

void StatDetailPanel::initDamageItem()
{
	Label* title = Label::createWithSystemFont(sLocaleMgr->getString("statdetail_title_damage"), DEFAULT_SYSTEM_FONT, 8);
	title->setAnchorPoint(Point::ANCHOR_TOP_LEFT);
	title->setPosition(GROUP_PADDING_LEFTRIGHT, m_miscGroup->getContentSize().height);
	title->setTextColor(Color4B(86, 111, 132, 255));
	Utils::enableBoldForLabel(title);
	m_miscGroup->addChild(title);

	m_damageValueLabel = StatValueLabel::create(StatValueLabel::NUMBER);
	m_damageValueLabel->setAnchorPoint(Point::ANCHOR_BOTTOM_RIGHT);
	m_damageValueLabel->setPosition(NORMAL_GROUP_SIZE.width - GROUP_PADDING_LEFTRIGHT, title->getBoundingBox().getMinY());
	m_miscGroup->addChild(m_damageValueLabel);

	Sprite* progBg = Sprite::createWithSpriteFrameName(FRAMENAME_STATDETAIL_YELLOW_BAR_BG);
	progBg->setContentSize(Size(NORMAL_GROUP_SIZE.width - GROUP_PADDING_LEFTRIGHT * 2, STATS_BAR_HEIGHT));
	progBg->setAnchorPoint(Point::ANCHOR_TOP_LEFT);
	progBg->setPosition(GROUP_PADDING_LEFTRIGHT, title->getBoundingBox().getMinY() - STATS_BAR_MARGIN_TOP);
	m_miscGroup->addChild(progBg);

	Sprite* progFg = Sprite::createWithSpriteFrameName(FRAMENAME_STATDETAIL_YELLOW_BAR_FG);
	progFg->setContentSize(progBg->getContentSize());
	m_damageProg = ProgressTimer::create(progFg);
	m_damageProg->setAnchorPoint(progBg->getAnchorPoint());
	m_damageProg->setPosition(progBg->getPosition());
	m_damageProg->setType(ProgressTimer::Type::BAR);
	m_damageProg->setBarChangeRate(Vec2(1, 0));
	m_damageProg->setMidpoint(Vec2(0, 0));
	m_miscGroup->addChild(m_damageProg);
}

void StatDetailPanel::initMoveSpeedItem()
{
	Label* title = Label::createWithSystemFont(sLocaleMgr->getString("statdetail_title_move_speed"), DEFAULT_SYSTEM_FONT, 8);
	title->setAnchorPoint(Point::ANCHOR_TOP_LEFT);
	title->setPosition(m_miscGroup->getContentSize().width - NORMAL_GROUP_SIZE.width + GROUP_PADDING_LEFTRIGHT, m_miscGroup->getContentSize().height);
	title->setTextColor(Color4B(86, 111, 132, 255));
	Utils::enableBoldForLabel(title);
	m_miscGroup->addChild(title);

	m_moveSpeedValueLabel = StatValueLabel::create(StatValueLabel::NUMBER);
	m_moveSpeedValueLabel->setAnchorPoint(Point::ANCHOR_BOTTOM_RIGHT);
	m_moveSpeedValueLabel->setPosition(m_miscGroup->getContentSize().width - GROUP_PADDING_LEFTRIGHT, title->getBoundingBox().getMinY());
	m_miscGroup->addChild(m_moveSpeedValueLabel);

	Sprite* progBg = Sprite::createWithSpriteFrameName(FRAMENAME_STATDETAIL_BLUE_BAR_BG);
	progBg->setContentSize(Size(NORMAL_GROUP_SIZE.width - GROUP_PADDING_LEFTRIGHT * 2, STATS_BAR_HEIGHT));
	progBg->setAnchorPoint(Point::ANCHOR_TOP_LEFT);
	progBg->setPosition(m_miscGroup->getContentSize().width - NORMAL_GROUP_SIZE.width + GROUP_PADDING_LEFTRIGHT, title->getBoundingBox().getMinY() - STATS_BAR_MARGIN_TOP);
	m_miscGroup->addChild(progBg);

	Sprite* progFg = Sprite::createWithSpriteFrameName(FRAMENAME_STATDETAIL_BLUE_BAR_FG);
	progFg->setContentSize(progBg->getContentSize());
	m_moveSpeedProg = ProgressTimer::create(progFg);
	m_moveSpeedProg->setAnchorPoint(progBg->getAnchorPoint());
	m_moveSpeedProg->setPosition(progBg->getPosition());
	m_moveSpeedProg->setType(ProgressTimer::Type::BAR);
	m_moveSpeedProg->setBarChangeRate(Vec2(1, 0));
	m_moveSpeedProg->setMidpoint(Vec2(0, 0));
	m_miscGroup->addChild(m_moveSpeedProg);
}

void StatDetailPanel::initAttackRangeItem()
{
	Label* title = Label::createWithSystemFont(sLocaleMgr->getString("statdetail_title_attack_range"), DEFAULT_SYSTEM_FONT, 8);
	title->setAnchorPoint(Point::ANCHOR_TOP_LEFT);
	title->setPosition(GROUP_PADDING_LEFTRIGHT, m_damageProg->getBoundingBox().getMinY() - ATTACK_RANGE_ITEM_MARGIN_TOP);
	title->setTextColor(Color4B(86, 111, 132, 255));
	Utils::enableBoldForLabel(title);
	m_miscGroup->addChild(title);

	Sprite* progBg = Sprite::createWithSpriteFrameName(FRAMENAME_STATDETAIL_YELLOW_BAR_BG);
	progBg->setContentSize(Size(106, STATS_BAR_HEIGHT));
	progBg->setAnchorPoint(Point::ANCHOR_TOP_LEFT);
	progBg->setPosition(GROUP_PADDING_LEFTRIGHT, title->getBoundingBox().getMinY() - STATS_BAR_MARGIN_TOP);
	m_miscGroup->addChild(progBg);

	Sprite* progFg = Sprite::createWithSpriteFrameName(FRAMENAME_STATDETAIL_YELLOW_BAR_FG);
	progFg->setContentSize(progBg->getContentSize());
	m_attackRangeProg = ProgressTimer::create(progFg);
	m_attackRangeProg->setAnchorPoint(progBg->getAnchorPoint());
	m_attackRangeProg->setPosition(progBg->getPosition());
	m_attackRangeProg->setType(ProgressTimer::Type::BAR);
	m_attackRangeProg->setBarChangeRate(Vec2(1, 0));
	m_attackRangeProg->setMidpoint(Vec2(0, 0));
	m_miscGroup->addChild(m_attackRangeProg);

	m_attackRangeValueLabel = StatValueLabel::create(StatValueLabel::NUMBER);
	m_attackRangeValueLabel->setAnchorPoint(Point::ANCHOR_BOTTOM_RIGHT);
	m_attackRangeValueLabel->setPosition(m_attackRangeProg->getBoundingBox().getMaxX(), title->getBoundingBox().getMinY());
	m_miscGroup->addChild(m_attackRangeValueLabel);
}

float StatDetailPanel::startProgressActions(float value, float maximum, StatValueLabel* valueLabel, ProgressTimer* progressTimer)
{
	this->stopProgressActions(valueLabel, progressTimer);

	float toPercentage = value / maximum * FULL_PERCENTAGE;
	float time = (toPercentage - progressTimer->getPercentage()) / PROGRESS_PER_SECOND;
	time = MAX(time, 0.1f);
	Action* progress = EaseIn::create(ProgressFromTo::create(time, progressTimer->getPercentage(), toPercentage), 0.2f);
	progress->setTag(ACTION_TAG_PROGRESS);
	progressTimer->runAction(progress);

	Action* tween = EaseIn::create(ActionTween::create(time, "", valueLabel->getValue(), value), 0.2f);
	tween->setTag(ACTION_TAG_PROGRESS);
	valueLabel->runAction(tween);

	return time;
}

void StatDetailPanel::stopProgressActions(Label* valueLabel, ProgressTimer* progressTimer)
{
	if (Action* action = valueLabel->getActionByTag(ACTION_TAG_PROGRESS))
		valueLabel->stopAction(action);

	if (Action* action = progressTimer->getActionByTag(ACTION_TAG_PROGRESS))
		progressTimer->stopAction(action);
}

float StatDetailPanel::startProgressAction(float value, float maximum, StatValueLabel* valueLabel)
{
	this->stopProgressAction(valueLabel);

	float fromPercentage = valueLabel->getValue() / maximum * FULL_PERCENTAGE;
	float toPercentage = value / maximum * FULL_PERCENTAGE;
	float time = (toPercentage - fromPercentage) / PROGRESS_PER_SECOND;
	time = MAX(time, 0.1f);
	Action* tween = EaseIn::create(ActionTween::create(time, "", valueLabel->getValue(), value), 0.2f);
	tween->setTag(ACTION_TAG_PROGRESS);
	valueLabel->runAction(tween);

	return time;
}

void StatDetailPanel::stopProgressAction(Label* valueLabel)
{
	if (Action* action = valueLabel->getActionByTag(ACTION_TAG_PROGRESS))
		valueLabel->stopAction(action);
}

void StatDetailPanel::playCountingTick(float duration)
{
	this->stopCountingTick();

	sSoundMgr->play(SOUND_STAT_COUNTING, true);
	this->scheduleOnce([](float dt) {
		sSoundMgr->stop(SOUND_STAT_COUNTING);
	}, duration, SCHEDULE_KEY_COUNTING_TICK);
}

void StatDetailPanel::stopCountingTick()
{
	if (this->getScheduler()->isScheduled(SCHEDULE_KEY_COUNTING_TICK, this))
		this->unschedule(SCHEDULE_KEY_COUNTING_TICK);

	if(sSoundMgr->isPlaying(SOUND_STAT_COUNTING))
		sSoundMgr->stop(SOUND_STAT_COUNTING);
}

void StatDetailPanel::setHeroName(std::string const& name)
{
	m_nameLabel->setString(name);
}

NS_END
