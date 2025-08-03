#include "Toaster.h"

NS_BEGIN

#define MAX_NUMBER_OF_TOASTS			3

#define LINE_SPACING					1
#define TOAST_OPACITY_MIN				76


Toaster::Toaster() :
	m_mainContainer(nullptr)
{
}

Toaster::~Toaster()
{
	m_mainContainer = nullptr;
}

Toaster* Toaster::create()
{
	Toaster* pRet = new Toaster();
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

bool Toaster::init()
{
	if (!Node::init())
		return false;

	m_mainContainer = Node::create();
	m_mainContainer->setPosition(Point::ZERO);
	m_mainContainer->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
	this->addChild(m_mainContainer);

	this->layoutElements();

	return true;
}

void Toaster::addToast(std::string const& text, MessageToast::Severity severity, float duration, int32 tag, bool flickering)
{
	MessageToast* toast = MessageToast::create();
	toast->setDuration(duration);
	toast->setTag(tag);
	toast->setText(text);
	toast->setSeverity(severity);
	toast->setFlickering(flickering);
	this->addToast(toast);
}

void Toaster::addToast(Toast* toast)
{
	// 如果超出Toast数量限制则将旧的Toast移除
	auto const& toasts = m_mainContainer->getChildren();
	if (toasts.size() >= MAX_NUMBER_OF_TOASTS)
	{
		Toast* toast = dynamic_cast<Toast*>(*toasts.begin());
		if (toast && toast->getDuration() != TOAST_DURATION_FOREVER)
			toast->removeFromParent();
	}

	if (toasts.size() < MAX_NUMBER_OF_TOASTS)
	{
		if (toast->getDuration() == TOAST_DURATION_FOREVER)
			toast->setLocalZOrder(MAX_NUMBER_OF_TOASTS - 1);

		m_mainContainer->addChild(toast);
		m_mainContainer->sortAllChildren();

		this->layoutElements();
	}
}

void Toaster::removeToastByTag(int32 tag)
{
	if (Toast* toast = dynamic_cast<Toast*>(m_mainContainer->getChildByTag(tag)))
		toast->hide();
}

void Toaster::layoutElements()
{
	float nextY = 0;
	float maxWidth = 0;

	Vector<Node*> const& toasts = m_mainContainer->getChildren();
	for (ssize_t i = toasts.size() - 1; i >= 0; --i)
	{
		Toast* toast = dynamic_cast<Toast*>(toasts.at(i));
		if (!toast)
			continue;

		if(toast->isHiding() && toast->getOpacity() < TOAST_OPACITY_MIN)
			continue;

		toast->setPosition(Vec2(0, nextY));
		toast->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
		nextY = toast->getBoundingBox().getMaxY();
		if (i > 0)
			nextY += LINE_SPACING;
		maxWidth = MAX(maxWidth, toast->getContentSize().width);
	}


	Size contentSize(maxWidth, nextY);
	m_mainContainer->setContentSize(contentSize);
	this->setContentSize(contentSize);
}



NS_END