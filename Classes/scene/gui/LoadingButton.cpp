//
//  LoadingButton.cpp
//  snowfight
//
//  Created by Luthier on 2021/8/1.
//

#include "LoadingButton.h"

#include "scene/Utils.h"
#include "scene/SoundMgr.h"

using namespace cocos2d::ui;

NS_BEGIN

LoadingButton* LoadingButton::create(std::string const& title, Size const& loadingIndicatorSize, std::string const& normalImage, std::string const& selectedImage, std::string const& disableImage)
{
    auto ret = new (std::nothrow) LoadingButton();
    if (ret && ret->init(title, loadingIndicatorSize, normalImage, selectedImage, disableImage))
    {
        ret->autorelease();
    }
    else
    {
        CC_SAFE_DELETE(ret);
    }
    
    return ret;
}

bool LoadingButton::init(std::string const& title, Size const& loadingIndicatorSize, std::string const& normalImage, std::string const& selectedImage, std::string const& disableImage)
{
    if (!Widget::init())
        return false;
    
	Label* titleLabel = Label::createWithSystemFont(title, DEFAULT_SYSTEM_FONT, 13);
	titleLabel->setAnchorPoint(Point::ANCHOR_MIDDLE);
	Utils::enableBoldForLabel(titleLabel);
	m_button = Button::create(normalImage, selectedImage, disableImage, Widget::TextureResType::PLIST);
	m_button->setPosition(Vec2(this->getContentSize().width / 2, this->getContentSize().height / 2));
	m_button->setContentSize(this->getContentSize());
	m_button->setScale9Enabled(true);
    m_button->addClickEventListener(CC_CALLBACK_1(LoadingButton::buttonClickCallback, this));
    m_button->setTitleLabel(titleLabel);
    this->addChild(m_button);
    
    m_loadingIndicator = LoadingIndicator::create(loadingIndicatorSize);
	m_loadingIndicator->setPosition(Vec2(this->getContentSize().width / 2, this->getContentSize().height / 2));
    m_loadingIndicator->setVisible(false);
    this->addChild(m_loadingIndicator);
    
    return true;
}

LoadingButton::LoadingButton() :
    m_button(nullptr),
    m_loadingIndicator(nullptr)
{
    
}

LoadingButton::~LoadingButton()
{
    m_button = nullptr;
    m_loadingIndicator = nullptr;
}

void LoadingButton::setVisible(bool visible)
{
    if(!visible)
        this->setShowLoadingIndicator(false);

	Widget::setVisible(visible);
}

void LoadingButton::setTitle(std::string const& title)
{
	Label* titleLabel = m_button->getTitleRenderer();
	//if (title.length() > 6)
	//	titleLabel->setSystemFontSize(10);
	//else if (title.length() > 5)
	//	titleLabel->setSystemFontSize(11);
	//else
	//	titleLabel->setSystemFontSize(12);

	titleLabel->setString(title);
}

void LoadingButton::setTitleColor(Color4B const& color)
{
	Label* titleLabel = m_button->getTitleRenderer();
	titleLabel->setTextColor(color);
	Utils::enableBoldForLabel(titleLabel);
}

void LoadingButton::setTitleFontSize(float fontSize)
{
	Label* titleLabel = m_button->getTitleRenderer();
	titleLabel->setSystemFontSize(fontSize);
}

void LoadingButton::setEnabled(bool enabled)
{
	Widget::setEnabled(enabled);
    m_button->setEnabled(enabled);
}

void LoadingButton::setTouchEnabled(bool enabled)
{
	Widget::setTouchEnabled(enabled);
	m_button->setTouchEnabled(enabled);
}

void LoadingButton::setLoadingIndicatorColor(Color3B const& color)
{
	m_loadingIndicator->setTintColor(color);
}

void LoadingButton::setShowLoadingIndicator(bool show)
{
    m_loadingIndicator->setVisible(show);
    m_button->getTitleLabel()->setVisible(!show);
}

void LoadingButton::setClickEventListener(LoadingButtonClickCallback const& callback)
{
    m_clickCallback = callback;
}

void LoadingButton::onEnter()
{
	Widget::onEnter();

	this->layoutElements();
}

void LoadingButton::layoutElements()
{
	m_button->setPosition(Vec2(this->getContentSize().width / 2, this->getContentSize().height / 2));
	m_button->setContentSize(this->getContentSize());

	m_loadingIndicator->setPosition(Vec2(this->getContentSize().width / 2, this->getContentSize().height / 2));
}

void LoadingButton::buttonClickCallback(Ref* sender)
{
    sSoundMgr->play(SOUND_BUTTON);
    if(m_clickCallback)
        m_clickCallback(this);
}

NS_END
