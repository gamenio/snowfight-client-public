#include "CarriedProp.h"

#include "scene/Utils.h"
#include "game/World.h"
#include "game/behaviors/CarriedItem.h"
#include "game/ObjectMgr.h"
#include "scene/TutorialService.h"

NS_BEGIN

#define ACTION_TAG_SCALE					1

CarriedProp* CarriedProp::createWithData(DataCarriedItem* data)
{
	CarriedProp *pRet = new CarriedProp();
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

CarriedProp::CarriedProp() :
	m_data(nullptr),
	m_isActive(false),
	m_mainSp(nullptr),
	m_nameLabel(nullptr)
{
}

CarriedProp::~CarriedProp()
{
	CC_SAFE_RELEASE_NULL(m_data);

	m_mainSp = nullptr;
	m_nameLabel = nullptr;
}

bool CarriedProp::initWithData(DataCarriedItem* data)
{
	if (!Node::init())
		return false;

	CC_SAFE_RETAIN(data);
	m_data = data;

	ItemTemplate const* tmpl = sObjectMgr->getItemTemplate(m_data->getItemId());
	NS_ASSERT(tmpl);

	this->setCascadeOpacityEnabled(true);

	m_mainSp = SILSprite::createWithSpriteFrameName(StringUtils::format(CARRIED_PROP_FRAME_FORMAT, tmpl->displayId));
	m_mainSp->setAnchorPoint(Point::ANCHOR_MIDDLE);
	m_mainSp->setSilhouetted(false);
	this->addChild(m_mainSp);

#if NS_DEBUG
	m_nameLabel = SILLabel::createWithSystemFont("", DEFAULT_SYSTEM_FONT, 6);
	m_nameLabel->setSilhouetted(false);
	std::stringstream ss;
	ss << m_data->getGuid().getTypeName();
	ss << "ID:" << m_data->getGuid().getCounter();
	m_nameLabel->setString(ss.str());
	m_nameLabel->setAnchorPoint(Point::ANCHOR_MIDDLE_BOTTOM);
	m_nameLabel->setTextColor(Color4B::WHITE);
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID) || (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
	m_nameLabel->enableOutline(Color4B(0, 0, 0, GLubyte(255 * 0.28f)), 1);
#endif
	m_nameLabel->enableShadow(Color4B(0, 0, 0, GLubyte(255 * 0.5f)), Size(0.5f, -0.5f), 0);
	this->addChild(m_nameLabel);

	World* world = World::getInstance();
	if (world->getSession() && world->getSession()->asWorldSession())
	{
		WorldSession* session = world->getSession()->asWorldSession();
		m_nameLabel->setVisible(session->hasGMPermission());
	}
#endif // NS_DEBUG

	this->layoutElements();

	return true;
}

void CarriedProp::update(float delta)
{
	Node::update(delta);

	if (!this->isActive())
		return;

}

void CarriedProp::onActivated()
{
	if (m_isActive)
		return;

	TutorialService* service = TutorialService::getInstance();
	if (service->isEnabled() && sGameCenter->getLocalPlayer()->isTrainee())
	{
		ItemTemplate const* tmpl = sObjectMgr->getItemTemplate(m_data->getItemId());
		NS_ASSERT(tmpl);
		if (tmpl->itemClass == ITEM_CLASS_CONSUMABLE)
			service->triggerEvent(TUTORIAL_EVENT_PICKED_UP_CONSUMABLE_ITEM);
		else if (tmpl->itemClass == ITEM_CLASS_EQUIPMENT)
			service->triggerEvent(TUTORIAL_EVENT_PICKED_UP_EQUIPMENT);
	}

	this->setVisible(true);
	m_isActive = true;
}

void CarriedProp::onInactivated()
{
	if (!m_isActive)
		return;

	m_mainSp->setScale(1.0f);
	this->stopScaleAnimation();

	this->cleanUpdateMask();
	this->setVisible(false);

	m_isActive = false;
}

void CarriedProp::cleanUpdateMask()
{
	if (m_data)
		m_data->clearUpdateFlags();
}

void CarriedProp::changeToNormalState()
{
	this->stopScaleAnimation();

	ScaleTo* scale = ScaleTo::create(0.05f, 1.0f);
	scale->setTag(ACTION_TAG_SCALE);
	m_mainSp->runAction(scale);
}

void CarriedProp::changeToPressedState()
{
	this->stopScaleAnimation();

	ScaleTo* scale = ScaleTo::create(0.05f, 1.15f);
	scale->setTag(ACTION_TAG_SCALE);
	m_mainSp->runAction(scale);
}

void CarriedProp::startReminderAnimation()
{
	this->stopScaleAnimation();

	ScaleBy* scale = ScaleBy::create(0.12f, 1.2f);
	Sequence* action = Sequence::create(scale, scale->reverse(), scale, scale->reverse(), nullptr);
	action->setTag(ACTION_TAG_SCALE);
	m_mainSp->runAction(action);
}

void CarriedProp::layoutElements()
{
	Size contentSize = m_mainSp->getContentSize();

	m_mainSp->setPosition(contentSize / 2);
	if (m_nameLabel && m_nameLabel->isVisible())
		m_nameLabel->setPosition(contentSize.width / 2, contentSize.height);

	this->setContentSize(contentSize);
}

void CarriedProp::stopScaleAnimation()
{
	if (Action* action = m_mainSp->getActionByTag(ACTION_TAG_SCALE))
		m_mainSp->stopAction(action);
}


NS_END

