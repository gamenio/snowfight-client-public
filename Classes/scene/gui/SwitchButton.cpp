#include "SwitchButton.h"

#include "2d/CCSpriteFrameCache.h"
#include "2d/CCCamera.h"
#include "ui/UIScale9Sprite.h"

#include "../SoundMgr.h"

NS_BEGIN

using namespace cocos2d::ui;

SwitchButton* SwitchButton::create(std::string const& offImage, std::string const& onImage, TextureResType texType)
{
    SwitchButton* btn = new (std::nothrow) SwitchButton;
    if (btn && btn->init(offImage, onImage, texType))
	{
        btn->autorelease();
    } 
	else 
	{
        CC_SAFE_DELETE(btn);
        btn = nullptr;
    }
    return btn;
}

bool SwitchButton::init(std::string const& offImage, std::string const& onImage, TextureResType texType)
{
	if (!ui::Widget::init())
		return false;

	m_texType = texType;

	switch (m_texType)
	{
	case TextureResType::LOCAL:
		m_switchOffRender->initWithFile(offImage);
		m_switchOnRender->initWithFile(onImage);
		break;
	case TextureResType::PLIST:
		m_switchOffRender->initWithSpriteFrameName(offImage);
		m_switchOnRender->initWithSpriteFrameName(onImage);
		break;
	default:
		break;
	}
	m_switchOffRender->setAnchorPoint(Point::ANCHOR_MIDDLE);
	m_switchOnRender->setAnchorPoint(Point::ANCHOR_MIDDLE);

	updateContentSizeWithTextureSize(m_switchOffRender->getContentSize());

    return true;
}

SwitchButton::SwitchButton() :
	m_isOn(false),
	m_switchOnRender(nullptr),
	m_switchOffRender(nullptr),
	m_texType(TextureResType::LOCAL),
	m_rendersAdaptDirty(true)
{
	setTouchEnabled(true);
}

SwitchButton::~SwitchButton()
{
}


bool SwitchButton::revertSwitch()
{
    setOn(!m_isOn);
    return m_isOn;
}

bool SwitchButton::isOn()
{
    return m_isOn;
}

void SwitchButton::setOn(bool on)
{
    if (on == m_isOn){
        return;
    }

	m_isOn = on;

	_brightStyle = BrightStyle::NONE;
	setBright(_bright);
}

void SwitchButton::onSizeChanged()
{
	Widget::onSizeChanged();
}

void SwitchButton::adaptRenderers()
{
	Widget::adaptRenderers();

	if (!m_rendersAdaptDirty)
		return;

	Size contentSize = this->getContentSize();

	m_switchOffRender->setPosition(contentSize.width / 2, contentSize.height / 2);
	m_switchOnRender->setPosition(contentSize.width / 2, contentSize.height / 2);

	m_rendersAdaptDirty = false;
}

void SwitchButton::initRenderer()
{
	m_switchOnRender = Scale9Sprite::create();
	m_switchOnRender->setRenderingType(ui::Scale9Sprite::RenderingType::SIMPLE);

	m_switchOffRender = Scale9Sprite::create();
	m_switchOffRender->setRenderingType(ui::Scale9Sprite::RenderingType::SIMPLE);


	addProtectedChild(m_switchOnRender);
	addProtectedChild(m_switchOffRender);
}

void SwitchButton::onPressStateChangedToNormal()
{
	if (m_isOn)
	{
		m_switchOnRender->setVisible(true);
		m_switchOnRender->setState(Scale9Sprite::State::NORMAL);
		m_switchOffRender->setVisible(false);
	}
	else
	{
		m_switchOnRender->setVisible(false);
		m_switchOffRender->setVisible(true);
		m_switchOffRender->setState(Scale9Sprite::State::NORMAL);
	}
}

void SwitchButton::onPressStateChangedToPressed()
{
}


void SwitchButton::onPressStateChangedToDisabled()
{
	if (m_isOn)
	{
		m_switchOnRender->setVisible(true);
		m_switchOnRender->setState(Scale9Sprite::State::GRAY);
		m_switchOffRender->setVisible(false);
	}
	else
	{
		m_switchOnRender->setVisible(false);
		m_switchOffRender->setVisible(true);
		m_switchOffRender->setState(Scale9Sprite::State::GRAY);
	}
}

bool SwitchButton::onTouchBegan(Touch* touch, Event* unusedEvent)
{
    return Widget::onTouchBegan(touch, unusedEvent);
}

void SwitchButton::onTouchMoved(Touch* touch, Event* unusedEvent)
{
	Widget::onTouchMoved(touch, unusedEvent);
}

void SwitchButton::onTouchEnded(Touch* touch, Event* unusedEvent)
{
	_touchEndPosition = touch->getLocation();

	/*
	* Propagate touch events to its parents
	*/
	if (_propagateTouchEvents)
	{
		this->propagateTouchEvent(TouchEventType::ENDED, this, touch);
	}

	if (_highlight)
	{
		this->revertSwitch();
	}

	bool highlight = _highlight;
	setHighlighted(false);
	if (highlight)
	{
		releaseUpEvent();
        sSoundMgr->play(SOUND_SWITCH);
	}
	else
	{
		cancelUpEvent();
	}
}

void SwitchButton::onTouchCancelled(Touch* touch, Event *unusedEvent)
{
	Widget::onTouchCancelled(touch, unusedEvent);
}



NS_END
