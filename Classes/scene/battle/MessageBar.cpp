#include "MessageBar.h"

#include "scene/Utils.h"

NS_BEGIN

#define ACTION_TAG_SHOW_OR_HIDE			1
#define FONT_SIZE						14

MessageBar::MessageBar() :
	m_isShown(false),
	m_iconSp(nullptr),
	m_textLabel(nullptr)
{
}

MessageBar* MessageBar::create()
{
	MessageBar* pRet = new MessageBar();
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

MessageBar::~MessageBar()
{
	m_textLabel = nullptr;
}

bool MessageBar::init()
{
	if (!Node::init())
		return false;

	this->setCascadeOpacityEnabled(true);

	m_iconSp = Sprite::create();
	m_iconSp->setAnchorPoint(Point::ANCHOR_MIDDLE_LEFT);
	m_iconSp->setVisible(false);
	this->addChild(m_iconSp);

	m_textLabel = Label::createWithSystemFont("Text", DEFAULT_SYSTEM_FONT, FONT_SIZE);
	m_textLabel->setAnchorPoint(Point::ANCHOR_MIDDLE_LEFT);
	m_textLabel->setAlignment(TextHAlignment::LEFT);
	this->setSeverity(INFO);
	this->addChild(m_textLabel);

	this->setVisible(false);

	return true;
}

void MessageBar::show(std::string const& text, Severity severity, std::string const& icon, float duration)
{
	if (Action* action = this->getActionByTag(ACTION_TAG_SHOW_OR_HIDE))
		this->stopAction(action);

	this->setText(text);
	this->setSeverity(severity);
	this->setIcon(icon);

	this->setOpacity(0);

	Vector<FiniteTimeAction*> actions;
	actions.pushBack(Show::create());
	actions.pushBack(FadeTo::create(0.5f, 255));
	if (duration > 0)
	{
		CallFunc* callfunc = CallFunc::create([this]() {
			this->hide();
		});
		actions.pushBack(DelayTime::create(duration));
		actions.pushBack(callfunc);
	}

	Sequence* action = Sequence::create(actions);
	action->setTag(ACTION_TAG_SHOW_OR_HIDE);
	this->runAction(action);

	this->layoutElements();

	m_isShown = true;
}

void MessageBar::hide()
{
	if (!m_isShown)
		return;

	m_isShown = false;

	if(Action* action = this->getActionByTag(ACTION_TAG_SHOW_OR_HIDE))
		this->stopAction(action);

	Vector<FiniteTimeAction*> actions;
	actions.pushBack(FadeTo::create(1.0f, 0));
	actions.pushBack(Hide::create());
	Sequence* action = Sequence::create(actions);
	action->setTag(ACTION_TAG_SHOW_OR_HIDE);
	this->runAction(action);
}

void MessageBar::onEnter()
{
	Node::onEnter();
}

void MessageBar::setText(std::string const& text)
{
	m_textLabel->setString(text);
}

void MessageBar::setIcon(std::string const& frameName)
{
	if (!frameName.empty())
	{
		m_iconSp->setVisible(true);
		m_iconSp->setSpriteFrame(frameName);
	}
	else
		m_iconSp->setVisible(false);
}

void MessageBar::setSeverity(Severity severity)
{
	switch (severity)
	{
	case NOTICE:
		m_textLabel->setTextColor(Color4B(250, 180, 5, 255));
		break;
	case ALERT:
		m_textLabel->setTextColor(Color4B(224, 41, 27, 255));
		break;
	default: // INFO
		m_textLabel->setTextColor(Color4B(63, 183, 0, 255));
		break;
	}
	Utils::enableBoldForLabel(m_textLabel);
}


void MessageBar::layoutElements()
{
	float maxHeight = std::max(m_iconSp->getBoundingBox().size.height, m_textLabel->getBoundingBox().size.height);
	float nextX = 0;
	if (m_iconSp->isVisible())
	{
		m_iconSp->setPosition(nextX, maxHeight / 2);
		nextX = m_iconSp->getBoundingBox().getMaxX();
	}
	m_textLabel->setPosition(nextX, maxHeight / 2);
	nextX = m_textLabel->getBoundingBox().getMaxX();

	this->setContentSize(Size(nextX, maxHeight));
}


NS_END

