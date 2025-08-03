#include "GameCoinSaleItem.h"

#include "game/LocaleMgr.h"
#include "game/store/Store.h"
#include "scene/Utils.h"

NS_BEGIN

USING_NS_CC;
using namespace cocos2d::ui;

#define ITEM_MARGIN_LEFTRIGHT				12
#define ITEM_MARGIN_TOPBOTTOM				12

#define PRODUCT_GROUP_HEIGHT				119

#define FRAMENAME_ITEM_DIVIDER				"divider.png"
#define FRAMENAME_ITEM_BG					"gamecoin_item_bg.png"
#define FRAMENAME_SHOP_BUY_BTN				"shop_buy_btn.png"

#define ITEM_PIC_FRAME_FORMAT				"gamecoin_item_pic%d.png"

#define SHOP_BUY_BTN_SIZE					Size(111, 33)
#define SHOP_BUY_LOADING_INDICATOR_SIZE		Size(43, 7)

GameCoinSaleItem::GameCoinSaleItem() :
	m_data(nullptr),
	m_dividerSp(nullptr),
	m_productGroup(nullptr),
	m_picSp(nullptr),
	m_bgSp(nullptr),
	m_amountLabel(nullptr),
	m_nameLabel(nullptr),
	m_buyBtn(nullptr)
{
}

GameCoinSaleItem::~GameCoinSaleItem()
{
	m_data = nullptr;
	m_dividerSp = nullptr;
	m_productGroup = nullptr;
	m_picSp = nullptr;
	m_bgSp = nullptr;
	m_amountLabel = nullptr;
	m_nameLabel = nullptr;
	m_buyBtn = nullptr;
}

GameCoinSaleItem* GameCoinSaleItem::create(Size const& size)
{
	GameCoinSaleItem* ret = new (std::nothrow) GameCoinSaleItem();
	if (ret && ret->init(size))
	{
		ret->autorelease();
		return ret;
	}
	else
	{
		CC_SAFE_DELETE(ret);
		return nullptr;
	}
}

bool GameCoinSaleItem::init(Size const& size)
{
	if (!GameCoinItem::init(size))
		return false;

	m_dividerSp = Sprite::createWithSpriteFrameName(FRAMENAME_ITEM_DIVIDER);
	m_dividerSp->setPosition(Vec2(size.width, size.height - ITEM_MARGIN_TOPBOTTOM));
	m_dividerSp->setAnchorPoint(Point::ANCHOR_TOP_RIGHT);
	m_dividerSp->setContentSize(Size(1, PRODUCT_GROUP_HEIGHT));
	this->addChild(m_dividerSp);

	m_productGroup = Widget::create();
	m_productGroup->setPosition(Vec2(ITEM_MARGIN_LEFTRIGHT, m_dividerSp->getBoundingBox().getMaxY()));
	m_productGroup->setAnchorPoint(Point::ANCHOR_TOP_LEFT);
	m_productGroup->setContentSize(Size(size.width - ITEM_MARGIN_LEFTRIGHT * 2 - m_dividerSp->getContentSize().width, PRODUCT_GROUP_HEIGHT));
	this->addChild(m_productGroup);

	m_bgSp = Scale9Sprite::createWithSpriteFrameName(FRAMENAME_ITEM_BG);
	m_bgSp->setPosition(Point::ZERO);
	m_bgSp->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
	m_bgSp->setContentSize(m_productGroup->getContentSize());
	m_productGroup->addChild(m_bgSp);

	m_picSp = Sprite::createWithSpriteFrameName(StringUtils::format(ITEM_PIC_FRAME_FORMAT, GAME_COIN_PILE_OF_GOLDS));
	m_picSp->setPosition(Vec2(m_productGroup->getContentSize().width / 2, m_productGroup->getContentSize().height - m_picSp->getContentSize().height / 2 - 2));
	m_picSp->setAnchorPoint(Point::ANCHOR_MIDDLE);
	m_productGroup->addChild(m_picSp);

	m_nameLabel = Label::createWithSystemFont("Name", DEFAULT_SYSTEM_FONT, 10);
	m_nameLabel->setTextColor(Color4B(132, 111, 52, 255));
	m_nameLabel->setPosition(Vec2(m_productGroup->getContentSize().width / 2, 14));
	m_nameLabel->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
	Utils::enableBoldForLabel(m_nameLabel);
	m_productGroup->addChild(m_nameLabel);

	m_amountLabel = Label::createWithSystemFont("9999", DEFAULT_SYSTEM_FONT, 14);
	m_amountLabel->setTextColor(Color4B(132, 111, 52, 255));
	m_amountLabel->setPosition(Vec2(m_productGroup->getContentSize().width / 2, m_nameLabel->getBoundingBox().getMaxY() + 15));
	Utils::enableBoldForLabel(m_amountLabel);
	m_productGroup->addChild(m_amountLabel);

	m_buyBtn = LoadingButton::create("$99.99", SHOP_BUY_LOADING_INDICATOR_SIZE, FRAMENAME_SHOP_BUY_BTN);
	m_buyBtn->setLoadingIndicatorColor(Color3B(247, 242, 205));
	m_buyBtn->setTitleColor(Color4B(242, 242, 235, 255));
	m_buyBtn->setTitleFontSize(13);
	m_buyBtn->setAnchorPoint(Point::ANCHOR_MIDDLE_BOTTOM);
	m_buyBtn->setPosition(Vec2(m_productGroup->getBoundingBox().getMidX(), ITEM_MARGIN_TOPBOTTOM));
	m_buyBtn->setContentSize(SHOP_BUY_BTN_SIZE);
	this->addChild(m_buyBtn);

	return true;
}

 void GameCoinSaleItem::setData(GameCoinTemplate const* data)
 {
	 m_data = data;
	 this->reloadData();
 }

 void GameCoinSaleItem::reloadData()
 {
	 m_picSp->setSpriteFrame(StringUtils::format(ITEM_PIC_FRAME_FORMAT, m_data->id));

	 m_nameLabel->setString(m_data->name);
	 m_amountLabel->setString(Utils::separateThousands(m_data->amount, true));

	 if (sStore->isValidatingProductIds())
	 {
		 m_buyBtn->setShowLoadingIndicator(true);
	 }
	 else
	 {
		 m_buyBtn->setShowLoadingIndicator(false);
		 ProductInfo const* product = sStore->getProductById(m_data->productId);
		 if (product)
		 {
			 std::string priceStr = StringUtils::format("%s", product->localizedPrice.c_str());
			 m_buyBtn->setTitle(priceStr);
		 }
		 else
		 {
			 m_buyBtn->setEnabled(false);
			 m_buyBtn->setTitle(sLocaleMgr->getString("btn_title_not_available"));
		 }
	 }
 }

 void GameCoinSaleItem::setLastItem(bool isLastItem)
 {
	 m_dividerSp->setVisible(!isLastItem);
 }

 void GameCoinSaleItem::reset()
 {
 }

 Sprite* GameCoinSaleItem::clonePicture()
 {
	 Sprite* sprite = Sprite::createWithSpriteFrame(m_picSp->getSpriteFrame());
	 sprite->setPosition(m_picSp->getPosition());
	 sprite->setAnchorPoint(m_picSp->getAnchorPoint());
	 m_productGroup->addChild(sprite);
	 return sprite;
 }

 void GameCoinSaleItem::setTouchEnabled(bool enabled)
 {
	 GameCoinItem::setTouchEnabled(enabled);
	 m_buyBtn->setTouchEnabled(enabled);
 }

 void GameCoinSaleItem::onEnter()
{
	Widget::onEnter();
}

NS_END