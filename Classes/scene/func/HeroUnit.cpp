#include "HeroUnit.h"

#include "game/store/Store.h"
#include "game/UserPreferences.h"
#include "game/gamecenter/GameCenter.h"

NS_BEGIN

#define SHOW_FRAMES										1
#define SHOW_DEFAULT_FRAME_INDEX						0
#define SHOW_ANIM_LOOP_TIMES							0


#define ACTION_TAG_SHOW							1

#define FRAMENAME_FUNC_HERO_LOCK			"func_hero_lock.png"

HeroUnit::HeroUnit() :
	m_activated(false),
	m_heroTemplate(nullptr),
	m_animation(nullptr),
	m_lockSp(nullptr)
{
}

HeroUnit::~HeroUnit()
{
	m_heroTemplate = nullptr;
	CC_SAFE_RELEASE_NULL(m_animation);
}

HeroUnit* HeroUnit::create(HeroTemplate const* tmpl)
{
	HeroUnit* node = new (std::nothrow) HeroUnit();
	if (node && node->init(tmpl))
	{
		node->autorelease();
		return node;
	}
	CC_SAFE_DELETE(node);
	return nullptr;
}

bool HeroUnit::init(HeroTemplate const* tmpl)
{
	if (!CatwalkUnit::init())
		return false;

	m_heroTemplate = tmpl;

	auto touchListener = EventListenerTouchOneByOne::create();
	touchListener->setSwallowTouches(false);

	touchListener->onTouchBegan = CC_CALLBACK_2(HeroUnit::onTouchBegan, this);
	touchListener->onTouchMoved = CC_CALLBACK_2(HeroUnit::onTouchMoved, this);
	touchListener->onTouchEnded = CC_CALLBACK_2(HeroUnit::onTouchEnded, this);
	touchListener->onTouchCancelled = CC_CALLBACK_2(HeroUnit::onTouchCancelled, this);
	this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(touchListener, this);

	this->setupAnimation();
	this->setDefaultFrame();

	Size contentSize = this->getContentSize();

	m_lockSp = Sprite::createWithSpriteFrameName(FRAMENAME_FUNC_HERO_LOCK);
	m_lockSp->setPosition(contentSize.width  / 2, 40.0f);
	m_lockSp->setAnchorPoint(Point::ANCHOR_MIDDLE_BOTTOM);
	m_lockSp->setVisible(false);
	m_lockDefaultGLProgramState = m_lockSp->getGLProgramState();
	m_lockSp->setGLProgramState(this->getGLProgramState());
	this->addChild(m_lockSp);
    
    this->updateLockState();

	return true;
}

void HeroUnit::activate()
{
	m_activated = true;

	if (m_locked)
	{
		m_lockSp->setGLProgramState(m_lockDefaultGLProgramState);
		this->setLightness(-0.3f);
	}

	//this->startAnimation();
}

void HeroUnit::inactivate()
{
	if (m_locked)
	{
		m_lockSp->setGLProgramState(this->getGLProgramState());
		this->setLightness(0.0f);
	}

	//this->stopAnimation();

	m_activated = false;
}

void HeroUnit::updateLockState()
{
	m_locked = !sGameCenter->getLocalPlayer()->hasUnlockedHero(m_heroTemplate->id);
    m_lockSp->setVisible(m_locked);
    
    if(m_activated)
    {
        if (m_locked)
        {
            m_lockSp->setGLProgramState(m_lockDefaultGLProgramState);
            this->setLightness(-0.3f);
        }
        else
        {
            m_lockSp->setGLProgramState(this->getGLProgramState());
            this->setLightness(0.0f);
        }
    }
}

bool HeroUnit::hitTest(Point const& p)
{
	Point localPoint = this->getParent()->convertToNodeSpace(p);
	Rect r = this->getTrimmedBoundingBox();
	if (r.containsPoint(localPoint))
	{
		return true;
	}

	return false;
}

bool HeroUnit::onTouchBegan(Touch* touch, Event* event)
{
	return m_activated;
}

void HeroUnit::onTouchEnded(Touch* touch, Event* event)
{
	if (!m_activated)
		return;

	Point touchPoint = touch->getLocation();
	if (this->hitTest(touchPoint))
	{
		CCLOG("Play animation");
	}
}

void HeroUnit::onTouchCancelled(Touch* touch, Event* event)
{
}

void HeroUnit::onTouchMoved(Touch* touch, Event* event)
{
}

void HeroUnit::setupAnimation()
{
	m_animation = Animation::create();
	CC_SAFE_RETAIN(m_animation);
	for (int32 i = 0; i < SHOW_FRAMES; ++i)
	{
		std::string frameName = StringUtils::format("show%d%02d.png", (int32)m_heroTemplate->id, i);
		SpriteFrame* frame = SpriteFrameCache::getInstance()->getSpriteFrameByName(frameName);
		NS_ASSERT(frame != nullptr);
		m_animation->addSpriteFrame(frame);
	}
	m_animation->setDelayPerUnit(ANIM_NORMAL_FRAME_DELAY);
	m_animation->setLoops(SHOW_ANIM_LOOP_TIMES);
}

void HeroUnit::startAnimation()
{
	this->stopActionByTag(ACTION_TAG_SHOW);

	Action* action = Sequence::create(DelayTime::create(1.0f),
		Animate::create(m_animation),
		nullptr);
	action->setTag(ACTION_TAG_SHOW);
	this->runAction(action);
}

void HeroUnit::stopAnimation()
{
	this->stopActionByTag(ACTION_TAG_SHOW);
	this->setDefaultFrame();
}

void HeroUnit::setDefaultFrame()
{
	auto frames = m_animation->getFrames();
	auto animFrame = frames.at(SHOW_DEFAULT_FRAME_INDEX);
	this->setSpriteFrame(animFrame->getSpriteFrame());
}

NS_END
