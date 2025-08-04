#include "Chest.h"

#include "common/utils/MathTools.h"
#include "common/debugging/DebugDrawer.h"
#include "scene/SoundMgr.h"
#include "scene/gamble/Hero.h"
#include "scene/Utils.h"
#include "scene/TutorialService.h"
#include "game/entities/DataItemBox.h"
#include "game/entities/DataPlayer.h"
#include "game/GameConfig.h"
#include "game/World.h"


NS_BEGIN

// Debug drawer
#if NS_DEBUG

#define DEBUG_COLLISION_BOX                    0

#endif // NS_DEBUG

// Chest sound effect configuration
#define SOUND_VOLUME_CHEST						1.0f

// Chest movement animation configuration
// Frame naming format: prefix + direction + frame index
#define MOVEMENT_FRAME_FORMAT			"chest%d%02d.png"
#define MAIN_FRAME_DEFAULT				"chest000.png"
#define MAIN_ACTION_TAG					1
#define SHADOW_DRAWING_OFFSET				Vec2(-32.0f, -16.0f)

// Locking animation configuration
#define LOCKED_POSE_FRAMEINDEX			0

// Opening animation configuration
#define OPEN_ANIM_FRAMES				8		// Number of animation frames
#define OPEN_POSE_FRAMEINDEX			(OPEN_ANIM_FRAMES - 1)

Chest::Chest() :
	m_data(nullptr),
	m_nameLabel(nullptr),
	m_healthBar(nullptr),
	m_mainSp(nullptr)
{
}

Chest::~Chest()
{
	CC_SAFE_RELEASE_NULL(m_data);
	m_nameLabel = nullptr;
	m_healthBar = nullptr;
	m_mainSp = nullptr;
}

Chest* Chest::createWithData(DataItemBox* data)
{
	Chest *pRet = new Chest();
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

bool Chest::initWithData(DataItemBox* data)
{
	if (!GameObject::init())
		return false;

	CC_SAFE_RETAIN(data);
	m_data = data;

	// Main
	m_mainSp = SILSprite::createWithSpriteFrameName(MAIN_FRAME_DEFAULT);
	m_mainSp->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
	m_mainSp->setPosition(SHADOW_DRAWING_OFFSET);
	this->addChild(m_mainSp);

	// Health bar
	m_healthBar = HealthBar::create();
	m_healthBar->setAnchorPoint(Point::ANCHOR_MIDDLE_BOTTOM);
	m_healthBar->setPosition(m_mainSp->getBoundingBox().getMidX(), m_mainSp->getBoundingBox().getMaxY() - 30.f);
	this->addChild(m_healthBar);

#if NS_DEBUG
	// Name
	m_nameLabel = SILLabel::createWithSystemFont("", DEFAULT_SYSTEM_FONT, 14);
	m_nameLabel->setPosition(m_mainSp->getBoundingBox().getMidX(), m_healthBar->getBoundingBox().getMaxY() + 2.0f);
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

	this->initAnimations();

	return true;
}

void Chest::onActivated()
{
	if (this->isActive())
		return;

	GameObject::onActivated();

	if (m_data->isLocked())
		this->animateLocked();
	else
		this->animateOpen(true);
	this->updatePosition();
	this->updateHealth(true);
	this->updateTutorialEvents(true);

	this->setVisible(true);
}

void Chest::onInactivated()
{
	if (!this->isActive())
		return;

	m_healthBar->setVisible(false);
	this->stopAnimation();

	this->setVisible(false);

	GameObject::onInactivated();
}

void Chest::update(float delta)
{
	GameObject::update(delta);

	if (!this->isActive())
		return;

	this->updateHealth(false);
	this->updateActionAnimation();
	this->updateTutorialEvents(false);

	this->debugDraw();
}

void Chest::setGlobalZOrder(float globalZOrder)
{
	m_mainSp->setGlobalZOrder(globalZOrder);
	if(m_nameLabel)
		m_nameLabel->setGlobalZOrder(globalZOrder);
	m_healthBar->setGlobalZOrder(globalZOrder);

	Node::setGlobalZOrder(globalZOrder);
}

void Chest::debugDraw()
{
#if NS_DEBUG

	Brush* debugDraw;
	debugDraw = nullptr;

#if DEBUG_COLLISION_BOX

	debugDraw = sDebugDrawer->getDrawByTag("Chest.CollisionBox");
	debugDraw->setClearing(true);

	// Draw the shape of the object
	debugDraw->drawRect(m_data->getBoundingBox().origin, Vec2(m_data->getBoundingBox().getMaxX(), m_data->getBoundingBox().getMaxY()), Color4F::RED);

	// Draw the shape of the object on the map
	float scaleX = std::sqrt(2);
	debugDraw->drawCircle(m_data->getPosition(), m_data->getObjectRadiusInMap(), scaleX, scaleX / 2, Color4F::RED);
	debugDraw->drawPoint(m_data->getPosition(), 2.0f, Color4F::RED);
#endif // DEBUG_COLLISION_BOX

#endif // NS_DEBUG
}

void Chest::initAnimations()
{
	this->createOpenAnimation();
}

Animation* Chest::createAnimation(uint8 dir, int32 numOfFrames, float delayPerUnit, bool repeated)
{
	Vector<AnimationFrame*> animFrameList;
	for (int32 i = 0; i < numOfFrames; ++i)
	{
		std::string frameName = StringUtils::format(MOVEMENT_FRAME_FORMAT, dir, i);
		SpriteFrame* frame = SpriteFrameCache::getInstance()->getSpriteFrameByName(frameName);
		NS_ASSERT(frame != nullptr);
		AnimationFrame* animFrame = AnimationFrame::create(frame, 1.0f, ValueMapNull);
		animFrameList.pushBack(animFrame);
	}
	uint32 loops = 1;
	if (repeated)
		loops = UINT_MAX;
	Animation* animation = Animation::create(animFrameList, delayPerUnit, loops);

	return animation;
}

void Chest::createOpenAnimation()
{
	for (uint8 dir = 0; dir < DataItemBox::MAX_DIRECTIONS; ++dir)
	{
		Animation* animation = this->createAnimation(dir, OPEN_ANIM_FRAMES, ANIM_NORMAL_FRAME_DELAY, false);
		m_animationSet.insert(dir, animation);
	}
}

void Chest::startOpenAnimation()
{
	Animation* animation = m_animationSet.at(m_data->getDirection());
	NS_ASSERT(animation != nullptr);
	this->runAnimation(animation);
}

void Chest::runAnimation(Animation* animation)
{
	Action* action = m_mainSp->getActionByTag(MAIN_ACTION_TAG);
	if (action)
		m_mainSp->stopAction(action);

	action = Animate::create(animation);
	action->setTag(MAIN_ACTION_TAG);
	m_mainSp->runAction(action);
}

void Chest::stopAnimation()
{
	if (Action* action = m_mainSp->getActionByTag(MAIN_ACTION_TAG))
		m_mainSp->stopAction(action);
}

void Chest::setAnimationFrame(int32 frameIndex)
{
	uint8 dir = m_data->getDirection();
	std::string frameName = StringUtils::format(MOVEMENT_FRAME_FORMAT, dir, frameIndex);
	SpriteFrame* frame = SpriteFrameCache::getInstance()->getSpriteFrameByName(frameName);
	NS_ASSERT(frame != nullptr);
	m_mainSp->setSpriteFrame(frame);
}

void Chest::animateLocked()
{
	this->stopAnimation();
	this->setAnimationFrame(LOCKED_POSE_FRAMEINDEX);
	m_healthBar->setVisible(true);
}

void Chest::animateOpen(bool skipOpening)
{
	this->stopAnimation();
	if (!skipOpening)
	{
		this->startOpenAnimation();
	}
	else
	{
		this->setAnimationFrame(OPEN_POSE_FRAMEINDEX);
	}
	m_healthBar->setVisible(false);
}

void Chest::updatePosition()
{
	DataItemBox* data = this->getData()->asDataItemBox();
    this->setPosition(data->getPosition());

	float newZ = data->getMapData()->getTileZForPos(data->getPosition());
	this->setGlobalZOrder(newZ);
}

void Chest::updateHealth(bool force)
{
	if (!force && !m_data->hasUpdatedField(CITEMBOX_FIELD_MAX_HEALTH) && !m_data->hasUpdatedField(CITEMBOX_FIELD_HEALTH))
		return;

	m_healthBar->setHealthProgress(m_data->getHealth(), m_data->getMaxHealth());
}

void Chest::updateActionAnimation()
{
	if (this->getData()->hasUpdatedField(CITEMBOX_FIELD_IS_LOCKED))
	{
		if (!m_data->isLocked())
		{
			this->animateOpen(false);
			sSoundMgr->play(SOUND_CHEST_OPEN, false, SOUND_VOLUME_CHEST);
		}
	}
}

void Chest::updateTutorialEvents(bool force)
{
	TutorialService* service = TutorialService::getInstance();
	if (!service->isEnabled() || !sGameCenter->getLocalPlayer()->isTrainee())
		return;

	DataPlayer* myChar = World::getInstance()->getMyself();
	if (!myChar)
		return;

	if (force || this->getData()->hasUpdatedField(CITEMBOX_FIELD_IS_LOCKED))
	{
		if (!this->getData()->isLocked())
			service->triggerEvent(TUTORIAL_EVENT_ITEMBOX_OPENED);
	}
}

NS_END

