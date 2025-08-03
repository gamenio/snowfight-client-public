#include "RemoveAdsDialog.h"

#include "../SoundMgr.h"
#include "scene/Utils.h"
#include "game/store/Store.h"
#include "game/ShopMgr.h"

using namespace cocos2d::ui;

NS_BEGIN

#define FRAME_SIZE							Size(280, 165)

#define BUTTON_WIDTH						110
#define BUTTON_PANEL_HEIGHT					36
#define BUTTON_PANEL_MARGIN_LEFTRIGHT       9
#define BUTTON_PANEL_MARGIN_BOTTOM			32

#define TITLE_BAR_MIN_WIDTH					163
#define TITLE_BAR_MAX_WIDTH					224
#define TITLE_BAR_HEIGHT					42
#define TITLE_MARGIN_LEFTRIGHT				27

#define CONTENT_MARGIN_LEFTRIGHT			30

#define BUY_LOADING_INDICATOR_SIZE			Size(43, 7)

RemoveAdsDialog* RemoveAdsDialog::create()
{
	auto ret = new (std::nothrow) RemoveAdsDialog();
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

bool RemoveAdsDialog::init()
{
	if (!ModalDialog::init(FRAME_SIZE))
		return false;


	sStore->addListener(this);

	Node* titleBar = Node::create();
	titleBar->setPosition(FRAME_SIZE.width / 2, FRAME_SIZE.height);
	this->setTitleBar(titleBar);

	m_titleLabel = Label::createWithSystemFont(sLocaleMgr->getString("func_dlg_title_remove_ads"), DEFAULT_SYSTEM_FONT, 15);
	m_titleLabel->setPosition(Vec2(0, 2));
	m_titleLabel->setHorizontalAlignment(TextHAlignment::CENTER);
	m_titleLabel->setAnchorPoint(Point::ANCHOR_MIDDLE);
	m_titleLabel->setTextColor(Color4B(244, 240, 237, 255));
	Utils::enableBoldForLabel(m_titleLabel);

	m_titleBg = Scale9Sprite::createWithSpriteFrameName("form_title_bg.png");
	m_titleBg->setAnchorPoint(Point::ANCHOR_MIDDLE);

	titleBar->addChild(m_titleBg);
	titleBar->addChild(m_titleLabel);

	m_messageLabel = Label::createWithSystemFont(sLocaleMgr->getString("func_dlg_msg_remove_ads"), DEFAULT_SYSTEM_FONT, 15);
	m_messageLabel->setTextColor(Color4B(150, 129, 56, 255));
	Utils::enableBoldForLabel(m_messageLabel);
	m_messageLabel->setPosition(FRAME_SIZE.width / 2, FRAME_SIZE.height / 2 + 18);
	m_messageLabel->setAnchorPoint(Point::ANCHOR_MIDDLE);
	m_messageLabel->setWidth(FRAME_SIZE.width - CONTENT_MARGIN_LEFTRIGHT * 2);
	this->setContent(m_messageLabel);

	Node* buttonPanel = Node::create();
	buttonPanel->setPosition(FRAME_SIZE.width / 2, BUTTON_PANEL_MARGIN_BOTTOM);
	buttonPanel->setIgnoreAnchorPointForPosition(false);
	buttonPanel->setAnchorPoint(Point::ANCHOR_MIDDLE);
	buttonPanel->setContentSize(Size(FRAME_SIZE.width - BUTTON_PANEL_MARGIN_LEFTRIGHT * 2, BUTTON_PANEL_HEIGHT));
	this->setButtonPanel(buttonPanel);

	Scale9Sprite* background = Scale9Sprite::createWithSpriteFrameName("form_bg.png");
	this->setBackground(background);

	this->initButtons();

	return true;
}

RemoveAdsDialog::RemoveAdsDialog() :
	m_titleBg(nullptr),
	m_titleLabel(nullptr),
	m_titleNode(nullptr),
	m_messageLabel(nullptr),
	m_buyBtn(nullptr),
	m_cancelBtn(nullptr)
{

}

RemoveAdsDialog::~RemoveAdsDialog()
{
	sStore->removeListener(this);

	m_titleBg = nullptr;
	m_titleLabel = nullptr;
	m_titleNode = nullptr;
	m_messageLabel = nullptr;

	CC_SAFE_RELEASE_NULL(m_buyBtn);
	CC_SAFE_RELEASE_NULL(m_cancelBtn);

}

void RemoveAdsDialog::onEnter()
{
	ModalDialog::onEnter();

	Vector<ui::Widget*> buttons;
	buttons.pushBack(m_buyBtn);
	buttons.pushBack(m_cancelBtn);

	this->setButtons(buttons);
	this->layoutTitleBar();

	if (!sStore->isProductIdsValidated() && !sStore->isValidatingProductIds())
		this->validateProductIds();

	this->updateBuyButton();
}

void RemoveAdsDialog::initButtons()
{
	m_buyBtn = LoadingButton::create("$99.99",  BUY_LOADING_INDICATOR_SIZE, "btn_bg_positive.png");
	m_buyBtn->setTitleColor(Color4B(243, 244, 237, 255));
	m_buyBtn->setTitleFontSize(12);
	m_buyBtn->setContentSize(Size(BUTTON_WIDTH, BUTTON_PANEL_HEIGHT));
	m_buyBtn->setLoadingIndicatorColor(Color3B(247, 242, 205));
	m_buyBtn->setShowLoadingIndicator(false);
	m_buyBtn->setClickEventListener([this](Ref* sender) {
		if(!sStore->isValidatingProductIds())
		{
			if (m_buyCallback)
				m_buyCallback(this, sShopMgr->getRemoveAdsTemplate());
			this->dismiss();
			sSoundMgr->play(SOUND_BUTTON);
		}
	});
	CC_SAFE_RETAIN(m_buyBtn);

	m_cancelBtn = this->createNormalButton(sLocaleMgr->getString("message_dialog_negative_button"), Color3B(234, 228, 228), "btn_bg_negative.png");
	m_cancelBtn->addClickEventListener([this](Ref* sender) {
		if (m_cancelBtnCallback)
			m_cancelBtnCallback(this);
		this->dismiss();
		sSoundMgr->play(SOUND_BUTTON);
	});
	CC_SAFE_RETAIN(m_cancelBtn);
}

void RemoveAdsDialog::layoutTitleBar()
{
	m_titleBg->setContentSize(Size(MAX(TITLE_BAR_MIN_WIDTH, m_titleLabel->getBoundingBox().size.width + TITLE_MARGIN_LEFTRIGHT * 2), TITLE_BAR_HEIGHT));
}

void RemoveAdsDialog::updateBuyButton()
{
	if (sStore->isValidatingProductIds())
	{
		m_buyBtn->setShowLoadingIndicator(true);
	}
	else
	{
		m_buyBtn->setShowLoadingIndicator(false);
		auto tmpl = sShopMgr->getRemoveAdsTemplate();
		ProductInfo const* product = sStore->getProductById(tmpl->productId);
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

ui::Button* RemoveAdsDialog::createNormalButton(std::string const& title, Color3B const& titleColor, std::string const& normalImage)
{
	Button* button = Button::create(normalImage, "", "", Widget::TextureResType::PLIST);
	button->setTitleColor(titleColor);
	button->setScale9Enabled(true);
	button->setTitleText(title);
	button->setTitleFontSize(12);
	Label* label = button->getTitleRenderer();
	Utils::enableBoldForLabel(label);
	button->setContentSize(Size(BUTTON_WIDTH, BUTTON_PANEL_HEIGHT));

	return button;
}

void RemoveAdsDialog::validateProductIds()
{
	std::vector<std::string> productIds = sShopMgr->getProductIdList();
	sStore->validateProductIds(productIds);
}

void RemoveAdsDialog::onProductsRequestFinished()
{
    this->updateBuyButton();
}

void RemoveAdsDialog::onProductsRequestFailed(StoreError const& error)
{
	this->updateBuyButton();
}

NS_END

