#include "OptionWicket.h"

#include "game/LocaleMgr.h"
#include "game/UserPreferences.h"
#include "game/gamecenter/GameCenter.h"
#include "game/ShopMgr.h"
#include "scene/SoundMgr.h"
#include "scene/gui/ModalDialog.h"
#include "scene/Utils.h"
#include "scene/review/StoreReview.h"

using namespace cocos2d::ui;

NS_BEGIN

#define FRAME_SIZE		Size(280, 198)

#define FRAMENAME_SWITCH_BUTTON_ON			"switchbutton_on.png"
#define FRAMENAME_SWITCH_BUTTON_OFF			"switchbutton_off.png"
#define FRAMENAME_DIVIDER					"divider.png"
#define FRAMENAME_BTN_RESTORE_BUY			"btn_bg_positive.png"
#define FRAMENAME_BTN_RATE					"option_btn_rate.png"

#define RESTORE_BUY_TITLE_MARGIN_TOPBOTTOM			9
#define RESTORE_BUY_TITLE_MARGIN_LEFTRIGHT			17

#define OPTION_MARGIN_TOP				9
#define OPTION_MARGIN_LEFTRIGHT			16

#define BUTTON_SIZE		Size(110, 30)


OptionWicket* OptionWicket::create(Node* owner)
{
	OptionWicket *ret = new (std::nothrow) OptionWicket();
	if (ret && ret->init(owner))
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

bool OptionWicket::init(Node* owner)
{
	if (!Wicket::init(owner, FRAME_SIZE, sLocaleMgr->getString("func_option_wicket_title"), true, false))
		return false;

	m_main = Node::create();
	this->setContent(m_main);

	Size frameSize = m_main->getBoundingBox().size;
	float nextY = frameSize.height - OPTION_MARGIN_TOP;

	m_soundEffectSwitch = SwitchButton::create(FRAMENAME_SWITCH_BUTTON_OFF, FRAMENAME_SWITCH_BUTTON_ON);
	m_soundEffectSwitch->setAnchorPoint(Point::ANCHOR_TOP_RIGHT);
	m_soundEffectSwitch->setPosition(Vec2(frameSize.width - OPTION_MARGIN_LEFTRIGHT, nextY));
	m_soundEffectSwitch->addClickEventListener(CC_CALLBACK_1(OptionWicket::switchSoundEffectCallback, this));
	m_soundEffectSwitch->setOn(sUserPreferences->isSoundEffectEnabled());
	m_main->addChild(m_soundEffectSwitch);

	m_soundEffectLabel = Label::createWithSystemFont(sLocaleMgr->getString("func_option_label_soundfx"), DEFAULT_SYSTEM_FONT, 13);
	m_soundEffectLabel->setTextColor(Color4B(150, 129, 56, 255));
	Utils::enableBoldForLabel(m_soundEffectLabel);
	m_soundEffectLabel->setAnchorPoint(Point::ANCHOR_MIDDLE_LEFT);
	m_soundEffectLabel->setPosition(Vec2(OPTION_MARGIN_LEFTRIGHT, m_soundEffectSwitch->getBoundingBox().getMidY()));
	m_main->addChild(m_soundEffectLabel);
	
	nextY = m_soundEffectSwitch->getBoundingBox().getMinY() - 3;

	m_musicSwitch = SwitchButton::create(FRAMENAME_SWITCH_BUTTON_OFF, FRAMENAME_SWITCH_BUTTON_ON);
	m_musicSwitch->setAnchorPoint(Point::ANCHOR_TOP_RIGHT);
	m_musicSwitch->setPosition(Vec2(frameSize.width - OPTION_MARGIN_LEFTRIGHT, nextY));
	m_musicSwitch->addClickEventListener(CC_CALLBACK_1(OptionWicket::switchMusicCallback, this));
	m_musicSwitch->setOn(sUserPreferences->isMusicEnabled());
	m_main->addChild(m_musicSwitch);

	m_musicLabel = Label::createWithSystemFont(sLocaleMgr->getString("func_option_label_music"), DEFAULT_SYSTEM_FONT, 13);
	m_musicLabel->setTextColor(Color4B(150, 129, 56, 255));
	Utils::enableBoldForLabel(m_musicLabel);
	m_musicLabel->setAnchorPoint(Point::ANCHOR_MIDDLE_LEFT);
	m_musicLabel->setPosition(Vec2(OPTION_MARGIN_LEFTRIGHT, m_musicSwitch->getBoundingBox().getMidY()));
	m_main->addChild(m_musicLabel);

	nextY = m_musicSwitch->getBoundingBox().getMinY() - 9;

	Sprite* dividerSp = Sprite::createWithSpriteFrameName(FRAMENAME_DIVIDER);
	dividerSp->setContentSize(Size(frameSize.width - OPTION_MARGIN_LEFTRIGHT * 2, 1));
	dividerSp->setPosition(Vec2(frameSize.width / 2, nextY));
	m_main->addChild(dividerSp);

	nextY = 31;

	m_rateBtn = Button::create(FRAMENAME_BTN_RATE, "", "", Widget::TextureResType::PLIST);
	m_rateBtn->setScale9Enabled(true);
	m_rateBtn->setContentSize(BUTTON_SIZE);
#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS
	m_rateBtn->setAnchorPoint(Point::ANCHOR_MIDDLE_LEFT);
	m_rateBtn->setPosition(Vec2(OPTION_MARGIN_LEFTRIGHT - 4, nextY));
#else
	m_rateBtn->setAnchorPoint(Point::ANCHOR_MIDDLE);
	m_rateBtn->setPosition(Vec2(frameSize.width / 2, nextY));
#endif
	m_rateBtn->addClickEventListener(CC_CALLBACK_1(OptionWicket::buttonRateClickCallback, this));
	Label* m_rateTitleLabel = Label::createWithSystemFont(sLocaleMgr->getString("func_option_btn_rate"), DEFAULT_SYSTEM_FONT, 11);
	m_rateBtn->setTitleLabel(m_rateTitleLabel);
	m_rateTitleLabel->setPosition(Vec2(BUTTON_SIZE.width / 2 + 3, BUTTON_SIZE.height / 2));
	m_rateTitleLabel->setTextColor(Color4B(242, 242, 235, 255));
	Utils::enableBoldForLabel(m_rateTitleLabel);

	m_main->addChild(m_rateBtn);

#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS
	sStore->addListener(this);

	m_restoreBuyBtn = Button::create(FRAMENAME_BTN_RESTORE_BUY, "", "", Widget::TextureResType::PLIST);
	m_restoreBuyBtn->setScale9Enabled(true);
	m_restoreBuyBtn->setAnchorPoint(Point::ANCHOR_MIDDLE_RIGHT);
	m_restoreBuyBtn->setPosition(Vec2(frameSize.width - OPTION_MARGIN_LEFTRIGHT + 1, nextY));
	m_restoreBuyBtn->addClickEventListener(CC_CALLBACK_1(OptionWicket::buttonRestoreBuyClickCallback, this));
	Label* restoreBuyTitleLabel = Label::createWithSystemFont(sLocaleMgr->getString("func_option_btn_restorebuy"), DEFAULT_SYSTEM_FONT, 11);
	m_restoreBuyBtn->setTitleLabel(restoreBuyTitleLabel);
	restoreBuyTitleLabel->setTextColor(Color4B(242, 242, 235, 255));
	Utils::enableBoldForLabel(restoreBuyTitleLabel);
	m_restoreBuyBtn->setContentSize(BUTTON_SIZE);
	m_main->addChild(m_restoreBuyBtn);

#endif

	std::stringstream version;
	version << "Version ";
	version << std::hex << ((APP_VERSION >> 16) & 0x000000FF) << "." << ((APP_VERSION >> 8) & 0x000000FF) << "." << (APP_VERSION & 0x000000FF);
	version << " (";
	version << std::dec << "Build " << BUILD_NUMBER;
	version << ")";
	m_verLabel = Label::createWithSystemFont(version.str(), DEFAULT_SYSTEM_FONT, 10);
	m_verLabel->setTextColor(Color4B(150, 129, 56, 255));
	m_verLabel->setAnchorPoint(Point::ANCHOR_MIDDLE);
	m_verLabel->setHorizontalAlignment(TextHAlignment::CENTER);
	Utils::enableBoldForLabel(m_verLabel);
	m_verLabel->setPosition(Vec2(frameSize.width / 2, -9));
	m_main->addChild(m_verLabel);

	return true;
}

OptionWicket::OptionWicket() :
	m_main(nullptr),
	m_musicSwitch(nullptr),
	m_soundEffectSwitch(nullptr),
	m_musicLabel(nullptr),
	m_soundEffectLabel(nullptr),
	m_rateBtn(nullptr),
	m_restoreBuyBtn(nullptr),
	m_verLabel(nullptr),
	m_loadingView(nullptr),
	m_restoredTransactions(0)
{
}

OptionWicket::~OptionWicket()
{
#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS
    sStore->removeListener(this);
#endif
    
	m_main = nullptr;
	m_musicSwitch = nullptr;
	m_soundEffectSwitch = nullptr;
	m_musicLabel = nullptr;
	m_soundEffectLabel = nullptr;
	m_rateBtn = nullptr;
	m_restoreBuyBtn = nullptr;
	m_verLabel = nullptr;
	m_loadingView = nullptr;
}

void OptionWicket::onEnter()
{
	Wicket::onEnter();

}

void OptionWicket::update(float delta)
{
	Wicket::update(delta);
}

void OptionWicket::onClosed()
{
	ValueMapIntKey data;
	this->setResult(Wicket::RESULT_CODE_OK, data);
}

void OptionWicket::switchSoundEffectCallback(Ref* sender)
{
	SwitchButton* switchBtn = (SwitchButton*)sender;
	bool isOn = switchBtn->isOn();
	if (isOn)
	{
		sSoundMgr->setEnableSoundEffect(true);
	}
	else
	{
		sSoundMgr->setEnableSoundEffect(false);
	}
	sUserPreferences->setSoundEffectEnabled(sSoundMgr->isSoundEffectEnabled());
}

void OptionWicket::switchMusicCallback(Ref* sender)
{
	SwitchButton* switchBtn = (SwitchButton*)sender;
	bool isOn = switchBtn->isOn();
	if (isOn)
	{
		sSoundMgr->setEnableBackgroundMusic(true);
		sSoundMgr->play(SOUND_FUNC_MUSIC, true);
	}
	else
	{
		sSoundMgr->setEnableBackgroundMusic(false);
	}
	sUserPreferences->setMusicEnabled(sSoundMgr->isBackgroundMusicEnabled());
}

void OptionWicket::buttonRateClickCallback(Ref* sender)
{
	sStoreReview->requestReviewManually();
	sSoundMgr->play(SOUND_BUTTON);
}


void OptionWicket::buttonRestoreBuyClickCallback(Ref* sender)
{
	this->restoreCompletedTransactions();
    sSoundMgr->play(SOUND_BUTTON);
}

void OptionWicket::restoreCompletedTransactions()
{
	bool ret = sStore->restoreCompletedTransactions();
	if (ret)
	{
		m_restoredTransactions = 0;
		this->showLoadingView();
	}
}

void OptionWicket::showLoadingView()
{
	if (!m_loadingView)
	{
		m_loadingView = LoadingView::create(this);
		m_loadingView->show();
	}
}

void OptionWicket::dismissLoadingView()
{
	if (m_loadingView)
	{
		m_loadingView->dismiss();
		m_loadingView = nullptr;
	}
}

void OptionWicket::onPaymentTransactionUpdated(PaymentTransaction const& transaction)
{
	if(transaction.state == PAYMENT_STATE_RESTORED)
    {
        IAPItem const* item = sShopMgr->getIAPItem(transaction.productId);
		// The consumable cannot be restored
        if(!item->isNull() && item->productType != PRODUCT_TYPE_CONSUMABLE)
            m_restoredTransactions++;
    }
}

void OptionWicket::onRestoreCompletedTransactionsFinished()
{
    this->dismissLoadingView();
    if (m_restoredTransactions > 0)
    {
        MessageDialog* dialog = MessageDialog::create();
		dialog->setTitle(sLocaleMgr->getString("store_dlg_title_restore_success"));
        dialog->setMessage(sLocaleMgr->getString("store_dlg_msg_restore_success"));
        dialog->addOkButton(nullptr);
        dialog->show();
    }
    else
    {
        MessageDialog* dialog = MessageDialog::create();
		dialog->setTitle(sLocaleMgr->getString("store_dlg_title_no_restorable"));
        dialog->setMessage(sLocaleMgr->getString("store_dlg_msg_no_restorable"));
        dialog->addOkButton(nullptr);
        dialog->show();
    }
    m_restoredTransactions = 0;
}

void OptionWicket::onRestoreCompletedTransactionsFailed(StoreError const& error)
{
    this->dismissLoadingView();
    if (error.code != STORE_ERROR_CANCELLED)
    {
        MessageDialog* dialog = MessageDialog::create();
		dialog->setTitle(sLocaleMgr->getString("store_dlg_title_restore_fail"));
        dialog->setMessage(sLocaleMgr->getString("store_dlg_msg_restore_fail"));
        dialog->addOkButton(nullptr);
        dialog->show();
    }
}

NS_END

