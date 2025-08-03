//
//  GiftBox.cpp
//  snowfight
//
//  Created by Luthier on 2021/10/22.
//

#include "GiftBox.h"

#include "game/GameConfig.h"
#include "game/LocaleMgr.h"
#include "scene/SoundMgr.h"

NS_BEGIN

#define GIFT_BOX_FRAMES							7
#define GIFT_BOX_DEFAULT_FRAME_INDEX			0
#define GIFT_BOX_FRAMENAME_FORMAT				"gift_box%.2d.png"
#define GIFT_BOX_FRAMENAME_CLAIMED				"gift_box_claimed.png"

#define ACTION_TAG_ANIM							1
#define ACTION_TAG_SCALE						2

#define SCHEDULE_KEY_TIPS_DELAYED				"TipsDelayed"

GiftBox* GiftBox::create()
{
    auto ret = new (std::nothrow) GiftBox();
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

bool GiftBox::init()
{
    if (!Node::init())
        return false;

	this->setIgnoreAnchorPointForPosition(false);
	this->setCascadeColorEnabled(true);

	m_touchListener = EventListenerTouchOneByOne::create();
	CC_SAFE_RETAIN(m_touchListener);
	m_touchListener->setSwallowTouches(true);
	m_touchListener->onTouchBegan = CC_CALLBACK_2(GiftBox::onTouchBegan, this);
	m_touchListener->onTouchMoved = CC_CALLBACK_2(GiftBox::onTouchMoved, this);
	m_touchListener->onTouchEnded = CC_CALLBACK_2(GiftBox::onTouchEnded, this);
	m_touchListener->onTouchCancelled = CC_CALLBACK_2(GiftBox::onTouchCancelled, this);
	getEventDispatcher()->addEventListenerWithSceneGraphPriority(m_touchListener, this);

	this->initAnimation();

	m_mainSp = Sprite::createWithSpriteFrame(m_animation->getFrames().at(GIFT_BOX_DEFAULT_FRAME_INDEX)->getSpriteFrame());
	m_mainSp->setAnchorPoint(Point::ANCHOR_MIDDLE);
	this->addChild(m_mainSp);

	m_infoBubble = InfoBubble::create(InfoBubble::ARROW_DOWN);
	m_infoBubble->setAnchorPoint(Point::ANCHOR_MIDDLE_BOTTOM);
	this->addChild(m_infoBubble);

	this->layoutElements();

    return true;
}

GiftBox::GiftBox() :
	m_state(GIFT_STATE_NONE),
	m_isTipsShown(false),
	m_animation(nullptr),
	m_mainSp(nullptr),
	m_infoBubble(nullptr)
{
    
}

GiftBox::~GiftBox()
{
	if (m_touchListener)
	{
		getEventDispatcher()->removeEventListener(m_touchListener);
		CC_SAFE_RELEASE_NULL(m_touchListener);
	}

	CC_SAFE_RELEASE(m_animation);

	m_hittedByCamera = nullptr;
	m_mainSp = nullptr;
	m_infoBubble = nullptr;
}


void GiftBox::onEnter()
{
	Node::onEnter();
}

void GiftBox::showTips(float delay)
{
	if (m_isTipsShown)
		return;

	this->scheduleOnce([this](float dt) {
		m_infoBubble->show(sLocaleMgr->getString("func_tips_get_daily_rewards"));
	}, delay, SCHEDULE_KEY_TIPS_DELAYED);

	m_isTipsShown = true;
}

void GiftBox::hideTips()
{
	if (!m_isTipsShown)
		return;

	this->unschedule(SCHEDULE_KEY_TIPS_DELAYED);

	m_infoBubble->hide();
	m_isTipsShown = false;
}

void GiftBox::setGiftState(GiftState state)
{
	if (m_state == state)
		return;

	m_state = state;

	switch (state)
	{
	case GIFT_STATE_READY:
		this->playAnimation();
		break;
	case GIFT_STATE_CLAIMED:
		this->stopAnimation();
		m_mainSp->setSpriteFrame(GIFT_BOX_FRAMENAME_CLAIMED);
		break;
	default:
		this->stopAnimation();
		m_mainSp->setSpriteFrame(m_animation->getFrames().at(GIFT_BOX_DEFAULT_FRAME_INDEX)->getSpriteFrame());
		break;
	}
}

bool GiftBox::onTouchBegan(Touch* touch, Event* event)
{
	m_hitted = false;
	if (isVisible())
	{
		Point touchLoc = touch->getLocation();
		auto camera = Camera::getVisitingCamera();
		if (hitTest(touchLoc, camera, nullptr))
		{
			m_hittedByCamera = camera;
			m_hitted = true;
			this->changeToPressedState();
		}
	}
	return m_hitted;
}

void GiftBox::onTouchMoved(Touch* touch, Event* event)
{
	Point touchLoc = touch->getLocation();
	bool hitted = hitTest(touchLoc, m_hittedByCamera, nullptr);
	if (hitted != m_hitted)
	{
		m_hitted = hitted;
		if (m_hitted)
			this->changeToPressedState();
		else
			this->changeToNormalState();
	}
}

void GiftBox::onTouchEnded(Touch* touch, Event* event)
{
	if (m_hitted)
	{
		this->changeToNormalState();
		sSoundMgr->play(SOUND_BUTTON);

		if (m_clickCallback)
			m_clickCallback(this);
	}

	m_hitted = false;
}

void GiftBox::onTouchCancelled(Touch* touch, Event* event)
{
	m_hitted = false;

	this->changeToNormalState();
}

void GiftBox::initAnimation()
{
	Vector<SpriteFrame*> frameSeq;
	for (int32 i = 0; i < GIFT_BOX_FRAMES; ++i)
	{
		std::string framename = StringUtils::format(GIFT_BOX_FRAMENAME_FORMAT, i);
		SpriteFrame* sf = SpriteFrameCache::getInstance()->getSpriteFrameByName(framename);
		NS_ASSERT(sf != nullptr);
		frameSeq.pushBack(sf);
	}
	m_animation = Animation::createWithSpriteFrames(frameSeq, ANIM_NORMAL_FRAME_DELAY);
	CC_SAFE_RETAIN(m_animation);
}

void GiftBox::playAnimation()
{
	if (Action* action = m_mainSp->getActionByTag(ACTION_TAG_ANIM))
		m_mainSp->stopAction(action);

	Animate* animate = Animate::create(m_animation);
	Sequence* seq = Sequence::create(DelayTime::create(1.0f), animate, nullptr);
	RepeatForever* repeat = RepeatForever::create(seq);
	repeat->setTag(ACTION_TAG_ANIM);
	m_mainSp->runAction(repeat);
}

void GiftBox::stopAnimation()
{
	if (Action* action = m_mainSp->getActionByTag(ACTION_TAG_ANIM))
		m_mainSp->stopAction(action);
}

void GiftBox::layoutElements()
{
	this->setContentSize(m_mainSp->getContentSize());
	m_mainSp->setPosition(this->getContentSize().width / 2, this->getContentSize().height / 2);
	m_infoBubble->setPosition(this->getContentSize().width / 2, this->getContentSize().height - 7);
}


bool GiftBox::hitTest(Vec2 const& pt, Camera const* camera, Vec3* p) const
{
	Rect rect;
	rect.size = getContentSize();
	return isScreenPointInRect(pt, camera, getWorldToNodeTransform(), rect, p);
}

void GiftBox::changeToNormalState()
{
	if (Action* action = m_mainSp->getActionByTag(ACTION_TAG_SCALE))
		m_mainSp->stopAction(action);

	auto* scale = ScaleTo::create(0.05f, 1.0f);
	m_mainSp->runAction(scale);
}

void GiftBox::changeToPressedState()
{
	if (Action* action = m_mainSp->getActionByTag(ACTION_TAG_SCALE))
		m_mainSp->stopAction(action);

	auto* scale = ScaleTo::create(0.05f, 1.15f);
	m_mainSp->runAction(scale);
}


NS_END