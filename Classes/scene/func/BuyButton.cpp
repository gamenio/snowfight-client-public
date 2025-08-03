//
//  BuyButton.cpp
//  snowfight
//
//  Created by Luthier on 2019/5/9.
//

#include "BuyButton.h"
#include "scene/Utils.h"

using namespace cocos2d::ui;

NS_BEGIN

#define FRAMENAME_BUY_BTN_CURRENCY            "buy_btn_currency.png"
#define FRAMENAME_BUY_BTN_GAMECOIN            "buy_btn_gamecoin.png"

#define LOADING_INDICATOR_SIZE               Size(43, 7)

BuyButton* BuyButton::create(MoneyType type)
{
    auto ret = new (std::nothrow) BuyButton();
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

bool BuyButton::init(MoneyType type)
{
    if (!Node::init())
        return false;
    
	m_type = type;

	Label* titleLabel = Label::createWithSystemFont("", DEFAULT_SYSTEM_FONT, 12);
	switch (type)
	{
	case MONEY_TYPE_CURRENCY:
		m_button = Button::create(FRAMENAME_BUY_BTN_CURRENCY, "", "", Widget::TextureResType::PLIST);
		titleLabel->setTextColor(Color4B(233, 242, 231, 255));
		break;
	default:
		m_button = Button::create(FRAMENAME_BUY_BTN_GAMECOIN, "", "", Widget::TextureResType::PLIST);
		titleLabel->setTextColor(Color4B(233, 233, 244, 255));
		break;
	}
    m_button->addClickEventListener(CC_CALLBACK_1(BuyButton::buttonClickCallback, this));
    m_button->setTitleLabel(titleLabel);
    titleLabel->setAnchorPoint(Point::ANCHOR_MIDDLE);
	Utils::enableBoldForLabel(titleLabel);
    titleLabel->setPosition(m_button->getContentSize().width / 2, 29);
    this->addChild(m_button);
    
    m_loadingIndicator = LoadingIndicator::create(LOADING_INDICATOR_SIZE, Color3B(247, 242, 205));
    m_loadingIndicator->setPosition(Vec2(0.0f, -4.0f));
    m_loadingIndicator->setVisible(false);
    this->addChild(m_loadingIndicator);
    
    return true;
}

BuyButton::BuyButton() :
	m_type(MONEY_TYPE_CURRENCY),
    m_button(nullptr),
    m_loadingIndicator(nullptr)
{
    
}

BuyButton::~BuyButton()
{
    m_button = nullptr;
    m_loadingIndicator = nullptr;
}

void BuyButton::setVisible(bool visible)
{
    if(!visible)
        this->setShowLoadingIndicator(false);

    Node::setVisible(visible);
}

void BuyButton::setTitle(std::string const& title)
{
	Label* titleLabel = m_button->getTitleRenderer();
	if (title.length() > 6)
		titleLabel->setSystemFontSize(10);
	else if (title.length() > 5)
		titleLabel->setSystemFontSize(11);
	else
		titleLabel->setSystemFontSize(12);

	titleLabel->setString(title);
}

void BuyButton::setEnabled(bool enabled)
{
    m_button->setEnabled(enabled);
}

void BuyButton::setShowLoadingIndicator(bool show)
{
    m_loadingIndicator->setVisible(show);
    m_button->getTitleLabel()->setVisible(!show);
}

void BuyButton::setClickEventListener(BuyButtonClickCallback const& callback)
{
    m_clickCallback = callback;
}

void BuyButton::buttonClickCallback(Ref* sender)
{
    if(m_clickCallback)
        m_clickCallback(this, m_type);
}

NS_END
