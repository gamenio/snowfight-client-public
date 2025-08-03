#include "Hero.h"

#include "game/entities/updates/ObjectUpdateFields.h"
#include "game/entities/DataPlayer.h"
#include "game/behaviors/SharedUnitDefines.h"

#include "common/debugging/DebugDrawer.h"
#include "common/utils/MathTools.h"
#include "game/GameConfig.h"
#include "game/ObjectMgr.h"
#include "scene/ObjectPools.h"
#include "scene/SoundMgr.h"
#include "scene/GameMapLayer.h"
#include "Footprint.h"
#include "GambleDefines.h"

NS_BEGIN


// Debug drawer
#if NS_DEBUG

#define DEBUG_COLLISION_BOX					0
#define DEBUG_BOUNDING_BOX					0
#define DEBUG_ATTACK_MAX_RANGE				0
#define DEBUG_LAUNCH_POS					0

#endif // NS_DEBUG

// 动作动画配置
// 帧命名格式：role<ComponentType><Style><BasicMovement><ComponentMovement><Direction><FrameIndex>.png
#define MOVEMENT_FRAME_FORMAT			"role%d%d%d%d%d%02d.png"
#define SCHEDULE_KEY_STAND_ANIM			"StandAnim"
#define SCHEDULE_KEY_WALK_ANIM			"WalkAnim"
#define COMPONENT_ACTION_TAG			1
#define BODY_FRAME_DEFAULT				"role0100007.png"
#define ARM_FRAME_DEFAULT				"role1101007.png"
#define SNOWBALL_FRAME_DEFAULT			"role2102007.png"

// 部件绘图偏移量
#define COMPONENT_DRAWING_OFFSET_ALIVE		Vec2(-32.0f, -11.0f)
#define COMPONENT_DRAWING_OFFSET_DEAD		Vec2(-32.0f, -21.0f)

// 行走动画配置
#define WALK_ANIM_FRAMES				8		// 英雄行走动画帧数
#define WALK_SPEED_IN_12FPS				40.0f	// 在帧率为12fps下的移动速度，调整数值将影响行走动画的帧速率
#define WALK_STEPLENGTH					12		// 每步跨越的距离，单位：points
#define WALK_STAND_POSE_FRAMEINDEX		(WALK_ANIM_FRAMES - 1)

// 站立动画配置
#define STAND_HURL_POSE_FRAMEINDEX		0
#define STAND_HURL_POSE_DURATION		0.1f		// 投掷姿势持续时间

// 受伤害动画配置
#define DAMAGED_POSE_FRAMEINDEX			0
#define DAMAGED_POSE_DURATION			0.2f		// 受伤害姿势持续时间

// 死亡动画配置
#define DEAD_ANIM_FRAMES				9
#define DEAD_POSE_FRAMEINDEX			(DEAD_ANIM_FRAMES - 1)
#define DEAD_FELL_DOWN_FRAMEINDEX		5

// 表情
#define SMILEY_FRAMENAME_FORMAT			"smiley%d.png"

Hero::Hero() :
	m_mainSp(nullptr),
	m_armSp(nullptr),
	m_snowballSp(nullptr),
	m_nameplate(nullptr),
	m_heroEffects(nullptr),
	m_smiley(nullptr),
	m_magicBean(nullptr),
	m_animationFrameDisplayedListener(nullptr),
	m_currBasicMovement(BASIC_MOVEMENT_STAND),
	m_currArmMovement(ARM_MOVEMENT_BASIC),
	m_isConcealing(false),
	m_isWalkAnimDirty(false),
	m_currStepPos(Point::ZERO),
	m_currStepLength(0)
{
}


Hero::~Hero()
{
	this->unregisterAnimationFrameDisplayedListener();
	this->stopAllAnimations();
	this->stopAllDelayedMovements();

	CC_SAFE_RELEASE_NULL(m_heroEffects);

	m_mainSp = nullptr;
	m_armSp = nullptr;
	m_snowballSp = nullptr;
	m_nameplate = nullptr;
	m_smiley = nullptr;
	m_magicBean = nullptr;
}	

Hero* Hero::createWithData(DataUnit* data)
{
	Hero *pRet = new Hero();
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

bool Hero::initWithData(DataUnit* data)
{
	if (!Role::initWithData(data))
		return false;

	m_heroEffects = HeroEffects::create();
	CC_SAFE_RETAIN(m_heroEffects);
	this->addChild(m_heroEffects->getBackEffectNode());

	m_mainSp = SILSprite::createWithSpriteFrameName(BODY_FRAME_DEFAULT);
	m_mainSp->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
	m_mainSp->setPosition(COMPONENT_DRAWING_OFFSET_ALIVE);
	this->addChild(m_mainSp);

	m_armSp = SILSprite::createWithSpriteFrameName(ARM_FRAME_DEFAULT);
	m_armSp->setAnchorPoint(m_mainSp->getAnchorPoint());
	m_armSp->setPosition(m_mainSp->getPosition());
	this->addChild(m_armSp);

	m_snowballSp = SILSprite::createWithSpriteFrameName(SNOWBALL_FRAME_DEFAULT);
	this->addChild(m_snowballSp);

	m_nameplate = Nameplate::create(m_data);
	m_nameplate->setAnchorPoint(Point::ANCHOR_MIDDLE_BOTTOM);
	m_nameplate->setPosition(m_mainSp->getBoundingBox().getMidX(), m_mainSp->getBoundingBox().getMaxY() - 2.0f);
	this->addChild(m_nameplate);

	m_magicBean = MagicBean::create();
	m_magicBean->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
	m_magicBean->setPosition(m_mainSp->getBoundingBox().getMidX() - 12, m_nameplate->getBoundingBox().getMaxY());
	m_magicBean->setVisible(false);
	this->addChild(m_magicBean);

	m_heroEffects->updatePosition(m_mainSp->getPosition(), m_mainSp->getContentSize());
	this->addChild(m_heroEffects->getFrontEffectNode());

	m_smiley = SILSprite::createWithSpriteFrameName(StringUtils::format(SMILEY_FRAMENAME_FORMAT, SMILEY_LAUGH));
	m_smiley->setAnchorPoint(Point::ANCHOR_MIDDLE_BOTTOM);
	m_smiley->setEffectOpacity(255);
	m_smiley->setVisible(false);
	this->addChild(m_smiley);

	this->initAnimations();

	return true;

}

void Hero::onActivated()
{
	if (this->isActive())
		return;

	Role::onActivated();

	this->registerAnimationFrameDisplayedListener();

	if (m_data->isAlive())
		this->animateAlive();
	else
		this->animateDead(true);
	this->updatePosition(true);
	this->updateFootprints(true);
	this->updateSmileyVisibility(true);
	this->updateMagicBeanCount(true);
	m_nameplate->updateElements(true);

	this->setVisible(true);
}

void Hero::onInactivated()
{
	if (!this->isActive())
		return;

	m_nameplate->setVisible(false);
	m_magicBean->setVisible(false);
	m_heroEffects->stopAll();
	this->stopAllAnimations();
	this->stopAllDelayedMovements();

	this->unregisterAnimationFrameDisplayedListener();
	m_currStepPos = Point::ZERO;
	m_currStepLength = 0;
	m_isWalkAnimDirty = false;

	this->setVisible(false);

	Role::onInactivated();
}

void Hero::startWalkAnimation(ComponentType componentType, uint32 style, int32 componentMovement)
{
	uint64 animId = this->generateAnimationId(componentType, style, BASIC_MOVEMENT_WALK, componentMovement, m_data->getDirection());
	Animation* animation = m_animationSet.at(animId);
	NS_ASSERT(animation != nullptr);

	float delayPerUnit = this->calcWalkFrameDelay(m_data->getMoveSpeed());
	animation->setDelayPerUnit(delayPerUnit);
	this->runAnimation(componentType, animation);
}

void Hero::startDeadAnimation()
{
	uint64 animId = this->generateAnimationId(COMPONENT_BODY, m_data->getDisplayId(), BASIC_MOVEMENT_DEAD, COMPONENT_MOVEMENT_NONE, m_data->getDirection());
	Animation* animation = m_animationSet.at(animId);
	NS_ASSERT(animation != nullptr);
	this->runAnimation(COMPONENT_BODY, animation);
}

void Hero::runAnimation(ComponentType componentType, Animation* animation)
{
	Sprite* target = this->getComponent(componentType);
	NS_ASSERT(target != nullptr);

	Action* action = target->getActionByTag(COMPONENT_ACTION_TAG);
	if (action)
		target->stopAction(action);

	if (target->isVisible())
		action = Animate::create(animation);
	else
		action = Spawn::createWithTwoActions(Show::create(), Animate::create(animation));

	action->setTag(COMPONENT_ACTION_TAG);
	target->runAction(action);
}

void Hero::stopAllAnimations()
{
	this->stopAnimation(COMPONENT_BODY);
	this->stopAnimation(COMPONENT_ARM);
	this->stopAnimation(COMPONENT_SNOWBALL);
}

void Hero::stopAnimation(ComponentType componentType)
{
	Sprite* target = this->getComponent(componentType);
	NS_ASSERT(target != nullptr);

	if (Action* action = target->getActionByTag(COMPONENT_ACTION_TAG))
		target->stopAction(action);
}

Sprite* Hero::getComponent(ComponentType componentType)
{
	Sprite* target = nullptr;
	switch (componentType)
	{
	case COMPONENT_BODY:
		target = m_mainSp;
		break;
	case COMPONENT_ARM:
		target = m_armSp;
		break;
	case COMPONENT_SNOWBALL:
		target = m_snowballSp;
		break;
	}

	return target;
}

void Hero::updateComponentSizeAndPosition(ComponentType componentType, std::string const& frameName)
{
	auto transformCache = ComponentTransformCache::getInstance();
	Point position = transformCache->getComponentPosition(componentType, frameName);
	Sprite* component = this->getComponent(componentType);
	component->setContentSize(m_mainSp->getContentSize());
	Point pos;
	pos.x = m_mainSp->getBoundingBox().getMinX() + position.x;
	pos.y = m_mainSp->getBoundingBox().getMinY() + position.y;
	component->setPosition(pos);

	//CCLOG("Hero::updateComponentSizeAndPosition componentType: %d, contentSize: [%f, %f] position: [%f, %f]", componentType, component->getContentSize().width, component->getContentSize().height, component->getPosition().x, component->getPosition().y);
}

uint64 Hero::generateAnimationId(ComponentType componentType, uint32 style, BasicMovement basicMovement, int32 componentMovement, uint8 dir)
{
	uint64 animId = 0;
	animId |= ((uint64)style << 32);				// 样式 占32位，值范围：0-UINT32_MAX
	animId |= ((uint64)componentType << 28);		// 部件 占4位，值范围：0-15
	animId |= ((uint64)basicMovement << 24);		// 基础动作 占4位，值范围：0-15
	animId |= ((uint64)componentMovement << 20);	// 部件动作 占4位，值范围：0-15
	animId |= ((uint64)dir << 16);					// 方向 占4位，值范围：0-15

	return animId;
}

void Hero::updatePosition(bool force)
{
	if (!force && !m_data->hasUpdatedField(CUNIT_FIELD_POSITION))
		return;

	this->setPosition(m_data->getPosition());
	if(m_data->isAlive())
		this->updateGlobalZOrderOnAlive();

	TileCoord currCoord(m_data->getMapData()->getMapSize(), m_data->getPosition());
	if (m_data->getMapData()->isConcealable(currCoord))
		this->conceal();
	else
		this->expose();
}

void Hero::initAnimations()
{
	this->createWalkAnimation(COMPONENT_BODY, m_data->getDisplayId());
	this->createWalkAnimation(COMPONENT_ARM, m_data->getDisplayId(), ARM_MOVEMENT_BASIC);
	this->createWalkAnimation(COMPONENT_ARM, m_data->getDisplayId(), ARM_MOVEMENT_HANDUP, true);
	this->createWalkAnimation(COMPONENT_SNOWBALL, SNOWBALL_STYLE_SQUARE, ARM_MOVEMENT_HANDUP);

	this->createDeadAnimation();
}

Animation* Hero::createAnimation(ComponentType componentType, uint32 style, BasicMovement basicMovement, int32 componentMovement, uint8 dir, int32 numOfFrames, float delayPerUnit, bool repeated, bool listenFrameDisplayedEvent)
{
	Vector<AnimationFrame*> animFrameList;
	for (int32 i = 0; i < numOfFrames; ++i)
	{
		std::string frameName = StringUtils::format(MOVEMENT_FRAME_FORMAT, componentType, style, basicMovement, componentMovement, dir, i);
		SpriteFrame* frame = SpriteFrameCache::getInstance()->getSpriteFrameByName(frameName);
		NS_ASSERT(frame != nullptr);
		ValueMap userInfo;
		if (listenFrameDisplayedEvent)
		{
			userInfo["BasicMovement"] = basicMovement;
			userInfo["ComponentType"] = componentType;
			userInfo["ComponentMovement"] = componentMovement;
			userInfo["FrameName"] = frameName;
			userInfo["FrameIndex"] = i;
		}
		AnimationFrame* animFrame = AnimationFrame::create(frame, 1.0f, userInfo);
		animFrameList.pushBack(animFrame);
	}
	uint32 loops = 1;
	if (repeated)
		loops = UINT_MAX;
	Animation* animation = Animation::create(animFrameList, delayPerUnit, loops);

	return animation;
}

void Hero::createWalkAnimation(ComponentType componentType, uint32 style, int32 componentMovement, bool listenFrameDisplayedEvent)
{
	float delayPerUnit = this->calcWalkFrameDelay(m_data->getMoveSpeed());
	for (uint8 dir = 0; dir < DataUnit::MAX_DIRECTIONS; ++dir)
	{
		Animation* animation = this->createAnimation(componentType, style, BASIC_MOVEMENT_WALK, componentMovement, dir, WALK_ANIM_FRAMES, delayPerUnit, true, listenFrameDisplayedEvent);
		uint64 animId = this->generateAnimationId(componentType, style, BASIC_MOVEMENT_WALK, componentMovement, dir);
		m_animationSet.insert(animId, animation);
	}

}

void Hero::createDeadAnimation()
{
	for (uint8 dir = 0; dir < DataUnit::MAX_DIRECTIONS; ++dir)
	{
		Animation* animation = this->createAnimation(COMPONENT_BODY, m_data->getDisplayId(), BASIC_MOVEMENT_DEAD, COMPONENT_MOVEMENT_NONE, dir, DEAD_ANIM_FRAMES, ANIM_NORMAL_FRAME_DELAY, false, true);
		uint64 animId = this->generateAnimationId(COMPONENT_BODY, m_data->getDisplayId(), BASIC_MOVEMENT_DEAD, COMPONENT_MOVEMENT_NONE, dir);
		m_animationSet.insert(animId, animation);
	}
}

float Hero::calcWalkFrameDelay(int32 moveSpeed)
{
    float delay = ANIM_NORMAL_FRAME_DELAY / ((float)moveSpeed / WALK_SPEED_IN_12FPS);
    return delay;
}

void Hero::adjustWalkAnimFrameDelay()
{
	if (m_currBasicMovement != BASIC_MOVEMENT_WALK)
		return;

	this->stopAllAnimations();

	this->startWalkAnimation(COMPONENT_BODY, m_data->getDisplayId(), COMPONENT_MOVEMENT_NONE);
	if (m_data->hasMovementFlag(MOVEMENT_FLAG_HANDUP))
	{
		m_snowballSp->setScale(this->getData()->getProjectileScale());
		this->startWalkAnimation(COMPONENT_SNOWBALL, SNOWBALL_STYLE_SQUARE, ARM_MOVEMENT_HANDUP);
		this->startWalkAnimation(COMPONENT_ARM, m_data->getDisplayId(), ARM_MOVEMENT_HANDUP);
	}
	else
	{
		this->startWalkAnimation(COMPONENT_ARM, m_data->getDisplayId(), ARM_MOVEMENT_BASIC);
	}
}

void Hero::setAnimationFrame(ComponentType componentType, uint32 style, BasicMovement basicMovement, int32 componentMovement, int32 frameIndex)
{
	uint8 dir = m_data->getDirection();
	std::string frameName = StringUtils::format(MOVEMENT_FRAME_FORMAT, componentType, style, basicMovement, componentMovement, dir, frameIndex);
	SpriteFrame* frame = SpriteFrameCache::getInstance()->getSpriteFrameByName(frameName);

	NS_ASSERT(frame != nullptr);
	switch (componentType)
	{
	case COMPONENT_BODY:
		m_mainSp->setVisible(true);
		m_mainSp->setSpriteFrame(frame);
		break;
	case COMPONENT_ARM:
		m_armSp->setVisible(true);
		m_armSp->setSpriteFrame(frame);
		break;
	case COMPONENT_SNOWBALL:
		m_snowballSp->setVisible(true);
		m_snowballSp->setSpriteFrame(frame);
		m_snowballSp->setContentSize(m_mainSp->getContentSize());
		break;
	}

	this->updateComponentWhenSpriteFrameChanged(basicMovement, componentType, componentMovement, frameName, frameIndex);
}

void Hero::update(float delta)
{
	Role::update(delta);

	if (!this->isActive())
		return;

	m_nameplate->updateElements(false);

	this->updatePosition(false);
	this->updateMovementAnimation();
	this->updateFootprints(false);
	this->updateSmileyVisibility(false);
	this->updateMagicBeanCount(false);

	this->debugDraw();
}

void Hero::onEnter()
{
	Role::onEnter();
}

void Hero::onItemApplicationUpdate(bool apply, uint32 itemId, int32 duration, int32 remainingTime)
{
	ItemTemplate const* tmpl = sObjectMgr->getItemTemplate(itemId);
	NS_ASSERT(tmpl);
	ItemApplicationTemplate const* appTmpl = sObjectMgr->getItemApplicationTemplate(tmpl->appId);
	NS_ASSERT(appTmpl);

	if (apply)
		this->applyItem(appTmpl, duration, remainingTime);
	else
		this->unapplyItem(appTmpl);
}

void Hero::debugDraw()
{
#if NS_DEBUG
    
    Brush* debugDraw;
    debugDraw = nullptr;

#if DEBUG_COLLISION_BOX

	debugDraw = sDebugDrawer->getDrawByTag("Hero.CollisionBox");
	debugDraw->setClearing(true);
	debugDraw->drawRect(m_data->getBoundingBox().origin, Vec2(m_data->getBoundingBox().getMaxX(), m_data->getBoundingBox().getMaxY()), Color4F::RED);

	debugDraw->drawCircle(m_data->getPosition(), m_data->getObjectRadiusInMap(), std::sqrt(2), std::sqrt(2) / 2, Color4F::RED);
    debugDraw->drawPoint(m_data->getPosition(), 2.0f, Color4F::RED);
#endif 

#if DEBUG_ATTACK_MAX_RANGE
	debugDraw = sDebugDrawer->getDrawByTag("Hero.AttackMaxRange");
	debugDraw->setClearing(true);
	debugDraw->drawCircle(m_data->getPosition(), m_data->getAttackRange(), Color4F::RED);
	debugDraw->drawCircle(m_data->getPosition(), m_data->getAttackRange() - MISS_DISTANCE, Color4F::YELLOW);
#endif 

#if DEBUG_LAUNCH_POS
	Point center = m_data->getPosition() + m_data->getLaunchCenter();
	debugDraw = sDebugDrawer->getDrawByTag("Hero.LaunchPos");
	debugDraw->setClearing(true);
	debugDraw->drawCircle(center, m_data->getLaunchRadiusInMap(), std::sqrt(2), std::sqrt(2) / 2, Color4F::GREEN);
	//debugDraw->drawPoint(startPos, 4, Color4F::YELLOW); // 抛出的起始点
	debugDraw->drawPoint(center, 4, Color4F::YELLOW);
	debugDraw->drawCircle(m_data->getPosition(), m_data->getLaunchRadiusInMap(), std::sqrt(2), std::sqrt(2) / 2, Color4F::ORANGE);
	//debugDraw->drawPoint(m_data->getPosition(), 4, Color4F::YELLOW);
#endif 

#if DEBUG_BOUNDING_BOX
	debugDraw = sDebugDrawer->getDrawByTag("Hero.BoundingBox");
	debugDraw->setClearing(true);
	Rect rect;
	rect.origin.x = this->getPositionX() + m_mainSp->getBoundingBox().getMinX();
	rect.origin.y = this->getPositionY() + m_mainSp->getBoundingBox().getMinY();
	rect.size = m_mainSp->getBoundingBox().size;
	debugDraw->drawRect(rect.origin, Vec2(rect.getMaxX(), rect.getMaxY()), Color4F::GREEN);
#endif
    
#endif // NS_DEBUG
}

void Hero::animateStand()
{
	this->stopAllAnimations();
	this->stopAllDelayedMovements();

	m_currBasicMovement = BASIC_MOVEMENT_STAND;

	this->setAnimationFrame(COMPONENT_BODY, m_data->getDisplayId(), BASIC_MOVEMENT_WALK, COMPONENT_MOVEMENT_NONE, WALK_STAND_POSE_FRAMEINDEX);
	if (m_data->hasMovementFlag(MOVEMENT_FLAG_HANDUP))
	{
		m_currArmMovement = ARM_MOVEMENT_HANDUP;
		m_snowballSp->setScale(this->getData()->getProjectileScale());
		this->setAnimationFrame(COMPONENT_SNOWBALL, SNOWBALL_STYLE_SQUARE, BASIC_MOVEMENT_WALK, ARM_MOVEMENT_HANDUP, WALK_STAND_POSE_FRAMEINDEX);
		this->setAnimationFrame(COMPONENT_ARM, m_data->getDisplayId(), BASIC_MOVEMENT_WALK, ARM_MOVEMENT_HANDUP, WALK_STAND_POSE_FRAMEINDEX);
	}
	else
	{
		m_currArmMovement = ARM_MOVEMENT_BASIC;
		m_snowballSp->setVisible(false);
		this->setAnimationFrame(COMPONENT_ARM, m_data->getDisplayId(), BASIC_MOVEMENT_WALK, ARM_MOVEMENT_BASIC, WALK_STAND_POSE_FRAMEINDEX);
	}
}

void Hero::animateWalk()
{
	this->stopAllAnimations();
	this->stopAllDelayedMovements();

	m_currBasicMovement = BASIC_MOVEMENT_WALK;

	this->startWalkAnimation(COMPONENT_BODY, m_data->getDisplayId(), COMPONENT_MOVEMENT_NONE);
	if (m_data->hasMovementFlag(MOVEMENT_FLAG_HANDUP))
	{
		m_currArmMovement = ARM_MOVEMENT_HANDUP;
		m_snowballSp->setScale(m_data->getProjectileScale());
		this->startWalkAnimation(COMPONENT_SNOWBALL, SNOWBALL_STYLE_SQUARE, ARM_MOVEMENT_HANDUP);
		this->startWalkAnimation(COMPONENT_ARM, m_data->getDisplayId(), ARM_MOVEMENT_HANDUP);
	}
	else
	{
		m_currArmMovement = ARM_MOVEMENT_BASIC;
		m_snowballSp->setVisible(false);
		this->startWalkAnimation(COMPONENT_ARM, m_data->getDisplayId(), ARM_MOVEMENT_BASIC);
	}
}

void Hero::animateDead(bool skipOpening)
{
	this->stopAllDelayedMovements();
	this->stopAllAnimations();

	m_currBasicMovement = BASIC_MOVEMENT_DEAD;
	m_currArmMovement = COMPONENT_MOVEMENT_NONE;

	m_mainSp->setPosition(COMPONENT_DRAWING_OFFSET_DEAD);
	m_armSp->setPosition(COMPONENT_DRAWING_OFFSET_DEAD);

	if (!skipOpening)
	{
		this->startDeadAnimation();
	}
	else
	{
		this->setAnimationFrame(COMPONENT_BODY, m_data->getDisplayId(), BASIC_MOVEMENT_DEAD, COMPONENT_MOVEMENT_NONE, DEAD_POSE_FRAMEINDEX);
		this->updateGlobalZOrderOnDead();
	}

	m_armSp->setVisible(false);
	m_snowballSp->setVisible(false);
	m_nameplate->setVisible(false);
	m_magicBean->setVisible(false);
	m_smiley->setPosition(m_mainSp->getBoundingBox().getMidX(), m_mainSp->getBoundingBox().getMaxY() - 13);
}

void Hero::animateAlive()
{
	m_mainSp->setPosition(COMPONENT_DRAWING_OFFSET_ALIVE);
	m_armSp->setPosition(COMPONENT_DRAWING_OFFSET_ALIVE);

	if (m_data->hasUnitFlag(UNIT_FLAG_DAMAGED))
		this->animateDamaged();
	else
	{
		if (m_data->hasMovementFlag(MOVEMENT_FLAG_WALKING))
			this->animateWalk();
		else
			this->animateStand();
	}

	m_nameplate->setVisible(true);
	if (m_data->getMagicBeanCount() > 0)
		m_magicBean->setVisible(true);
	m_smiley->setPosition(m_mainSp->getBoundingBox().getMidX(), m_nameplate->getBoundingBox().getMaxY() - 4);
}

void Hero::animateDamaged()
{
	this->stopAllAnimations();
	this->stopAllDelayedMovements();

	m_currBasicMovement = BASIC_MOVEMENT_DAMAGED;

	this->setAnimationFrame(COMPONENT_BODY, m_data->getDisplayId(), BASIC_MOVEMENT_DAMAGED, COMPONENT_MOVEMENT_NONE, DAMAGED_POSE_FRAMEINDEX);
	if (m_data->hasMovementFlag(MOVEMENT_FLAG_HANDUP))
	{
		m_currArmMovement = ARM_MOVEMENT_HANDUP;
		m_snowballSp->setScale(this->getData()->getProjectileScale());
		this->setAnimationFrame(COMPONENT_SNOWBALL, SNOWBALL_STYLE_SQUARE, BASIC_MOVEMENT_DAMAGED, ARM_MOVEMENT_HANDUP, DAMAGED_POSE_FRAMEINDEX);
		this->setAnimationFrame(COMPONENT_ARM, m_data->getDisplayId(), BASIC_MOVEMENT_DAMAGED, ARM_MOVEMENT_HANDUP, DAMAGED_POSE_FRAMEINDEX);
	}
	else
	{
		m_currArmMovement = ARM_MOVEMENT_BASIC;
		m_snowballSp->setVisible(false);
		this->setAnimationFrame(COMPONENT_ARM, m_data->getDisplayId(), BASIC_MOVEMENT_DAMAGED, ARM_MOVEMENT_BASIC, DAMAGED_POSE_FRAMEINDEX);
	}

	m_isWalkAnimDirty = true;
}

void Hero::animateHurl()
{
	this->stopAllAnimations();
	this->stopAllDelayedMovements();

	m_currBasicMovement = BASIC_MOVEMENT_STAND;
	m_currArmMovement = ARM_MOVEMENT_HURL;

	this->setAnimationFrame(COMPONENT_BODY, m_data->getDisplayId(), BASIC_MOVEMENT_WALK, COMPONENT_MOVEMENT_NONE, WALK_STAND_POSE_FRAMEINDEX);
	this->setAnimationFrame(COMPONENT_ARM, m_data->getDisplayId(), BASIC_MOVEMENT_STAND, ARM_MOVEMENT_HURL, STAND_HURL_POSE_FRAMEINDEX);
	m_snowballSp->setVisible(false);

	m_isWalkAnimDirty = true;
}

bool Hero::isHurlAnimated() const
{
	return m_currBasicMovement == BASIC_MOVEMENT_STAND && m_currArmMovement == ARM_MOVEMENT_HURL;
}

void Hero::animateDelayedStand()
{
	if (this->isScheduled(SCHEDULE_KEY_WALK_ANIM))
		this->unschedule(SCHEDULE_KEY_WALK_ANIM);

	float delay = 0.f;
	switch (m_currBasicMovement)
	{
	case BASIC_MOVEMENT_WALK:
		this->stopAllAnimations();
		delay = this->calcWalkFrameDelay(m_data->getMoveSpeed());
		break;
	case BASIC_MOVEMENT_DAMAGED:
		delay = DAMAGED_POSE_DURATION;
		break;
	case BASIC_MOVEMENT_STAND:
		if (m_currArmMovement == ARM_MOVEMENT_HURL)
			delay = STAND_HURL_POSE_DURATION;
		break;
	default:
		break;
	}

	if (delay > 0)
	{
		if (!this->isScheduled(SCHEDULE_KEY_STAND_ANIM))
		{
			this->scheduleOnce([this](float dt) {
				this->animateStand();
			}, delay, SCHEDULE_KEY_STAND_ANIM);
		}
	}
	else
		this->animateStand();
}

void Hero::animateDelayedWalk()
{
	if (this->isScheduled(SCHEDULE_KEY_STAND_ANIM))
		this->unschedule(SCHEDULE_KEY_STAND_ANIM);

	float delay = 0.f;
	switch (m_currBasicMovement)
	{
	case BASIC_MOVEMENT_DAMAGED:
		delay = DAMAGED_POSE_DURATION;
		break;
	case BASIC_MOVEMENT_STAND:
		if (m_currArmMovement == ARM_MOVEMENT_HURL)
			delay = STAND_HURL_POSE_DURATION;
		break;
	default:
		break;
	}

	if (delay > 0)
	{
		if (!this->isScheduled(SCHEDULE_KEY_WALK_ANIM))
		{
			this->scheduleOnce([this](float dt) {
				this->animateWalk();
			}, delay, SCHEDULE_KEY_WALK_ANIM);
		}
	}
	else
		this->animateWalk();

}

void Hero::stopAllDelayedMovements()
{
	if (this->isScheduled(SCHEDULE_KEY_WALK_ANIM))
		this->unschedule(SCHEDULE_KEY_WALK_ANIM);

	if (this->isScheduled(SCHEDULE_KEY_STAND_ANIM))
		this->unschedule(SCHEDULE_KEY_STAND_ANIM);
}

void Hero::updateGlobalZOrderOnAlive()
{
	NS_ASSERT(m_data->isAlive());
	float offsetZ;
	float zOrder = this->getData()->getMapData()->getTileZForPos(this->getData()->getPosition(), &offsetZ) + offsetZ;
	m_mainSp->setGlobalZOrder(zOrder);
	m_armSp->setGlobalZOrder(zOrder);
	m_snowballSp->setGlobalZOrder(zOrder);
	m_nameplate->setGlobalZOrder(zOrder);
	m_heroEffects->setGlobalZOrder(zOrder);
	m_magicBean->setGlobalZOrder(zOrder);
	m_smiley->setGlobalZOrder(zOrder);
}

void Hero::updateGlobalZOrderOnDead()
{
	NS_ASSERT(!m_data->isAlive());
	float offsetZ;
	float zOrder = this->getData()->getMapData()->getTileZForPos(this->getData()->getPosition(), &offsetZ) + OVERLAY_CORPSE + offsetZ;
	m_mainSp->setGlobalZOrder(zOrder);
}

bool Hero::addFootprint(Point const& pos, float orient)
{
	MapData* mapData = m_data->getMapData();

	TileCoord coord(mapData->getMapSize(), this->getPosition());
	if (mapData->isSnow(coord))
	{
		Footprint* footprint = FootprintPool::getInstance()->take();
		m_gameMapLayer->addChild(footprint, false);
		float offsetZ;
		float z = mapData->getTileZForPos(pos, &offsetZ) + OVERLAY_FOOTPRINT + offsetZ;
		footprint->run(pos, orient, z);

		return true;
	}

	return false;
}

void Hero::updateMovementAnimation()
{
	if (m_data->isAlive())
	{
		if (m_data->hasUpdatedField(CUNIT_FIELD_PROJECTILE_SCALE))
			m_snowballSp->setScale(m_data->getProjectileScale());

		if (m_data->hasUpdatedField(CUNIT_FIELD_HEALTH) && m_data->hasUnitFlag(UNIT_FLAG_DAMAGED))
			this->animateDamaged();

		if (m_data->hasUpdatedField(CUNIT_FIELD_MOVE_SPEED) && m_data->hasMovementFlag(MOVEMENT_FLAG_WALKING))
			this->adjustWalkAnimFrameDelay();

		if (m_isWalkAnimDirty || m_data->hasUpdatedField(CUNIT_FIELD_MOVEMENT_FLAG) || m_data->hasUpdatedField(CUNIT_FIELD_DIRECTION))
		{
			if (m_data->hasMovementFlag(MOVEMENT_FLAG_WALKING))
				this->animateDelayedWalk();
			else
				this->animateDelayedStand();
		}
	}

	m_isWalkAnimDirty = false;
}

void Hero::updateSmileyVisibility(bool force)
{
	if (!force && !m_data->hasUpdatedField(CUNIT_FIELD_SMILEY))
		return;

	uint16 code = m_data->getSmiley();
	if (code > 0)
	{
		std::string frameName = StringUtils::format(SMILEY_FRAMENAME_FORMAT, code);
		m_smiley->setSpriteFrame(frameName);
		m_smiley->setVisible(true);
	}
	else
		m_smiley->setVisible(false);
	this->updateMagicBeanVisibility();
}

void Hero::updateMagicBeanVisibility()
{
	if (!m_smiley->isVisible() && m_data->isAlive() && m_data->getMagicBeanCount() > 0)
		m_magicBean->setVisible(true);
	else
		m_magicBean->setVisible(false);
}

void Hero::updateMagicBeanCount(bool force)
{
	if (!force && !m_data->hasUpdatedField(CUNIT_FIELD_MAGIC_BEAN_COUNT))
		return;

	m_magicBean->setCount(m_data->getMagicBeanCount());
	this->updateMagicBeanVisibility();
}

void Hero::conceal()
{
	if (m_isConcealing)
		return;

	m_mainSp->setOpacity(CONCEALING_OPACITY);
	m_armSp->setOpacity(CONCEALING_OPACITY);
	m_snowballSp->setOpacity(CONCEALING_OPACITY);
	m_heroEffects->setOpacity(CONCEALING_OPACITY);
	m_isConcealing = true;
}

void Hero::expose()
{
	if (!m_isConcealing)
		return;

	m_mainSp->setOpacity(EXPOSED_OPACITY);
	m_armSp->setOpacity(EXPOSED_OPACITY);
	m_snowballSp->setOpacity(EXPOSED_OPACITY);
	m_heroEffects->setOpacity(EXPOSED_OPACITY);
	m_isConcealing = false;

}

void Hero::updateFootprints(bool force)
{
	if (m_data->isAlive())
	{
		if (force || m_data->hasUpdatedField(CUNIT_FIELD_MOVEMENT_FLAG))
		{
			if (m_data->hasMovementFlag(MOVEMENT_FLAG_WALKING))
			{
				m_currStepPos = this->getPosition();
				m_currStepLength = 0;
				this->addFootprint(this->getPosition(), m_data->getOrientation());
			}
			else
			{
				m_currStepPos = Point::ZERO;
				m_currStepLength = 0;
			}
		}

		if (m_data->hasMovementFlag(MOVEMENT_FLAG_WALKING))
		{
			float dist = m_currStepPos.getDistance(this->getPosition());
			m_currStepLength += dist;
			if (m_currStepLength >= WALK_STEPLENGTH)
			{
				m_currStepLength = 0;
				this->addFootprint(this->getPosition(), m_data->getOrientation());

			}
			m_currStepPos = this->getPosition();
		}
	}
}

void Hero::onAnimationFrameDisplayed(EventCustom* event)
{
	void* userData = event->getUserData();
	if (userData)
	{
		AnimationFrame::DisplayedEventInfo* frameDisplayedEventInfo = (AnimationFrame::DisplayedEventInfo*)userData;
		auto userInfo = frameDisplayedEventInfo->userInfo;
		if (userInfo)
		{
			ValueMap values = *userInfo;
			ComponentType componentType = (ComponentType)values["ComponentType"].asInt();
			Sprite* component = this->getComponent(componentType);
			if (component == frameDisplayedEventInfo->target)
			{
				BasicMovement basicMovement = (BasicMovement)values["BasicMovement"].asInt();
				int32 componentMovement = values["ComponentMovement"].asInt();
				std::string framename = values["FrameName"].asString();
				int32 frameIndex = values["FrameIndex"].asInt();
				this->updateComponentWhenSpriteFrameChanged(basicMovement, componentType, componentMovement, framename, frameIndex);
				//CCLOG("Hero::onAnimationFrameDisplayed guid: %u basicMovement: %d componentType: %d componentMovement: %d framename: %s frameIndex: %d", m_data->getGuid().getRawValue(), basicMovement, componentType, componentMovement, framename.c_str(), frameIndex);
			}
		}
	}
}

void Hero::registerAnimationFrameDisplayedListener()
{
	if (!m_animationFrameDisplayedListener)
	{
		m_animationFrameDisplayedListener = Director::getInstance()->getEventDispatcher()->addCustomEventListener(AnimationFrameDisplayedNotification, CC_CALLBACK_1(Hero::onAnimationFrameDisplayed, this));
		CC_SAFE_RETAIN(m_animationFrameDisplayedListener);
	}
}

void Hero::unregisterAnimationFrameDisplayedListener()
{
	if (m_animationFrameDisplayedListener)
	{
		Director::getInstance()->getEventDispatcher()->removeEventListener(m_animationFrameDisplayedListener);
		CC_SAFE_RELEASE_NULL(m_animationFrameDisplayedListener);
	}
}

void Hero::updateComponentWhenSpriteFrameChanged(BasicMovement basicMovement, ComponentType componentType, int32 componentMovement, std::string const& frameName, int32 frameIndex)
{
	switch (basicMovement)
	{
	case BASIC_MOVEMENT_DEAD:
		if (frameIndex == DEAD_FELL_DOWN_FRAMEINDEX)
			this->updateGlobalZOrderOnDead();
		break;
	default:
		break;
	}

	switch (componentType)
	{
	case COMPONENT_ARM:
		if (componentMovement == ARM_MOVEMENT_HANDUP)
			this->updateComponentSizeAndPosition(COMPONENT_SNOWBALL, frameName);
		break;
	default:
		break;
	}
}

NS_END
