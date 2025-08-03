#include "InventorySlot.h"

#include "game/behaviors/CarriedItem.h"
#include "game/behaviors/MyCharacter.h"
#include "game/World.h"
#include "scene/SoundMgr.h"

NS_BEGIN

#define FRAMENAME_INV_SLOT_BG_NORMAL				"inv_slot_bg_normal.png"
#define FRAMENAME_INV_SLOT_BG_PRESS					"inv_slot_bg_press.png"
#define FRAMENAME_INV_SLOT_CUSTOM_NORMAL			"inv_slot_custom_normal.png"
#define FRAMENAME_INV_SLOT_CUSTOM_PRESS				"inv_slot_custom_press.png"
#define FRAMENAME_INV_SLOT_FIRST_AID_NORMAL			"inv_slot_first_aid_normal.png"
#define FRAMENAME_INV_SLOT_FIRST_AID_PRESS			"inv_slot_first_aid_press.png"
#define FRAMENAME_INV_SLOT_PROG						"inv_slot_prog.png"

#define LOCAL_ZORDER_PROP						1
#define LOCAL_ZORDER_COUNT_LABEL				2
#define LOCAL_ZORDER_PROG_TIMER					3

#define FULL_PERCENTAGE							100

#define ACTION_TAG_PROGRESS						1

InventorySlot* InventorySlot::create(SlotType type)
{
	auto ret = new (std::nothrow) InventorySlot();
	if (ret && ret->init(type))
	{
		ret->autorelease();
	}
	else
	{
		CC_SAFE_DELETE(ret);
	}

	return ret;
}

InventorySlot::InventorySlot() :
	m_type(FIRST_AID),
	m_cooldownState(COOLDOWN_STATE_NONE),
	m_isEnabled(true),
	m_touchListener(nullptr),
	m_hitted(false),
	m_hittedByCamera(nullptr),
	m_fgSp(nullptr),
	m_prop(nullptr),
	m_countLabel(nullptr),
	m_progTimer(nullptr)
{
}

InventorySlot::~InventorySlot()
{
	if (m_touchListener)
	{
		getEventDispatcher()->removeEventListener(m_touchListener);
		CC_SAFE_RELEASE_NULL(m_touchListener);
	}

	m_fgSp = nullptr;
	m_prop = nullptr;
	m_countLabel = nullptr;
	m_progTimer = nullptr;
}

bool InventorySlot::init(SlotType type)
{
	if (!Node::init())
		return false;

	m_type = type;

	m_touchListener = EventListenerTouchOneByOne::create();
	CC_SAFE_RETAIN(m_touchListener);
	m_touchListener->setSwallowTouches(true);
	m_touchListener->onTouchBegan = CC_CALLBACK_2(InventorySlot::onTouchBegan, this);
	m_touchListener->onTouchMoved = CC_CALLBACK_2(InventorySlot::onTouchMoved, this);
	m_touchListener->onTouchEnded = CC_CALLBACK_2(InventorySlot::onTouchEnded, this);
	m_touchListener->onTouchCancelled = CC_CALLBACK_2(InventorySlot::onTouchCancelled, this);
	getEventDispatcher()->addEventListenerWithSceneGraphPriority(m_touchListener, this);

	this->setIgnoreAnchorPointForPosition(false);

	m_bgSp = Sprite::createWithSpriteFrameName(FRAMENAME_INV_SLOT_BG_NORMAL);
	m_bgSp->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
	this->addChild(m_bgSp);

	m_fgSp = Sprite::create();
	m_fgSp->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
	this->addChild(m_fgSp);

	m_countLabel = Label::createWithSystemFont("0/0", DEFAULT_SYSTEM_FONT, 10);
	m_countLabel->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
	m_countLabel->setPosition(10, 7);
	m_countLabel->setTextColor(Color4B(237, 235, 242, 255));
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID) || (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
	m_countLabel->enableOutline(Color4B(0, 0, 0, GLubyte(255 * 0.28f)), 1);
#endif
	m_countLabel->enableShadow(Color4B(0, 0, 0, GLubyte(255 * 0.5f)), Size(0.5f, -0.5f), 0);
	m_countLabel->setVisible(false);
	this->addChild(m_countLabel, LOCAL_ZORDER_COUNT_LABEL);

	m_progTimer = ProgressTimer::create(Sprite::createWithSpriteFrameName(FRAMENAME_INV_SLOT_PROG));
	m_progTimer->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
	m_progTimer->setType(ProgressTimer::Type::RADIAL);
	m_progTimer->setPercentage(FULL_PERCENTAGE);
	m_progTimer->setReverseDirection(true);
	m_progTimer->setVisible(false);
	this->addChild(m_progTimer, LOCAL_ZORDER_PROG_TIMER);

	this->setContentSize(m_bgSp->getContentSize());

	this->changeToNormalState();

	return true;
}

void InventorySlot::update(float delta)
{
	this->updatePropCount(false);
}

void InventorySlot::setEnabled(bool enabled)
{
	m_isEnabled = enabled;
}

void InventorySlot::setProp(CarriedProp* prop)
{
	if (m_prop != prop)
	{
		this->removeProp();
		m_prop = prop;
		if (prop)
		{
			prop->setAnchorPoint(Point::ANCHOR_MIDDLE);
			prop->setPosition(this->getContentSize() / 2);
			this->addChild(prop, LOCAL_ZORDER_PROP);
			sSoundMgr->play(SOUND_ITEM_STORE);
		}
	}
}

void InventorySlot::removeProp()
{
	if (!m_prop)
		return;

	m_prop->removeFromParentAndCleanup(true);
	m_prop = nullptr;

	this->clearAfterPropInactivated();
}

void InventorySlot::onPropActivated()
{
	NS_ASSERT_LOG(m_prop, "No prop for slot");
	m_prop->onActivated();
	DataCarriedItem* item = m_prop->getData();
	if (item->getStackable() == ITEM_STACK_UNLIMITED || item->getStackable() > 1)
		m_countLabel->setVisible(true);
	else
		m_countLabel->setVisible(false);
	m_fgSp->setVisible(false);

	this->updatePropCount(true);
	this->updateCooldown();
}

void InventorySlot::onPropInactivated()
{
	NS_ASSERT_LOG(m_prop, "No prop for slot");
	if(m_prop)
		m_prop->onInactivated();

	this->clearAfterPropInactivated();
}

void InventorySlot::onItemUseResult(ItemUseStatus status)
{
	if (status == ITEM_USE_STATUS_OK)
		this->updateCooldown();
	else
		m_cooldownState = COOLDOWN_STATE_NONE;
}

bool InventorySlot::onTouchBegan(Touch* touch, Event* event)
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

void InventorySlot::onTouchMoved(Touch* touch, Event* event)
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

void InventorySlot::onTouchEnded(Touch* touch, Event* event)
{
	if (m_hitted)
	{
		this->changeToNormalState();
		if (m_prop && m_isEnabled)
		{
			sSoundMgr->play(SOUND_BUTTON);
			if (m_cooldownState == COOLDOWN_STATE_NONE)
			{
				DataCarriedItem* data = m_prop->getData();
				World::getInstance()->sendUseItem(data->getSlot(), data->getGuid());
				m_cooldownState = COOLDOWN_STATE_PENDING;
			}
		}
	}

	m_hitted = false;
}

void InventorySlot::onTouchCancelled(Touch* touch, Event* event)
{
	m_hitted = false;

	this->changeToNormalState();
}

bool InventorySlot::hitTest(Vec2 const& pt, Camera const* camera, Vec3* p) const
{
	Rect rect;
	rect.size = getContentSize();
	return isScreenPointInRect(pt, camera, getWorldToNodeTransform(), rect, p);
}

void InventorySlot::changeToNormalState()
{
	if (m_prop && m_isEnabled)
		m_prop->changeToNormalState();
	else
	{
		m_bgSp->setSpriteFrame(FRAMENAME_INV_SLOT_BG_NORMAL);
		switch (m_type)
		{
		case FIRST_AID:
			m_fgSp->setSpriteFrame(FRAMENAME_INV_SLOT_FIRST_AID_NORMAL);
			break;
		case CUSTOM:
			m_fgSp->setSpriteFrame(FRAMENAME_INV_SLOT_CUSTOM_NORMAL);
			break;
		}
	}
}

void InventorySlot::changeToPressedState()
{
	if (m_prop && m_isEnabled)
		m_prop->changeToPressedState();
	else
	{
		m_bgSp->setSpriteFrame(FRAMENAME_INV_SLOT_BG_PRESS);
		switch (m_type)
		{
		case FIRST_AID:
			m_fgSp->setSpriteFrame(FRAMENAME_INV_SLOT_FIRST_AID_PRESS);
			break;
		case CUSTOM:
			m_fgSp->setSpriteFrame(FRAMENAME_INV_SLOT_CUSTOM_PRESS);
			break;
		}
	}
}

void InventorySlot::startCooldownProgress(float elapsed, float duration)
{
	this->stopCooldownProgress();

	m_progTimer->setVisible(true);
	m_progTimer->setPercentage(FULL_PERCENTAGE);

	ProgressFromTo* progress = ProgressFromTo::create(duration, m_progTimer->getPercentage(), 0);
	CallFunc* callFunc = CallFunc::create(CC_CALLBACK_0(InventorySlot::onCooldownCompleted, this));
	Sequence* action = Sequence::create(progress, callFunc, Hide::create(), nullptr);
	action->setTag(ACTION_TAG_PROGRESS);
	m_progTimer->runAction(action);

	action->step(0);
	action->step(elapsed);
}

void InventorySlot::stopCooldownProgress()
{
	if (Action* action = m_progTimer->getActionByTag(ACTION_TAG_PROGRESS))
		m_progTimer->stopAction(action);
}

void InventorySlot::onCooldownCompleted()
{
	m_cooldownState = COOLDOWN_STATE_NONE;

	if (m_prop)
		m_prop->startReminderAnimation();
}

void InventorySlot::clearAfterPropInactivated()
{
	m_countLabel->setVisible(false);
	m_fgSp->setVisible(true);

	m_cooldownState = COOLDOWN_STATE_NONE;
	m_progTimer->setVisible(false);
	this->stopCooldownProgress();
}

void InventorySlot::updatePropCount(bool force)
{
	if (!m_prop)
		return;

	DataCarriedItem* item = m_prop->getData();
	if (force || item->hasUpdatedField(CCARRIEDITEM_FIELD_COUNT))
	{
		if (item->getStackable() == ITEM_STACK_UNLIMITED)
			m_countLabel->setString(StringUtils::format("%d", item->getCount()));
		else if (item->getStackable() > 1)
			m_countLabel->setString(StringUtils::format("%d/%d", item->getCount(), item->getStackable()));

		if(!force)
			sSoundMgr->play(SOUND_ITEM_STORE);
	}
}

void InventorySlot::updateCooldown()
{
	MyCharacter* myChar = World::getInstance()->getMyCharacter();
	if (myChar)
	{
		ItemCooldownProcesser* processer = myChar->getItemCooldownProcesser();
		DataCarriedItem* data = m_prop->getData();
		if (!processer->isReady(data->getItemId()))
		{
			m_cooldownState = COOLDOWN_STATE_STARTED;

			float duration = time_util::toGameTimeSeconds(data->getCooldownDuration());
			float elapsed = processer->getElapsedCooldown(data->getItemId());
			this->startCooldownProgress(elapsed, duration);
		}
	}

}

NS_END

