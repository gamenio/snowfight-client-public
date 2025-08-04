#include "Prop.h"

#include "scene/SoundMgr.h"
#include "scene/gamble/Hero.h"
#include "scene/Utils.h"
#include "game/entities/DataItem.h"
#include "game/entities/DataPlayer.h"
#include "common/debugging/DebugDrawer.h"
#include "common/utils/MathTools.h"
#include "game/World.h"
#include "game/ObjectMgr.h"


NS_BEGIN

// Debug drawer
#if NS_DEBUG

#define DEBUG_COLLISION_BOX                 0
#define DEBUG_BOUNDING_BOX					0
#define DEBUG_TRAJECTORY					0
#define DEBUG_PROP_SHADOW					0

#endif // NS_DEBUG

#define PROP_FRAME_FORMAT					"prop%d.png"
#define PROP_GOLD_FRAME_FORMAT				"prop%d%d%d.png"	// Frame naming format：prop<GoldDisplayId><GoldStackSize><PartIndex>.png
#define PROP_MAGICBEAN_FRAME_FORMAT			"prop%d%d.png"		// Frame naming format：prop<MagicBeanDisplayId><MagicBeanStyle>.png
#define PROP_DEFAULT_FRAMENAME				"prop1.png"
#define EQUIP_LEVEL_FRAME_FORMAT			"equip_level%d.png"
#define FRAMENAME_SHADOW					"prop_shadow.png"
#define FRAMENAME_PICKUP_FORBIDDEN			"pickup_forbidden.png"

#define ACTION_TAG_PARABOLA					1
#define ACTION_TAG_JUMPING					1
#define ACTION_TAG_BOUNCING					1

// Element drawing offset
#define SHADOW_DRAWING_OFFSET				Vec2(-16.f, -7.0f)
#define MAIN_DRAWING_OFFSET					Vec2(-16.f, 4.0f)

#define GOLD_NUMBER_OF_PARTS				3

// Sound effect configuration
#define SOUNDID_HOLDER_MYHERO               1
#define SOUNDID_HOLDER_OTHER				2

static BounceConfig s_goldBounceConfigs[GOLD_NUMBER_OF_PARTS] = {
	{ 0.25f, 0.25f, Vec2(0, 31) },		// Part 3
	{ 0.25f, 0.17f, Vec2(0, 19) },		// Part 2
	{ 0.25f, 0.35f, Vec2(0, 53) },		// Part 1
};

#define MAX_SPRITES_IN_MAIN_NODE			GOLD_NUMBER_OF_PARTS

Prop::Prop() :
	m_data(nullptr),
	m_isJumpAnimationStopping(false),
	m_parabola(nullptr),
	m_mainNode(nullptr),
	m_shadowSp(nullptr),
	m_levelSp(nullptr),
	m_pickupProgressBar(nullptr),
	m_forbiddenSp(nullptr),
	m_nameLabel(nullptr)
{
}

Prop::~Prop()
{
	CC_SAFE_RELEASE_NULL(m_parabola);
	CC_SAFE_RELEASE_NULL(m_data);
	m_mainNode = nullptr;
	m_shadowSp = nullptr;
	m_levelSp = nullptr;
	m_pickupProgressBar = nullptr;
	m_forbiddenSp = nullptr;
	m_nameLabel = nullptr;
}

Prop* Prop::createWithData(DataItem* data)
{
	Prop *pRet = new Prop();
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

bool Prop::initWithData(DataItem* data)
{
	if (!GameObject::init())
		return false;

	CC_SAFE_RETAIN(data);
	m_data = data;

	m_shadowSp = SILSprite::createWithSpriteFrameName(FRAMENAME_SHADOW);
	m_shadowSp->setAnchorPoint(Utils::drawingOffsetToAnchorPoint(SHADOW_DRAWING_OFFSET, m_shadowSp->getContentSize()));

	m_levelSp = SILSprite::create();
	m_levelSp->setAnchorPoint(m_shadowSp->getAnchorPoint());
	m_levelSp->setVisible(false);
	this->addChild(m_levelSp);

	this->addChild(m_shadowSp);

	m_pickupProgressBar = PickupProgressBar::create();
	m_pickupProgressBar->setAnchorPoint(Point::ANCHOR_MIDDLE);
	m_pickupProgressBar->setVisible(false);
	this->addChild(m_pickupProgressBar);

	m_forbiddenSp = SILSprite::createWithSpriteFrameName(FRAMENAME_PICKUP_FORBIDDEN);
	m_forbiddenSp->setAnchorPoint(Point::ANCHOR_MIDDLE);
	m_forbiddenSp->setVisible(false);
	this->addChild(m_forbiddenSp);

	SpriteFrame* defaultFrame = SpriteFrameCache::getInstance()->getSpriteFrameByName(PROP_DEFAULT_FRAMENAME);
	m_mainNode = Node::create();
	m_mainNode->setIgnoreAnchorPointForPosition(false);
	m_mainNode->setCascadeColorEnabled(true);
	m_mainNode->setAnchorPoint(Utils::drawingOffsetToAnchorPoint(MAIN_DRAWING_OFFSET, defaultFrame->getOriginalSize()));
	m_mainNode->setContentSize(defaultFrame->getOriginalSize());
	this->addChild(m_mainNode);

	for (int32 i = 0; i < MAX_SPRITES_IN_MAIN_NODE; ++i)
	{
		SILSprite* sp = SILSprite::create();
		sp->setVisible(false);
		m_mainNode->addChild(sp);
	}

#if NS_DEBUG
	m_nameLabel = SILLabel::createWithSystemFont("", DEFAULT_SYSTEM_FONT, 14);
	m_nameLabel->setPosition(Vec2(m_mainNode->getBoundingBox().getMidX(), m_mainNode->getBoundingBox().getMaxY()));
	std::stringstream ss;
	ss << m_data->getGuid().getTypeName();
	ss << "ID:" << m_data->getGuid().getCounter();
	m_nameLabel->setString(ss.str());
	m_nameLabel->setAnchorPoint(Point::ANCHOR_MIDDLE_BOTTOM);
	m_nameLabel->setTextColor(Color4B(103, 119, 137, 255));
	Utils::enableBoldForLabel(m_nameLabel);
	this->addChild(m_nameLabel);

	World* world = World::getInstance();
	if (world->getSession() && world->getSession()->asWorldSession())
	{
		WorldSession* session = world->getSession()->asWorldSession();
		m_nameLabel->setVisible(session->hasGMPermission());
	}
#endif // NS_DEBUG

	return true;
}

void Prop::onActivated()
{
	if (this->isActive())
		return;

	GameObject::onActivated();

	ItemTemplate const* tmpl = sObjectMgr->getItemTemplate(m_data->getItemId());
	NS_ASSERT(tmpl);

	if (tmpl->itemClass == ITEM_CLASS_EQUIPMENT)
	{
		m_levelSp->setSpriteFrame(StringUtils::format(EQUIP_LEVEL_FRAME_FORMAT, tmpl->level));
		m_levelSp->setVisible(false);
	}

	switch (tmpl->itemClass)
	{
	case ITEM_CLASS_GOLD:
		this->displayGold(tmpl);
		break;
	case ITEM_CLASS_MAGIC_BEAN:
		this->displayMagicBean(tmpl);
		break;
	default:
		this->displayOtherProp(tmpl);
		break;
	}

	this->launch();
}

void Prop::onInactivated()
{
	if (!this->isActive())
		return;

	this->reset();

	GameObject::onInactivated();
}

void Prop::update(float delta)
{
	GameObject::update(delta);

	if (!this->isActive())
		return;

	this->updateShadow();

	if (!m_parabola)
	{
		this->updatePickupProgressVisibility(false);
		this->updateAvailability(false);
	}
	else
	{
		if (m_parabola->isDone())
		{
			CC_SAFE_RELEASE_NULL(m_parabola);
			this->onDropAnimationEnded();
			this->playDropSound();
		}
	}
    
	this->debugDraw();
}

void Prop::setGlobalZOrder(float globalZOrder)
{
	ItemTemplate const* tmpl = sObjectMgr->getItemTemplate(m_data->getItemId());
	NS_ASSERT(tmpl);

	auto const& children = m_mainNode->getChildren();
	for (Node* node : children)
	{
		if(node->isVisible())
			node->setGlobalZOrder(globalZOrder);
	}

	m_shadowSp->setGlobalZOrder(globalZOrder);
	if (tmpl->itemClass == ITEM_CLASS_EQUIPMENT)
		m_levelSp->setGlobalZOrder(globalZOrder);
	if(m_nameLabel)
		m_nameLabel->setGlobalZOrder(globalZOrder);

	Node::setGlobalZOrder(globalZOrder);
}

void Prop::onItemPickupResult(PickupStatus status, NSTime remainingTime)
{
	this->hidePickupProgress();
	this->hideForbiddenSymbol();

	if (status == PICKUP_STATUS_OK)
	{
		DataPlayer* myChar = World::getInstance()->getMyself();
		if (myChar && remainingTime > 0)
		{
			NSTime elapsedTime = myChar->getPickupDuration() - remainingTime;
			this->showPickupProgress(elapsedTime, myChar->getPickupDuration());
		}
	}
	else // ITEM_PICKUP_STATUS_FORBIDDEN
		this->showForbiddenSymbol();
}

void Prop::launch()
{
	if (m_data->getDropElapsed() >= m_data->getDropDuration())
		this->onDropAnimationEnded();
	else
		this->startDropAnimation();
}

void Prop::reset()
{
	this->stopDropAnimation();
	this->stopJumpAnimation(false, false);

	this->hidePickupProgress();
	this->hideForbiddenSymbol();

	if(m_nameLabel)
		m_nameLabel->setVisible(true);
	m_mainNode->setPosition(Point::ZERO);
	auto const& children = m_mainNode->getChildren();
	for (Node* node: children)
		node->setVisible(false);
	m_levelSp->setVisible(false);

	m_shadowSp->setVisible(true);
	m_shadowSp->setScale(1.0f);

	this->setScale(1.0f);
	this->setPosition(m_data->getPosition());
	this->setVisible(false);
}

void Prop::displayGold(ItemTemplate const* tmpl)
{
	GoldStackSize stackSize = this->getGoldStackSize(m_data->getCount());
	auto const& children = m_mainNode->getChildren();
	for (int32 i = GOLD_NUMBER_OF_PARTS - 1; i >= 0; --i)
	{
		int32 spriteIndex = GOLD_NUMBER_OF_PARTS - 1 - i;
		NS_ASSERT(spriteIndex < children.size());
		Sprite* sp = dynamic_cast<Sprite*>(children.at(spriteIndex));
		NS_ASSERT(sp != nullptr);
		sp->setSpriteFrame(StringUtils::format(PROP_GOLD_FRAME_FORMAT, tmpl->displayId, stackSize, i));
		sp->setPosition(m_mainNode->getContentSize() / 2);
		sp->setVisible(true);
	}
}

void Prop::displayMagicBean(ItemTemplate const* tmpl)
{
	auto const& children = m_mainNode->getChildren();
	auto it = children.begin();
	NS_ASSERT(it != children.end());
	Sprite* sp = dynamic_cast<Sprite*>(*it);
	NS_ASSERT(sp != nullptr);

	MagicBeanStyle style = MAGICBEAN_SINGLE;
	if (m_data->getCount() > 1)
		style = MAGICBEAN_STACKED;
	sp->setSpriteFrame(StringUtils::format(PROP_MAGICBEAN_FRAME_FORMAT, tmpl->displayId, style));
	sp->setPosition(m_mainNode->getContentSize() / 2);
	sp->setVisible(true);
}

void Prop::displayOtherProp(ItemTemplate const* tmpl)
{
	auto const& children = m_mainNode->getChildren();
	auto it = children.begin();
	NS_ASSERT(it != children.end());
	Sprite* sp = dynamic_cast<Sprite*>(*it);
	NS_ASSERT(sp != nullptr);

	sp->setSpriteFrame(StringUtils::format(PROP_FRAME_FORMAT, tmpl->displayId));
	sp->setPosition(m_mainNode->getContentSize() / 2);
	sp->setVisible(true);
}

void Prop::startDropAnimation()
{
	this->stopDropAnimation();

	BezierCurveConfig const& config = m_data->getTrajectory();
#if DEBUG_TRAJECTORY
	Brush* debugDraw = sDebugDrawer->getDrawByTag("Prop.Trajectory");
	config.draw(debugDraw->getDrawNode());
#endif // DEBUG_TRAJECTORY

	m_shadowSp->setVisible(false);
	if(m_nameLabel)
		m_nameLabel->setVisible(false);
	this->setPosition(config.startPosition);
	this->setScale(0.1f);
	this->setVisible(false);

	// Parabola animation
	float delay = time_util::toGameTimeSeconds(m_data->getDropDuration() - ITEM_PARABOLA_DURATION);
	float duration = time_util::toGameTimeSeconds(ITEM_PARABOLA_DURATION);
	float elapsed = time_util::toGameTimeSeconds(m_data->getDropElapsed());
	BezierCurve* bezier = BezierCurve::create(duration, config);
	ScaleTo* scale = ScaleTo::create(duration, 1.0f);
	Spawn* spawn = Spawn::createWithTwoActions(bezier, scale);
	CC_ASSERT(!m_parabola);
	Vector<FiniteTimeAction*> arrayOfActions;
	if (delay > 0)
		arrayOfActions.pushBack(DelayTime::create(delay));
	arrayOfActions.pushBack(Show::create());
	arrayOfActions.pushBack(spawn);
	m_parabola = Sequence::create(arrayOfActions);
	CC_SAFE_RETAIN(m_parabola);
	m_parabola->setTag(ACTION_TAG_PARABOLA);
	this->runAction(m_parabola);
	m_parabola->step(0);
	m_parabola->step(elapsed);

	auto const& children = m_mainNode->getChildren();
	for (ssize_t i = 0; i < children.size(); ++i)
	{
		BounceConfig const& config = s_goldBounceConfigs[i];
		this->startBounceAnimation(children.at(i), config, elapsed);
	}
}

void Prop::stopDropAnimation()
{
	auto const& children = m_mainNode->getChildren();
	for (Node* node: children)
	{
		if (Action* action = node->getActionByTag(ACTION_TAG_BOUNCING))
			node->stopAction(action);
	}

	if (m_parabola)
	{
		this->stopAction(m_parabola);
		CC_SAFE_RELEASE_NULL(m_parabola);
	}
}

void Prop::onDropAnimationEnded()
{
	this->updateAvailability(true);
	this->updatePickupProgressVisibility(true);
}

void Prop::playDropSound()
{
	DataPlayer* myChar = World::getInstance()->getMyself();
	if (!myChar)
		return;

	ItemTemplate const* tmpl = sObjectMgr->getItemTemplate(m_data->getItemId());
	NS_ASSERT(tmpl);
	int32 soundId = SOUNDID_HOLDER_OTHER;
	if (m_data->getHolder() == myChar->getGuid())
		soundId = SOUNDID_HOLDER_MYHERO;

	switch (tmpl->itemClass)
	{
	case ITEM_CLASS_GOLD:
		sSoundMgr->play(SOUND_COINS_DROP, false, 1.0f, soundId, 1);
		break;
	default:
		sSoundMgr->play(SOUND_ITEM_DROP, false, 1.0f, soundId, 1);
		break;
	}
}

void Prop::startJumpAnimation()
{
	if (this->isJumpAnimationRunning())
	{
		m_isJumpAnimationStopping = false;
		return;
	}

	ItemTemplate const* tmpl = sObjectMgr->getItemTemplate(m_data->getItemId());
	NS_ASSERT(tmpl);

	this->updatePosition();
	this->setVisible(true);

	if (tmpl->itemClass == ITEM_CLASS_EQUIPMENT)
		m_levelSp->setVisible(true);

	if(m_nameLabel)
		m_nameLabel->setVisible(true);
	m_shadowSp->setVisible(true);
	m_shadowSp->setScale(1.0f);
	m_mainNode->setPosition(Point::ZERO);

	float delay = 0.f;
	auto const& children = m_mainNode->getChildren();
	for (Node* node : children)
	{
		ActionInterval* action = dynamic_cast<ActionInterval*>(node->getActionByTag(ACTION_TAG_BOUNCING));
		if (action)
		{
			float remainder = action->getDuration() - action->getElapsed();
			delay = std::max(remainder, delay);
		}
	}

	Action* action = nullptr;

	Vector<FiniteTimeAction*> arrayOfActions;
	if (delay > 0.f)
		arrayOfActions.pushBack(DelayTime::create(delay));

	CallFunc* callFunc = CallFunc::create(CC_CALLBACK_0(Prop::onJumpCompleted, this));
	MoveBy* move = MoveBy::create(0.7f, Vec2(0, 5));
	Repeat* repeat = Repeat::create(Sequence::create(move, move->reverse(), callFunc, nullptr), std::pow(2, 30));
	arrayOfActions.pushBack(repeat);
	action = Sequence::create(arrayOfActions);
	action->setTag(ACTION_TAG_JUMPING);
	m_mainNode->runAction(action);

	ScaleBy* scale = ScaleBy::create(0.7f, 0.7f);
	action = RepeatForever::create(Sequence::create(scale, scale->reverse(), nullptr));
	action->setTag(ACTION_TAG_JUMPING);
	m_shadowSp->runAction(action);
}

void Prop::stopJumpAnimation(bool restore, bool delayed)
{
	if (!this->isJumpAnimationRunning())
		return;

	if(delayed)
		m_isJumpAnimationStopping = delayed;
	else
	{
		if (Action* action = m_mainNode->getActionByTag(ACTION_TAG_JUMPING))
			m_mainNode->stopAction(action);
		if (Action* action = m_shadowSp->getActionByTag(ACTION_TAG_JUMPING))
			m_shadowSp->stopAction(action);

		m_isJumpAnimationStopping = false;
	}

	if (restore)
	{
		m_mainNode->setPosition(Point::ZERO);
		m_shadowSp->setVisible(true);
		m_shadowSp->setScale(1.0f);
	}
}

bool Prop::isJumpAnimationRunning() const
{
	return m_mainNode->getActionByTag(ACTION_TAG_JUMPING) != nullptr;
}

void Prop::onJumpCompleted()
{
	if (!m_isJumpAnimationStopping)
		return;

	this->stopJumpAnimation(true, false);
	m_isJumpAnimationStopping = false;
}

void Prop::showPickupProgress(NSTime elapsedTime, NSTime duration)
{
	m_pickupProgressBar->setVisible(true);
	m_pickupProgressBar->start(elapsedTime, duration);
}

void Prop::hidePickupProgress()
{
	if (!m_pickupProgressBar->isVisible())
		return;

	m_pickupProgressBar->stop();
	m_pickupProgressBar->setVisible(false);
}

void Prop::showForbiddenSymbol()
{
	m_forbiddenSp->setVisible(true);
}

void Prop::hideForbiddenSymbol()
{
	m_forbiddenSp->setVisible(false);
}

void Prop::updatePosition()
{
    DataItem* data = this->getData()->asDataItem();
    this->setPosition(data->getPosition());

	float offsetZ;
	float newZ = data->getMapData()->getTileZForPos(data->getPosition(), &offsetZ);
	newZ += offsetZ;

	this->setGlobalZOrder(newZ);
}

void Prop::updateShadow()
{
	if (!m_parabola || m_parabola->isDone())
		return;

	BezierCurveConfig const& config = m_data->getTrajectory();
	Point startPos = config.startPosition;
	Point endPos = config.startPosition + config.endPosition;

	// Calculate the starting Y coordinate of the shadow
	Point shadowStartPos;
	shadowStartPos.x = startPos.x;
	shadowStartPos.y = startPos.y - m_data->getLaunchCenter().y;

	// Calculate the current position of the shadow
	float scale = std::max(0.f, std::min(1.f, m_parabola->getElapsed() / m_parabola->getDuration()));
	float dx = (endPos.x - shadowStartPos.x) * scale;
	float dy = (endPos.y - shadowStartPos.y) * scale;
	Point shadowCurrPos(shadowStartPos.x + dx, shadowStartPos.y + dy);

#if NS_DEBUG

	Brush* debugDraw;
	debugDraw = nullptr;

#if DEBUG_PROP_SHADOW
	debugDraw = sDebugDrawer->getDrawByTag("Prop.Shadow");
	debugDraw->drawLine(shadowStartPos, endPos, Color4F::GRAY);
	debugDraw->drawLine(shadowStartPos, shadowCurrPos, Color4F::ORANGE);
	debugDraw->drawLine(startPos, shadowStartPos, Color4F::BLUE);
#endif // DEBUG_PROP_SHADOW

#endif // NS_DEBUG

	// Calculate Z coordinate
	float newZ;
	MapData const* mapData = m_data->getMapData();
	Size mapSize = mapData->getMapSize();
	TileCoord coord(mapSize, shadowCurrPos);
	if (mapData->isPenetrable(coord))
	{
		// Ensure that prop are displayed above penetrable building
		newZ = mapData->getMaxTileZ();
	}
	else
	{
		float offsetZ;
		newZ = mapData->getTileZForPos(shadowCurrPos, &offsetZ);
		newZ += offsetZ;
	}

	auto const& children = m_mainNode->getChildren();
	for (Node* node : children)
		node->setGlobalZOrder(newZ);
}

void Prop::updateAvailability(bool force)
{
	if (m_data->hasUpdatedField(CITEM_FIELD_IS_AVAILABLE) || force)
	{
		if (m_data->isAvailable())
		{
			m_mainNode->setColor(Color3B::WHITE);
			this->startJumpAnimation();
		}
		else
		{
			m_mainNode->setColor(Color3B(160, 160, 160));

			if (force)
				this->startJumpAnimation();
			this->stopJumpAnimation(false, true);
		}

		ItemTemplate const* tmpl = sObjectMgr->getItemTemplate(m_data->getItemId());
		NS_ASSERT(tmpl);
		if (tmpl->itemClass == ITEM_CLASS_EQUIPMENT)
			m_levelSp->setColor(m_mainNode->getColor());
	}
}

void Prop::updatePickupProgressVisibility(bool force)
{
	DataPlayer* myChar = World::getInstance()->getMyself();
	if (!myChar)
		return;

	if (myChar->hasUpdatedField(CPLAYER_FIELD_PICKUP_TARGET) || force)
	{
		if (myChar->getPickupTarget() != this->getData()->getGuid())
		{
			this->hidePickupProgress();
			this->hideForbiddenSymbol();
		}
	}
}

Prop::GoldStackSize Prop::getGoldStackSize(int32 golds) const
{
	if (golds < 20)
		return GOLD_STACK_TINY;
	else if (golds < 50)
		return GOLD_STACK_SMALL;
	else if (golds < 100)
		return GOLD_STACK_MEDIUM;
	else if (golds < 200)
		return GOLD_STACK_LARGE;
	else
		return GOLD_STACK_HUGE;
}

void Prop::startBounceAnimation(Node* node, BounceConfig const& config, float elapsed)
{
	ActionInterval* riseMove = EaseQuadraticActionOut::create(MoveBy::create(config.riseDuration, config.deltaPosition));
	ActionInterval* fallMove = EaseQuadraticActionIn::create(MoveBy::create(config.fallDuration, -config.deltaPosition));
	Action* action = Sequence::create(riseMove, fallMove, nullptr);
	action->setTag(ACTION_TAG_BOUNCING);
	node->runAction(action);
	action->step(0);
	action->step(elapsed);
}

void Prop::debugDraw()
{
#if NS_DEBUG

	Brush* debugDraw;
	debugDraw = nullptr;

#if DEBUG_COLLISION_BOX

	debugDraw = sDebugDrawer->getDrawByTag("Prop.CollisionBox");
	debugDraw->setClearing(true);

	// Draw object shape
	debugDraw->drawRect(m_data->getBoundingBox().origin, Vec2(m_data->getBoundingBox().getMaxX(), m_data->getBoundingBox().getMaxY()), Color4F::RED);

	// Draw object shape in map
	float scaleX = std::sqrt(2);
	debugDraw->drawCircle(m_data->getPosition(), m_data->getObjectRadiusInMap(), scaleX, scaleX / 2, Color4F::RED);
	debugDraw->drawPoint(m_data->getPosition(), 2.0f, Color4F::RED);
#endif // DEBUG_COLLISION_BOX

#if DEBUG_BOUNDING_BOX
	debugDraw = sDebugDrawer->getDrawByTag("Prop.BoundingBox");
	debugDraw->setClearing(true);
	Rect rect;
	rect.origin.x = this->getPositionX() + m_mainSp->getBoundingBox().getMinX();
	rect.origin.y = this->getPositionY() + m_mainSp->getBoundingBox().getMinY();
	rect.size = m_mainSp->getBoundingBox().size;
	debugDraw->drawRect(rect.origin, Vec2(rect.getMaxX(), rect.getMaxY()), Color4F::GREEN);
#endif // DEBUG_BOUNDING_BOX

#endif // NS_DEBUG
}

NS_END

