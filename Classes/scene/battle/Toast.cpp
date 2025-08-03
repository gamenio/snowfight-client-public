#include "Toast.h"

#include "scene/Utils.h"
#include "game/World.h"
#include "game/entities/DataPlayer.h"
#include "game/LocaleMgr.h"

NS_BEGIN

#define ACTION_TAG_SHOW				1
#define FONT_SIZE					12

#define TOAST_ICON_KILLED			"toast_icon_killed.png"
#define TOAST_ICON_DIED				"toast_icon_died.png"

// Toast

Toast::Toast() :
	m_isHiding(false),
	m_flickering(false),
	m_duration(TOAST_DURATION_SHORT)
{
}

Toast::~Toast()
{
}

bool Toast::init()
{
	if (!Node::init())
		return false;

	this->setCascadeOpacityEnabled(true);

	return true;
}

void Toast::show()
{
	this->setOpacity(0);

	Vector<FiniteTimeAction*> actions;

	FadeTo* fadeIn = FadeTo::create(0.5f, 255);
	actions.pushBack(fadeIn);

	if (m_duration > 0)
	{
		CallFunc* callfunc = CallFunc::create([this]() {
			this->hide();
		});

		actions.pushBack(DelayTime::create(m_duration));
		actions.pushBack(callfunc);
	}
	else if(m_flickering)
	{
		Sequence* seq = Sequence::create(FadeTo::create(1.f, 0), FadeTo::create(1.f, 255), nullptr);
		actions.pushBack(Repeat::create(seq, pow(2, 30)));
	}

	Sequence* seq = Sequence::create(actions);
	seq->setTag(ACTION_TAG_SHOW);
	this->runAction(seq);
}

void Toast::hide()
{
	if (m_isHiding)
		return;

	m_isHiding = true;

	if(Action* action = this->getActionByTag(ACTION_TAG_SHOW))
		this->stopAction(action);

	FadeTo* fadeOut = FadeTo::create(1.0f, 0);
	Sequence* seq = Sequence::create(fadeOut, RemoveSelf::create(), nullptr);
	this->runAction(seq);
}

void Toast::onEnter()
{
	Node::onEnter();

	this->layoutElements();
	this->show();
}

// MessageToast

MessageToast::MessageToast() :
	m_label(nullptr)
{

}

MessageToast::~MessageToast()
{
	m_label = nullptr;
}


bool MessageToast::init()
{
	if (!Toast::init())
		return false;

	m_label = Label::createWithSystemFont("Text", DEFAULT_SYSTEM_FONT, FONT_SIZE);
	m_label->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
	m_label->setAlignment(TextHAlignment::LEFT);
	this->setSeverity(INFO);
	this->addChild(m_label);

	return true;
}

MessageToast* MessageToast::create()
{
	MessageToast* pRet = new MessageToast();
	if (pRet && pRet->init())
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

void MessageToast::setText(std::string const& text)
{
	m_label->setString(text);
}

void MessageToast::setSeverity(Severity severity)
{
	switch (severity)
	{
	case NOTICE:
		m_label->setTextColor(Color4B(250, 180, 5, 255));
		break;
	case ALERT:
		m_label->setTextColor(Color4B(224, 41, 27, 255));
		break;
	default: // INFO
		m_label->setTextColor(Color4B(63, 183, 0, 255));
		break;
	}
	Utils::enableBoldForLabel(m_label);
}


void MessageToast::layoutElements()
{
	this->setContentSize(m_label->getContentSize());
}


// ActionToast

ActionToast::ActionToast() :
	m_actionLabel(nullptr),
	m_actionIcon(nullptr),
	m_name1Label(nullptr),
	m_name2Label(nullptr)
{
}

ActionToast::~ActionToast()
{
	m_actionLabel = nullptr;
	m_actionIcon = nullptr;
	m_name1Label = nullptr;
	m_name2Label = nullptr;
}

bool ActionToast::init(ActionType type)
{
	if (!Toast::init())
		return false;

	switch (type)
	{
	case ActionType::ACTION_TYPE_ACTOR1_LOGGED_IN:
		this->addName1Label();
		this->addActionLabel(sLocaleMgr->getString("battle_toast_player_logged_in"));
		break;
	case ActionToast::ACTION_TYPE_ACTOR1_KILL_ACTOR2:
		this->addName1Label();
		this->addActionIcon(TOAST_ICON_KILLED);
		this->addName2Label();
		break;
	case ActionToast::ACTION_TYPE_ACTOR1_DIED:
		this->addName1Label();
		this->addActionIcon(TOAST_ICON_DIED);
		break;
	default:
		NS_ASSERT_LOG(false, "ActionToast: ActionType undefined.");
		break;
	}

	return true;
}

ActionToast* ActionToast::create(ActionType type)
{
	ActionToast* pRet = new ActionToast();
	if (pRet && pRet->init(type))
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

void ActionToast::setActor1(std::string const& name, ObjectGuid const& guid)
{
	if (!m_name1Label)
		return;

	m_name1Label->setString(name);
	this->setNameLabelTextColor(m_name1Label, guid);

}

void ActionToast::setActor2(std::string const& name, ObjectGuid const& guid)
{
	if (!m_name2Label)
		return;

	m_name2Label->setString(name);
	this->setNameLabelTextColor(m_name2Label, guid);
}

Label* ActionToast::addName1Label()
{
	m_name1Label = Label::createWithSystemFont("Name1", DEFAULT_SYSTEM_FONT, FONT_SIZE);
	m_name1Label->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
	this->addChild(m_name1Label);

	return m_name1Label;
}

Label* ActionToast::addName2Label()
{
	m_name2Label = Label::createWithSystemFont("Name2", DEFAULT_SYSTEM_FONT, FONT_SIZE);
	m_name2Label->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
	this->addChild(m_name2Label);

	return m_name2Label;
}

Label* ActionToast::addActionLabel(std::string const& text)
{
	m_actionLabel = Label::createWithSystemFont(text, DEFAULT_SYSTEM_FONT, FONT_SIZE);
	m_actionLabel->setTextColor(Color4B(250, 180, 5, 255));
	m_actionLabel->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
	Utils::enableBoldForLabel(m_actionLabel);

	this->addChild(m_actionLabel);

	return m_actionLabel;
}

Sprite* ActionToast::addActionIcon(std::string const& iconFrameName)
{
	m_actionIcon = Sprite::createWithSpriteFrameName(iconFrameName);
	m_actionIcon->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);

	this->addChild(m_actionIcon);

	return m_actionIcon;
}

void ActionToast::setNameLabelTextColor(Label* label, ObjectGuid const& guid)
{
	DataPlayer* myChar = World::getInstance()->getMyself();
	NS_ASSERT(myChar);

	if (myChar->getGuid() == guid)
		label->setTextColor(Color4B(79, 165, 28, 255));
	else
	{
		label->setTextColor(Color4B(224, 41, 27, 255));
	}

	Utils::enableBoldForLabel(label);
}

void ActionToast::layoutElements()
{
	float nextLeft = 0, maxHeight = 0;
	auto const& children = this->getChildren();
	for (auto node : children)
	{
		node->setPosition(nextLeft, 0);
		maxHeight = std::max(maxHeight, node->getContentSize().height);
		nextLeft += node->getContentSize().width;
	}
	this->setContentSize(Size(nextLeft, maxHeight));
}


NS_END

