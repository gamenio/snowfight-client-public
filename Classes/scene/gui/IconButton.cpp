#include "IconButton.h"

#include "scene/Utils.h"

NS_BEGIN

using namespace cocos2d::ui;

static const float ZOOM_ACTION_TIME_STEP = 0.05f;

IconButton* IconButton::create(std::string const& backgroundImage, std::string const& title, std::string const& iconImage)
{
	IconButton* pRet = new IconButton();
	if (pRet && pRet->init(backgroundImage, title, iconImage))
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

bool IconButton::init(std::string const& backgroundImage, std::string const& title, std::string const& iconImage)
{
	if (!Widget::init())
		return false;

	this->ignoreContentAdaptWithSize(false);

	m_backgroundContainer->setAnchorPoint(Point::ANCHOR_MIDDLE);
	m_backgroundContainer->setIgnoreAnchorPointForPosition(false);

	m_backgroundRenderer->initWithSpriteFrameName(backgroundImage);

	m_titleContainer->setAnchorPoint(Point::ANCHOR_MIDDLE);
	m_titleContainer->setIgnoreAnchorPointForPosition(false);

	m_iconRenderer->initWithSpriteFrameName(iconImage);
	m_iconRenderer->setAnchorPoint(Point::ANCHOR_MIDDLE_LEFT);

	m_titleRenderer->setString(title);
	m_titleRenderer->setAnchorPoint(Point::ANCHOR_MIDDLE_LEFT);

	return true;
}

IconButton::IconButton() :
	m_zoomScale(0.1f),
	m_backgroundContainer(nullptr),
	m_backgroundRenderer(nullptr),
	m_backgroundIconRenderer(nullptr),
	m_iconRenderer(nullptr),
	m_titleRenderer(nullptr)
{
	setTouchEnabled(true);
}

IconButton::~IconButton()
{
	m_backgroundContainer = nullptr;
	m_backgroundRenderer = nullptr;
	m_backgroundIconRenderer = nullptr;
	m_iconRenderer = nullptr;
	m_titleRenderer = nullptr;
}


void IconButton::setBackgroundIconImage(std::string const& image)
{
	if (!m_backgroundIconRenderer)
	{
		m_backgroundIconRenderer = Sprite::createWithSpriteFrameName(image);
		m_backgroundIconRenderer->setAnchorPoint(Point::ANCHOR_MIDDLE);
		m_backgroundContainer->addChild(m_backgroundIconRenderer);
	}
	else
		m_backgroundIconRenderer->setSpriteFrame(image);

	this->updateBackgroundIconPosition();
}


void IconButton::setBackgroundImage(std::string const& image)
{
	m_backgroundRenderer->setSpriteFrame(image);
}

void IconButton::setTitleText(std::string const& text)
{
	m_titleRenderer->setString(text);
	Utils::enableBoldForLabel(m_titleRenderer);
	m_titleAdaptDirty = true;
}


void IconButton::setTitleColor(Color4B const& color)
{
	m_titleRenderer->setTextColor(color);
	Utils::enableBoldForLabel(m_titleRenderer);
}


void IconButton::setTitleFontSize(float size)
{
	m_titleRenderer->setSystemFontSize(size);
	Utils::enableBoldForLabel(m_titleRenderer);
	m_titleAdaptDirty = true;
}

void IconButton::setZoomScale(float scale)
{
	m_zoomScale = scale;
}

float IconButton::getZoomScale()const
{
	return m_zoomScale;
}

void IconButton::initRenderer()
{
	m_backgroundContainer = Node::create();

	m_backgroundRenderer = Scale9Sprite::create();
	m_backgroundRenderer->setRenderingType(Scale9Sprite::RenderingType::SLICE);
	m_backgroundContainer->addChild(m_backgroundRenderer);

	m_titleContainer = Node::create();

	m_iconRenderer = Sprite::create();
	m_titleContainer->addChild(m_iconRenderer);

	m_titleRenderer = Label::create();
	m_titleContainer->addChild(m_titleRenderer);

	addProtectedChild(m_backgroundContainer);
	addProtectedChild(m_titleContainer);
}

void IconButton::onPressStateChangedToNormal()
{
	m_backgroundRenderer->setState(Scale9Sprite::State::NORMAL);
	m_backgroundContainer->stopAllActions();
	m_backgroundContainer->setScale(1.0);

	m_titleContainer->stopAllActions();
	if (_unifySize)
	{
		Action *zoomTitleAction = ScaleTo::create(ZOOM_ACTION_TIME_STEP, 1.0f, 1.0f);
		m_titleContainer->runAction(zoomTitleAction);
	}
	else
	{
		m_titleContainer->setScaleX(1.0f);
		m_titleContainer->setScaleY(1.0f);
	}
}

void IconButton::onPressStateChangedToPressed()
{
	m_backgroundRenderer->setState(Scale9Sprite::State::NORMAL);
	m_backgroundContainer->stopAllActions();
	Action *zoomAction = ScaleTo::create(ZOOM_ACTION_TIME_STEP, 1.0f + m_zoomScale, 1.0f + m_zoomScale);
	m_backgroundContainer->runAction(zoomAction);

	m_titleContainer->stopAllActions();
	Action *zoomTitleAction = ScaleTo::create(ZOOM_ACTION_TIME_STEP, 1.0f + m_zoomScale, 1.0f + m_zoomScale);
	m_titleContainer->runAction(zoomTitleAction);
}

void IconButton::onPressStateChangedToDisabled()
{

}

void IconButton::onSizeChanged()
{
	Widget::onSizeChanged();

	this->updateBackgroundIconPosition();

	m_backgroundAdaptDirty = true;
	m_titleAdaptDirty = true;
}


void IconButton::adaptRenderers()
{
	if (m_backgroundAdaptDirty)
	{
		m_backgroundContainer->setContentSize(_contentSize);
		m_backgroundContainer->setPosition(_contentSize.width / 2, _contentSize.height / 2);

		m_backgroundRenderer->setPreferredSize(m_backgroundContainer->getContentSize());
		m_backgroundRenderer->setPosition(m_backgroundContainer->getContentSize().width / 2.0f, m_backgroundContainer->getContentSize().height / 2.0f);

		m_backgroundAdaptDirty = false;
	}
	
	if (m_titleAdaptDirty)
	{
		float titleWidth = m_iconRenderer->getContentSize().width + m_titleRenderer->getContentSize().width;
		float titleHeight = MAX(m_iconRenderer->getContentSize().height, m_titleRenderer->getContentSize().height);
		m_titleContainer->setContentSize(Size(titleWidth, titleHeight));
		m_titleContainer->setPosition(_contentSize.width / 2, _contentSize.height / 2);

		m_iconRenderer->setPosition(0, m_titleContainer->getContentSize().height / 2);
		m_titleRenderer->setPosition(m_iconRenderer->getBoundingBox().getMaxX(), m_titleContainer->getContentSize().height / 2);

		m_titleAdaptDirty = false;
	}
}

void snowfight::IconButton::updateBackgroundIconPosition()
{
	if (!m_backgroundIconRenderer)
		return;
	m_backgroundIconRenderer->setPosition(0, _contentSize.height / 2);
}


NS_END