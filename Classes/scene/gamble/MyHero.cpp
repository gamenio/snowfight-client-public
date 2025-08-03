#include "MyHero.h"

#include "common/utils/MathTools.h"
#include "common/utils/TimeUtil.h"
#include "common/Machine.h"
#include "game/entities/DataUnit.h"
#include "game/entities/DataWorldObject.h"
#include "game/entities/DataPlayer.h"
#include "game/utils/UnitHelper.h"
#include "game/behaviors/SharedUnitDefines.h"
#include "game/behaviors/ObjectShapes.h"
#include "game/LocaleMgr.h"
#include "game/gamecenter/GameCenter.h"
#include "game/World.h"
#include "game/behaviors/MyCharacter.h"
#include "game/ObjectMgr.h"
#include "scene/TutorialService.h"
#include "scene/Utils.h"
#include "scene/ObjectPools.h"
#include "scene/gamble/Snowball.h"
#include "scene/SoundMgr.h"
#include "scene/tilemap/FastTMXLayer.h"
#include "GambleDefines.h"
#include "CarriedProp.h"
#include "Prop.h"

NS_BEGIN

// Debug drawer
#if NS_DEBUG

#define DEBUG_HIDINGSPOT_SEARCHER		0

#endif // NS_DEBUG

// 声音触发器
#define SOUND_TIMER_INTERVAL		30000 // 声音计时器间隔时间，单位：毫秒
#define SOUND_TRIGGER_COW		    "Cow"
#define SOUND_TRIGGER_SHEEP			"Sheep"
#define SOUND_TRIGGER_CHICKEN		"Chicken"
#define SOUND_TRIGGER_PIG			"Pig"
#define SOUND_TRIGGER_DOG			"Dog"
#define SOUND_TRIGGER_DUCK          "Duck"
std::unordered_map<std::string, std::string> sSoundTriggerResources = {
	{ SOUND_TRIGGER_COW,        SOUND_COW		},
	{ SOUND_TRIGGER_SHEEP,      SOUND_SHEEP		},
	{ SOUND_TRIGGER_CHICKEN,    SOUND_CHICKEN	},
	{ SOUND_TRIGGER_PIG,        SOUND_PIG		},
	{ SOUND_TRIGGER_DOG,        SOUND_DOG		},
	{ SOUND_TRIGGER_DUCK,       SOUND_DUCK		},
};

#define FRAMENAME_MY_HERO_ARROW			"my_hero_arrow.png"
#define TAG_ARROW						1
#define ARROW_MOVE_DURATION				0.25f

#define FLOATING_LABEL_ACTION_TAG		1

#define HIDING_SPOT_OPACITY				125
#define HIDING_SOPT_NORMAL_COLOR		Color3B(255, 255, 255)
#define HIDING_SOPT_HIGHLIGHT_COLOR		Color3B(0, 255, 0)

#define TEST_BLOCK_SPACING				1.0f		// 测试障碍物的间距，单位：points
#define RECEIVED_PROP_CONTENT_SIZE		Size(50, 50)

// GlobalZOrder < 0
#define LOCAL_ZORDER_PROP_BUBBLE		1
#define LOCAL_ZORDER_SMILEY				2

// GlobalZOrder == 0
#define LOCAL_ZORDER_EQUIPMENT_BAR		1
#define LOCAL_ZORDER_PROP_RECEIVED		2
#define LOCAL_ZORDER_FLOATING_LABEL		3
#define LOCAL_ZORDER_ARROW				4

MyHero::MyHero():
	m_fired(false),
	m_aimPointer(nullptr),
	m_equipmentBar(nullptr),
	m_floatingLabelContainer(nullptr),
	m_propBubble(nullptr),
	m_currTileCoord(TileCoord::INVALID),
	m_isPositionInTilesDirty(false),
	m_hidingSpotSearcher(nullptr)
{
}


MyHero::~MyHero()
{
	CC_SAFE_DELETE(m_hidingSpotSearcher);

	m_aimPointer = nullptr;
	m_equipmentBar = nullptr;
	m_floatingLabelContainer = nullptr;
	m_propBubble = nullptr;
}

MyHero* MyHero::createWithData(DataPlayer* data)
{
	MyHero *pRet = new MyHero();
	if (pRet && pRet->initWithData(data))
	{
		pRet->autorelease();
		return pRet;
	}
	else
	{
		CC_SAFE_DELETE(pRet);
		return nullptr;
	}
}

bool MyHero::initWithData(DataPlayer* data)
{
	if (!Hero::initWithData(data))
		return false;

	World* world = World::getInstance();
	world->addMyCharacterListener(this);
	world->addMessageListener(this);

	m_hidingSpotSearcher = new HidingSpotSearcher(data->getMapData());
#if DEBUG_HIDINGSPOT_SEARCHER
	DrawNode* drawNode = sDebugDrawer->getDrawByTag("HidingSpotSearcher")->getDrawNode();
	m_hidingSpotSearcher->setDebugDraw(drawNode);
#endif // DEBUG_HIDINGSPOT_SEARCHER

	m_aimPointer = AimPointer::create();
	m_aimPointer->setVisible(false);
	this->addChild(m_aimPointer);

	m_propBubble = PropBubble::create();
	m_propBubble->setAnchorPoint(Point::ANCHOR_MIDDLE_BOTTOM);
	m_propBubble->setPosition(m_mainSp->getBoundingBox().getMidX(), m_nameplate->getBoundingBox().getMaxY() - 3);
	this->addChild(m_propBubble);

	m_equipmentBar = EquipmentBar::create();
	m_equipmentBar->setAnchorPoint(Point::ANCHOR_MIDDLE);
	m_equipmentBar->setPosition(m_mainSp->getBoundingBox().getMidX(), 31);
	this->addChild(m_equipmentBar);

	m_floatingLabelContainer = Node::create();
	m_floatingLabelContainer->setAnchorPoint(Point::ANCHOR_MIDDLE_BOTTOM);
	this->addChild(m_floatingLabelContainer);

	m_propBubble->setLocalZOrder(LOCAL_ZORDER_PROP_BUBBLE);
	m_smiley->setLocalZOrder(LOCAL_ZORDER_SMILEY);
	m_equipmentBar->setLocalZOrder(LOCAL_ZORDER_EQUIPMENT_BAR);
	m_floatingLabelContainer->setLocalZOrder(LOCAL_ZORDER_FLOATING_LABEL);

	return true;
}

void MyHero::update(float delta)
{
	Hero::update(delta);

	if (!this->isActive())
		return;

	this->saveLevelAndXPIfUpdated(false);
	this->updatePositionInTiles(false);

	this->updateAlive();
	this->updateLevel();
	this->updateHidingSpots(false);
	this->updatePropBubbleVisibility(false);
	this->updateMagicBeanCount();
	this->updateReceiveDamage();

	this->updateAimPointer(delta);
	this->checkSoundTriggers();
}

void MyHero::onEnter()
{
	Hero::onEnter();
}

void MyHero::onActivated()
{
	if (this->isActive())
		return;

	Hero::onActivated();

	this->saveLevelAndXPIfUpdated(true);

	this->updatePositionInTiles(true);
	this->updateHidingSpots(true);
	this->updatePropBubbleVisibility(true);

	m_floatingLabelContainer->setPosition(m_mainSp->getBoundingBox().getMidX(), m_nameplate->getBoundingBox().getMaxY() - 2);
}

void MyHero::onInactivated()
{
	if (!this->isActive())
		return;

	m_nameplate->setStaminaCyclicRollingEnabled(false);

	Hero::onInactivated();
}

void MyHero::hurl()
{
	Hero::hurl();
    
    if(!this->getData()->isAlive())
        return;
    
	if (this->isHurlAnimated())
		return;

	this->animateHurl();
	sSoundMgr->play(SOUND_THROW, false, SOUND_VOLUME_MYHERO, SOUNDID_MYHERO, 3);
}

void MyHero::showArrow(float delay, float duration)
{
	if (Node* node = this->getChildByTag(TAG_ARROW))
		node->removeFromParentAndCleanup(true);

	Sprite* arrowSp = Sprite::createWithSpriteFrameName(FRAMENAME_MY_HERO_ARROW);
	arrowSp->setAnchorPoint(Point::ANCHOR_MIDDLE_BOTTOM);
	arrowSp->setVisible(false);
	arrowSp->setPosition(m_mainSp->getBoundingBox().getMidX(), m_nameplate->getBoundingBox().getMaxY());
	arrowSp->setTag(TAG_ARROW);
	this->addChild(arrowSp, LOCAL_ZORDER_ARROW);

	Vector<FiniteTimeAction*> arrayOfActions;
	if (delay > 0)
		arrayOfActions.pushBack(DelayTime::create(delay));
	arrayOfActions.pushBack(Show::create());

	FiniteTimeAction* move = EaseQuadraticActionOut::create(MoveBy::create(ARROW_MOVE_DURATION, Vec2(0, 8)));
	uint32 times = (uint32)(duration / (ARROW_MOVE_DURATION * 2));
	Repeat* repeat = Repeat::create(Sequence::createWithTwoActions(move, move->reverse()), times);
	arrayOfActions.pushBack(repeat);
	arrayOfActions.pushBack(RemoveSelf::create());

	Sequence* action = Sequence::create(arrayOfActions);
	arrowSp->runAction(action);
}

void MyHero::toggleEquipmentBar()
{
	if (m_equipmentBar->isShown())
		m_equipmentBar->hide();
	else
	{
		float offsetY = m_magicBean->getBoundingBox().getMaxY() - m_equipmentBar->getBoundingBox().origin.y + 22;
		m_equipmentBar->show(Vec2(0, offsetY));
	}
}

bool MyHero::moveBy(float direction)
{
	MyCharacter* myChar = World::getInstance()->getMyCharacter();
	if (myChar && myChar->moveBy(direction))
		return true;

	return false;
}

void MyHero::moveStop()
{
	MyCharacter* myChar = World::getInstance()->getMyCharacter();
	if (myChar)
		myChar->moveStop();
}

void MyHero::setAimingDirection(float direction)
{
	m_aimPointer->setDirection(direction);
	float width = PROJECTILE_OBJECT_SIZE.width * m_data->getProjectileScale();
	m_aimPointer->setWidth(width);
	m_aimPointer->setVisible(true);
}

void MyHero::fire(float direction)
{
	m_aimPointer->reset();
	m_aimPointer->setVisible(false);

	MyCharacter* myChar = World::getInstance()->getMyCharacter();
	if (myChar)
	{
		TutorialService* service = TutorialService::getInstance();
		if (service->isEnabled() && sGameCenter->getLocalPlayer()->isTrainee())
		{
			if (myChar->isInCharge())
				service->triggerEvent(TUTORIAL_EVENT_CONTROL_SUPER_ATTACK);
			else
				service->triggerEvent(TUTORIAL_EVENT_CONTROL_ATTACK);
		}

		myChar->attack(direction);
	}
}

void MyHero::fire(Point const& aimPoint)
{
	float direction = FLT_MAX;
	if(aimPoint.x >= 0 && aimPoint.y >= 0)
		direction = MathTools::computeAngleInRadians(this->getData()->getPosition(), aimPoint);
	this->fire(direction);
}

void MyHero::fireStop(bool isFinal)
{
	m_aimPointer->reset();
	m_aimPointer->setVisible(false);

	if (isFinal)
	{
		MyCharacter* myChar = World::getInstance()->getMyCharacter();
		if(myChar && myChar->isInCharge())
			myChar->chargeStop();
	}
}

void MyHero::charge()
{
	MyCharacter* myChar = World::getInstance()->getMyCharacter();
	if(myChar)
		myChar->charge();
}

void MyHero::onCharacterInfo(CharacterInfo const& info)
{
	if (!World::getInstance()->isLocalPlayerAuthed(sGameCenter->getLocalPlayer()))
		return;

	LocalPlayer* localPlayer = sGameCenter->getLocalPlayer();
	std::string nickname = localPlayer->getNickname();
	if (nickname.empty())
	{
		localPlayer->setNickname(info.name());
		localPlayer->saveDataAsync();
	}
}

void MyHero::onRewardMessage(RewardMessage const& message)
{
	if (!World::getInstance()->isLocalPlayerAuthed(sGameCenter->getLocalPlayer()))
		return;

	if (message.type() == RewardMessage::REWARD_MONEY)
	{
		LocalPlayer* localPlayer = sGameCenter->getLocalPlayer();
		localPlayer->addMoney(message.value());
		localPlayer->saveDataAsync();
	}

	switch (message.type())
	{
	case RewardMessage::REWARD_XP:
		this->addFloatingLabel(StringUtils::format(sLocaleMgr->getString("floatinglabel_xp").c_str(), message.value()), Color4B(130, 70, 210, 255));
		break;
	case RewardMessage::REWARD_MONEY:
		this->addFloatingLabel(StringUtils::format(sLocaleMgr->getString("floatinglabel_gold").c_str(), message.value()), Color4B(250, 180, 5, 255));
		break;
	default:
		break;
	}
}

void MyHero::onItemPickupResult(ItemPickupResult const& result)
{
	MyCharacter* myChar = World::getInstance()->getMyCharacter();
	if (!myChar)
		return;

	PickupStatus status = static_cast<PickupStatus>(result.status());
	ObjectGuid itemGuid(result.item());

	Prop* prop = m_gameMapLayer->getGameObject<Prop>(itemGuid);
	if (prop)
	{
		ItemTemplate const* tmpl = sObjectMgr->getItemTemplate(prop->getData()->getItemId());
		NS_ASSERT(tmpl);
		if(this->hasPropBubble(tmpl))
			m_propBubble->show(tmpl, status, myChar->getMap()->getCombatGrade());
	}
}

void MyHero::onItemActionMessage(ItemActionMessage const& message)
{
	NS_ASSERT(message.owner() == this->getData()->getGuid().getRawValue());
	if (message.type() == ItemActionMessage::ITEM_RECEIVED)
	{
		this->startPropReceivedAnimation(message.item_id());
	}
}

void MyHero::saveLevelAndXPIfUpdated(bool force)
{
	DataPlayer* myChar = this->getData();
	if (myChar->hasUpdatedField(CUNIT_FIELD_LEVEL) 
		|| myChar->hasUpdatedField(CPLAYER_FIELD_EXPERIENCE) 
		|| force)
	{
		if (World::getInstance()->isLocalPlayerAuthed(sGameCenter->getLocalPlayer()))
		{
			LocalPlayer* localPlayer = sGameCenter->getLocalPlayer();
			localPlayer->setLevel(myChar->getLevel());
			localPlayer->setExperience(myChar->getExperience());
			localPlayer->saveDataAsync();
		}
	}
}

void MyHero::updatePositionInTiles(bool force)
{
	DataPlayer* myChar = this->getData();
	MapData* mapData = myChar->getMapData();
	m_isPositionInTilesDirty = false;
	if (force || myChar->hasUpdatedField(CUNIT_FIELD_POSITION))
	{
		TileCoord currCoord(mapData->getMapSize(), myChar->getPosition());
		if (m_currTileCoord != currCoord)
		{
			m_isPositionInTilesDirty = true;
			m_currTileCoord = currCoord;
		}
	}
}

void MyHero::updateHidingSpots(bool force)
{
	if (!force && !m_isPositionInTilesDirty && !this->getData()->hasUpdatedField(CPLAYER_FIELD_CONCEALMENT_STATE))
		return;

	DataPlayer* myChar = this->getData();
	MapData* mapData = myChar->getMapData();

#if DEBUG_HIDINGSPOT_SEARCHER
	auto brush = sDebugDrawer->getDrawByTag("HidingSpotSearcher");
	brush->clear();
	brush->setLineWidth(2.0f);
#endif // DEBUG_HIDINGSPOT_SEARCHER

	std::vector<HidingSpotInfo> visibleList;
	std::vector<HidingSpotInfo> outOfRangeList;
	m_hidingSpotSearcher->update(m_currTileCoord, visibleList, outOfRangeList);

	for (auto const& info : visibleList)
	{
		Color4B color;
		if (m_hidingSpotSearcher->getVisibleRange() == HidingSpotSearcher::POINT)
			color = Color4B(HIDING_SOPT_NORMAL_COLOR, HIDING_SPOT_OPACITY);
		else
			color = Color4B(HIDING_SOPT_HIGHLIGHT_COLOR, HIDING_SPOT_OPACITY);
		m_gameMapLayer->setTileColor(info.layerName, info.originCoord, color);
	}

	for (auto const& info : outOfRangeList)
		m_gameMapLayer->setTileColor(info.layerName, info.originCoord, Color4B(HIDING_SOPT_NORMAL_COLOR, 255));

	HidingSpotInfo info;
	if (mapData->getHidingSpotInfo(m_currTileCoord, info))
	{
		if (myChar->getConcealmentState() == CONCEALMENT_STATE_CONCEALED)
			m_gameMapLayer->setTileColor(info.layerName, info.originCoord, Color4B(HIDING_SOPT_NORMAL_COLOR, 255));
	}
}

void MyHero::updateAlive()
{
	if (this->getData()->hasUpdatedField(CUNIT_FIELD_IS_ALIVE))
	{
		if (!m_data->isAlive())
		{
			this->animateDead();
			m_floatingLabelContainer->setPosition(m_mainSp->getBoundingBox().getMidX(), m_mainSp->getBoundingBox().getMaxY() - 11);
			sSoundMgr->play(SOUND_PLAYER_DIED, false, SOUND_VOLUME_MYHERO, SOUNDID_MYHERO);
		}
	}
}

void MyHero::updateLevel()
{
	if (this->getData()->hasUpdatedField(CUNIT_FIELD_LEVEL))
	{
		m_heroEffects->play(EFFECT_LEVELUP);
		sSoundMgr->play(SOUND_LEVELUP, false, SOUND_VOLUME_MYHERO, SOUNDID_MYHERO);
	}
}

void MyHero::updatePropBubbleVisibility(bool force)
{
	if (!m_propBubble->isShown())
		return;

	if (this->getData()->hasUpdatedField(CPLAYER_FIELD_PICKUP_TARGET) || force)
	{
		if (this->getData()->getPickupTarget() != ObjectGuid::EMPTY)
		{
			Prop* prop = m_gameMapLayer->getGameObject<Prop>(this->getData()->getPickupTarget());
			if (prop)
			{
				ItemTemplate const* tmpl = sObjectMgr->getItemTemplate(prop->getData()->getItemId());
				NS_ASSERT(tmpl);
				if (!this->hasPropBubble(tmpl))
					m_propBubble->hide();
			}
		}
	}

	if (this->getData()->hasUpdatedField(CUNIT_FIELD_POSITION))
	{
		if (this->getData()->getPickupTarget() == ObjectGuid::EMPTY)
			m_propBubble->hide();
	}
}

void MyHero::updateMagicBeanCount()
{
	if (m_data->hasUpdatedField(CUNIT_FIELD_MAGIC_BEAN_COUNT))
	{
		if (m_data->getMagicBeanCount() > 0)
			sSoundMgr->play(SOUND_MAGICBEAN_RECEIVE);

		TutorialService* service = TutorialService::getInstance();
		if (service->isEnabled() && sGameCenter->getLocalPlayer()->isTrainee())
		{
			World* world = World::getInstance();
			if (m_data->getMagicBeanCount() >= world->getMap()->getMagicBeanCount())
				service->triggerEvent(TUTORIAL_EVENT_PICKED_UP_ALL_MAGICBEANS);
		}
	}
}

void MyHero::updateReceiveDamage()
{
	if (!m_data->isAlive())
		return;

	if (m_data->hasUpdatedField(CUNIT_FIELD_HEALTH) && m_data->hasUnitFlag(UNIT_FLAG_DAMAGED))
		sSoundMgr->play(SOUND_PLAYER_DAMAGED, false, SOUND_VOLUME_MYHERO, SOUNDID_MYHERO, 1,  2.f);
}

void MyHero::updateAimPointer(float delta)
{
	if (!m_aimPointer->isVisible())
		return;

	MapData* mapData = m_data->getMapData();
	float maxLength = this->getData()->getAttackRange();
	float length = 0;
	while (length < maxLength)
	{
		float ox = std::cos(m_aimPointer->getDirection()) * length;
		float oy = std::sin(m_aimPointer->getDirection()) * length;
		Point currPos = this->getData()->getPosition() + Vec2(ox, oy);
		TileCoord currCoord(mapData->getMapSize(), currPos);
		if (mapData->isCollidable(currCoord))
			break;

		length += TEST_BLOCK_SPACING;
	}
	length = std::min(length, maxLength);
	m_aimPointer->setLength(length);

	float offsetZ;
	float zOrder = mapData->getTileZForPos(this->getData()->getPosition(), &offsetZ) + OVERLAY_AIM_POINTER + offsetZ;
	m_aimPointer->setGlobalZOrder(zOrder);

	if (m_data->hasUpdatedField(CUNIT_FIELD_PROJECTILE_SCALE))
	{
		float width = PROJECTILE_OBJECT_SIZE.width * m_data->getProjectileScale();
		m_aimPointer->setWidth(width);
	}

	m_aimPointer->update(delta);
}

void MyHero::checkSoundTriggers()
{
	DataPlayer* myChar = this->getData();

	std::string triggeredName;
	bool triggered = myChar->getMapData()->isSoundTriggered(triggeredName, myChar->getPosition());
	if (triggered)
	{
		NSTime time = m_soundTriggerTimers[triggeredName];
		NSTime diff = time_util::getUptimeMillis() - time;
		if (time == 0 || diff > SOUND_TIMER_INTERVAL)
		{
			std::string soundRes = sSoundTriggerResources[triggeredName];
			if (sSoundMgr->play(soundRes) != AudioEngine::INVALID_AUDIO_ID)
				CCLOG("Play sound %s", soundRes.c_str());

			m_soundTriggerTimers[triggeredName] = time_util::getUptimeMillis();
		}
	}
}

void MyHero::updateGlobalZOrderOnAlive()
{
	Hero::updateGlobalZOrderOnAlive();

	m_propBubble->setGlobalZOrder(m_mainSp->getGlobalZOrder());
}

void MyHero::onItemApplicationUpdate(bool apply, uint32 itemId, int32 duration, int32 remainingTime)
{
	Hero::onItemApplicationUpdate(apply, itemId, duration, remainingTime);

	TutorialService* service = TutorialService::getInstance();
	if (service->isEnabled() && sGameCenter->getLocalPlayer()->isTrainee())
	{
		ItemTemplate const* tmpl = sObjectMgr->getItemTemplate(itemId);
		NS_ASSERT(tmpl);
		if (apply)
		{
			if (tmpl->itemClass == ITEM_CLASS_CONSUMABLE)
				service->triggerEvent(TUTORIAL_EVENT_ITEM_USED);
		}
	}
}

void MyHero::applyItem(ItemApplicationTemplate const* appTmpl, int32 duration, int32 remainingTime)
{
	Hero::applyItem(appTmpl, duration, remainingTime);

	switch (appTmpl->visualId)
	{
	case ITEM_VISUAL_STAMINA_CYCLIC_ROLLING:
		m_nameplate->setStaminaCyclicRollingEnabled(true);
		break;
	case ITEM_VISUAL_SHIELD:
		if (remainingTime >= duration)
		{
			m_heroEffects->play(EFFECT_SHIELDED);
			sSoundMgr->play(SOUND_SHIELD, false, SOUND_VOLUME_MYHERO, SOUNDID_MYHERO);
		}
		else
			m_heroEffects->play(EFFECT_SHIELDED, true);
		break;
	case ITEM_VISUAL_HIGHLIGHT_HIDING_SPOTS:
		m_hidingSpotSearcher->setVisibleRange(HidingSpotSearcher::VIEWPORT);
		this->updateHidingSpots(true);
		break;
	case ITEM_VISUAL_HEALING:
		m_heroEffects->play(EFFECT_HEALING);
		break;
	default:
		break;
	}
}

void MyHero::unapplyItem(ItemApplicationTemplate const* appTmpl)
{
	switch (appTmpl->visualId)
	{
	case ITEM_VISUAL_STAMINA_CYCLIC_ROLLING:
		m_nameplate->setStaminaCyclicRollingEnabled(false);
		break;
	case ITEM_VISUAL_SHIELD:
		m_heroEffects->play(EFFECT_SHIELDED, false, true);
		sSoundMgr->play(SOUND_UNSHIELD, false, SOUND_VOLUME_MYHERO, SOUNDID_MYHERO);
		break;
	case ITEM_VISUAL_HIGHLIGHT_HIDING_SPOTS:
		m_hidingSpotSearcher->setVisibleRange(HidingSpotSearcher::POINT);
		this->updateHidingSpots(true);
		break;
	case ITEM_VISUAL_ATTACK_STICK_CHARGE_ENABLE:
	{
		MyCharacter* myChar = World::getInstance()->getMyCharacter();
		if (myChar && myChar->isInCharge())
			myChar->chargeStop();
		break;
	}
	default:
		break;
	}

	Hero::unapplyItem(appTmpl);
}

bool MyHero::addFootprint(Point const& pos, float orient)
{
	sSoundMgr->play(SOUND_FOOTSTEP, false, SOUND_VOLUME_MYHERO, SOUNDID_MYHERO);
	return Hero::addFootprint(pos, orient);
}

void MyHero::startPropReceivedAnimation(uint32 itemId)
{
	ItemTemplate const* tmpl = sObjectMgr->getItemTemplate(itemId);
	NS_ASSERT(tmpl);

	SpriteFrame* frame = SpriteFrameCache::getInstance()->getSpriteFrameByName(StringUtils::format(CARRIED_PROP_FRAME_FORMAT, tmpl->displayId));
	if (!frame)
		return;

	SILSprite* prop = SILSprite::createWithSpriteFrame(frame);
	prop->setContentSize(RECEIVED_PROP_CONTENT_SIZE);
	prop->setAnchorPoint(Point::ANCHOR_MIDDLE);
	prop->setSilhouetted(false);
	this->addChild(prop, LOCAL_ZORDER_PROP_RECEIVED);

	prop->setPosition(m_mainSp->getBoundingBox().getMaxX() - 15, m_mainSp->getBoundingBox().getMaxY() - 9);
	prop->setScale(0.3f);
	prop->setOpacity(0);

	Spawn* scaleFade = Spawn::createWithTwoActions(ScaleTo::create(0.4f, 0.5f), FadeTo::create(0.4f, 255));
	ScaleTo* scale = ScaleTo::create(0.4f, 0.3f);
	Point dest(m_mainSp->getBoundingBox().getMidX(), m_mainSp->getBoundingBox().getMidY());
	Spawn* fadeScaleMove = Spawn::create(
		FadeTo::create(0.4f, 0),
		ScaleTo::create(0.4f, 0.2f),
		MoveTo::create(0.4f, dest),
		nullptr
	);
	Sequence* action = Sequence::create(scaleFade, scale, fadeScaleMove, RemoveSelf::create(), nullptr);
	prop->runAction(action);
}

void MyHero::addFloatingLabel(std::string const& text, Color4B const& textColor)
{
	float endScale = 3.f;
	float startInr = 0.5f, decrCount = 8;

	Spawn* spawn = Spawn::create(ScaleBy::create(0.3f, endScale), MoveBy::create(1.0f, Vec2(0, 22)), nullptr);
	float duration = spawn->getDuration();

	SILLabel* label = SILLabel::createWithSystemFont(text, DEFAULT_SYSTEM_FONT, 5 * endScale);
	label->setSilhouetted(false);
	label->setAnchorPoint(Point::ANCHOR_MIDDLE_BOTTOM);
	label->setTextColor(textColor);
	Utils::enableBoldForLabel(label);
	label->setScale(1.0f / endScale);
	label->setVisible(false);

	Vector<FiniteTimeAction*> arrayOfActions;

	auto const& children = m_floatingLabelContainer->getChildren();
	if (!children.empty())
	{
		Node* lastNode = children.back();
		ActionInterval* action = dynamic_cast<ActionInterval*>(lastNode->getActionByTag(FLOATING_LABEL_ACTION_TAG));
		if (action)
		{
			float lastDelay = action->getDuration() - duration;
			float elapsed = action->getElapsed() - lastDelay;
			float interval = MAX(0.f, startInr - (children.size() - 1) * (startInr / decrCount));
			//CCLOG("last duration: %f last elapsed: %f last delay: %f elapsed: %f interval: %f", action->getDuration(), action->getElapsed(), lastDelay, elapsed, interval);
			if (elapsed < interval)
			{
				float delay = interval - elapsed;
				//CCLOG("delay: %f", delay);
				arrayOfActions.pushBack(DelayTime::create(delay));
			}
		}
	}
	arrayOfActions.pushBack(Show::create());

	arrayOfActions.pushBack(spawn);
	arrayOfActions.pushBack(RemoveSelf::create());

	Sequence* seq = Sequence::create(arrayOfActions);
	seq->setTag(FLOATING_LABEL_ACTION_TAG);

	m_floatingLabelContainer->addChild(label);
	label->runAction(seq);
}

bool MyHero::hasPropBubble(ItemTemplate const* itemTemplate) const
{
	if (sGameCenter->getLocalPlayer()->isTrainee())
		return false;

	if (itemTemplate->itemClass != ITEM_CLASS_CONSUMABLE && itemTemplate->itemClass != ITEM_CLASS_EQUIPMENT)
		return false;

	return true;
}

NS_END
