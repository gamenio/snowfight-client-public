//
//  FuncButton.cpp
//  snowfight
//
//  Created by Luthier on 2020/07/05.
//

#include "FuncButton.h"
#include "game/UserPreferences.h"
#include "game/store/Store.h"
#include "game/gamecenter/GameCenter.h"
#include "game/LocaleMgr.h"
#include "scene/Utils.h"

using namespace cocos2d::ui;

NS_BEGIN

#define FRAMENAME_FUNC_BTN_PLAY				"func_btn_play.png"

FuncButton* FuncButton::create()
{
	auto ret = new (std::nothrow) FuncButton();
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

bool FuncButton::init()
{
	if (!Node::init())
		return false;

	SpriteFrame* frame = SpriteFrameCache::getInstance()->getSpriteFrameByName(FRAMENAME_FUNC_BTN_PLAY);
	this->setContentSize(frame->getOriginalSize());

	m_playBtn = Button::create(FRAMENAME_FUNC_BTN_PLAY, "", FRAMENAME_FUNC_BTN_PLAY, Widget::TextureResType::PLIST);
	m_playBtn->setAnchorPoint(Point::ANCHOR_MIDDLE);
	m_playBtn->setPosition(Vec2(this->getContentSize().width / 2, this->getContentSize().height / 2));
	this->addChild(m_playBtn);

	m_currencyBuyBtn = BuyButton::create(MONEY_TYPE_CURRENCY);
	m_currencyBuyBtn->setVisible(false);
	m_currencyBuyBtn->setAnchorPoint(m_playBtn->getAnchorPoint());
	m_currencyBuyBtn->setPosition(m_playBtn->getPosition());
	this->addChild(m_currencyBuyBtn);

	m_gameCoinBuyBtn = BuyButton::create(MONEY_TYPE_GAME_COIN);
	m_gameCoinBuyBtn->setVisible(false);
	m_gameCoinBuyBtn->setAnchorPoint(m_playBtn->getAnchorPoint());
	m_gameCoinBuyBtn->setPosition(m_playBtn->getPosition());
	this->addChild(m_gameCoinBuyBtn);

	return true;
}

FuncButton::FuncButton() :
	m_template(nullptr),
	m_playBtn(nullptr),
	m_currencyBuyBtn(nullptr),
	m_gameCoinBuyBtn(nullptr)
{

}

FuncButton::~FuncButton()
{
	m_template = nullptr;
	m_playBtn = nullptr;
	m_currencyBuyBtn = nullptr;
	m_gameCoinBuyBtn = nullptr;
}

void FuncButton::setHeroTemplate(HeroTemplate const* tmpl)
{
	m_template = tmpl;
	this->updateButton();
}

void FuncButton::updateButton()
{
	m_playBtn->setVisible(false);
	m_playBtn->setEnabled(true);

	m_currencyBuyBtn->setVisible(false);
	m_currencyBuyBtn->setEnabled(true);

	m_gameCoinBuyBtn->setVisible(false);
	m_gameCoinBuyBtn->setEnabled(true);

	if (sGameCenter->getLocalPlayer()->hasUnlockedHero(m_template->id))
	{
		m_playBtn->setVisible(true);
	}
	else
	{
		if (m_template->productType != PRODUCT_TYPE_PRIZE)
		{
			m_currencyBuyBtn->setVisible(true);
			if (sStore->isValidatingProductIds())
			{
				m_currencyBuyBtn->setShowLoadingIndicator(true);
			}
			else
			{
				m_currencyBuyBtn->setShowLoadingIndicator(false);
				ProductInfo const* product = sStore->getProductById(m_template->productId);
				if (product)
				{
					std::string priceStr = StringUtils::format("%s", product->localizedPrice.c_str());
					m_currencyBuyBtn->setTitle(priceStr);
				}
				else
				{
					m_currencyBuyBtn->setEnabled(false);
					m_currencyBuyBtn->setTitle(sLocaleMgr->getString("btn_title_not_available"));
				}
			}
		}
		else
		{
			m_gameCoinBuyBtn->setVisible(true);
			m_gameCoinBuyBtn->setTitle(Utils::separateThousands(m_template->price));
		}
	}
}

void FuncButton::setEnabled(bool enabled)
{
	m_playBtn->setEnabled(enabled);
	m_currencyBuyBtn->setEnabled(enabled);
	m_gameCoinBuyBtn->setEnabled(enabled);
}

void FuncButton::setBuyButtonClickEventListener(BuyButtonClickCallback const& callback)
{
	m_gameCoinBuyBtn->setClickEventListener(callback);
	m_currencyBuyBtn->setClickEventListener(callback);
}

void FuncButton::setPlayButtonClickEventListener(ui::Widget::ccWidgetClickCallback const& callback)
{
	m_playBtn->addClickEventListener(callback);
}

NS_END
