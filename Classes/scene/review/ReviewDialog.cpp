#include "ReviewDialog.h"

#include "../SoundMgr.h"
#include "scene/Utils.h"

using namespace cocos2d::ui;

NS_BEGIN

#define FRAME_SIZE							Size(320, 180)

#define BUTTON_WRAP_CONTENT_MIN_WIDTH       100
#define BUTTON_MARGIN_LEFTRIGHT				3
#define BUTTON_TITLE_MARGIN_LEFTRIGHT       15
#define BUTTON_PANEL_HEIGHT					36
#define BUTTON_PANEL_MARGIN_LEFTRIGHT       13
#define BUTTON_PANEL_MARGIN_BOTTOM			32


#define CONTENT_MARGIN_LEFTRIGHT			30

ReviewDialog* ReviewDialog::create()
{
	auto ret = new (std::nothrow) ReviewDialog();
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

bool ReviewDialog::init()
{
	if (!ModalDialog::init(FRAME_SIZE))
		return false;

	SpriteFrame* titleBgFrame = SpriteFrameCache::getInstance()->getSpriteFrameByName("review_dlg_title_bg.png");
	Node* titleBar = Node::create();
	titleBar->setAnchorPoint(Point::ANCHOR_MIDDLE_BOTTOM);
	titleBar->setPosition(FRAME_SIZE.width / 2, FRAME_SIZE.height - 34);
	titleBar->setContentSize(titleBgFrame->getOriginalSize());
	this->setTitleBar(titleBar);

	m_titleBg = Scale9Sprite::createWithSpriteFrame(titleBgFrame);
	m_titleBg->setPosition(titleBar->getContentSize().width / 2, titleBar->getContentSize().height / 2);
	m_titleBg->setAnchorPoint(Point::ANCHOR_MIDDLE);
	titleBar->addChild(m_titleBg);

	m_titleLabel = Label::createWithSystemFont(sLocaleMgr->getString("message_dialog_title"), DEFAULT_SYSTEM_FONT, 15);
	m_titleLabel->setPosition(titleBar->getContentSize().width / 2, 43);
	m_titleLabel->setAnchorPoint(Point::ANCHOR_MIDDLE);
	m_titleLabel->setTextColor(Color4B(237, 232, 239, 255));
	Utils::enableBoldForLabel(m_titleLabel);
	titleBar->addChild(m_titleLabel);

	m_messageLabel = Label::createWithSystemFont("MessageLabel", DEFAULT_SYSTEM_FONT, 15);
	m_messageLabel->setTextColor(Color4B(150, 129, 56, 255));
	Utils::enableBoldForLabel(m_messageLabel);
	m_messageLabel->setPosition(FRAME_SIZE.width / 2, FRAME_SIZE.height / 2 + 18);
	m_messageLabel->setAnchorPoint(Point::ANCHOR_MIDDLE);
	this->setContent(m_messageLabel);

	Node* buttonPanel = Node::create();
	buttonPanel->setPosition(FRAME_SIZE.width / 2, BUTTON_PANEL_MARGIN_BOTTOM);
	buttonPanel->setAnchorPoint(Point::ANCHOR_MIDDLE);
	buttonPanel->setContentSize(Size(FRAME_SIZE.width - BUTTON_PANEL_MARGIN_LEFTRIGHT * 2, BUTTON_PANEL_HEIGHT));
	this->setButtonPanel(buttonPanel);

	Scale9Sprite* background = Scale9Sprite::createWithSpriteFrameName("form_bg.png");
	this->setBackground(background);

	return true;
}

ReviewDialog::ReviewDialog() :
	m_titleBg(nullptr),
	m_titleLabel(nullptr),
	m_messageLabel(nullptr),
	m_menu(nullptr)
{
	for (int32 i = 0; i < BUTTON_TYPES; ++i)
		m_buttons[i] = nullptr;
}

ReviewDialog::~ReviewDialog()
{
	m_titleBg = nullptr;
	m_titleLabel = nullptr;
	m_messageLabel = nullptr;
	m_menu = nullptr;

	for (int32 i = 0; i < BUTTON_TYPES; ++i)
		if (m_buttons[i])
			CC_SAFE_RELEASE_NULL(m_buttons[i]);

}

void ReviewDialog::setTitle(std::string const& title)
{
	m_titleLabel->setString(title);
}

void ReviewDialog::setMessage(std::string const& message)
{
	m_messageLabel->setString(message);
	float maxMsgWidth = FRAME_SIZE.width - CONTENT_MARGIN_LEFTRIGHT * 2;
	if(m_messageLabel->getContentSize().width > maxMsgWidth)
		m_messageLabel->setWidth(maxMsgWidth);
    
}

void ReviewDialog::onEnter()
{
	ModalDialog::onEnter();

	float maxTitleWidth = 0.f;
	Vector<ui::Widget*> buttons;
	for (int32 i = 0; i < BUTTON_TYPES; ++i)
	{
		Button* button = m_buttons[i];
		if (button)
		{
			maxTitleWidth = MAX(maxTitleWidth, button->getTitleRenderer()->getContentSize().width);
			buttons.pushBack(button);
		}
	}

	float gridWidth = m_buttonPanel->getContentSize().width / buttons.size();
	float fillWidth = gridWidth - BUTTON_MARGIN_LEFTRIGHT * 2;
	float wrapContentWidth = MAX(BUTTON_WRAP_CONTENT_MIN_WIDTH, maxTitleWidth + BUTTON_TITLE_MARGIN_LEFTRIGHT * 2);
	for (auto button : buttons)
	{
		button->setContentSize(Size(MIN(wrapContentWidth, fillWidth), BUTTON_PANEL_HEIGHT));
	}

	this->setButtons(buttons);
}

void ReviewDialog::setButton(ButtonType type, std::string const& title, DialogCallback const& callback)
{
	Button* button = nullptr;
	switch (type)
	{
	case BUTTON_POSITIVE:
		button = Button::create("btn_bg_positive.png", "", "", Widget::TextureResType::PLIST);
        button->setTitleColor(Color3B(243, 244, 237));
		break;
	case BUTTON_NEGATIVE:
		button = Button::create("btn_bg_negative.png", "", "", Widget::TextureResType::PLIST);
		button->setTitleColor(Color3B(234, 228, 228));
		break;
	default: // BUTTON_NEUTRAL
		button = Button::create("btn_bg_neutral.png", "", "", Widget::TextureResType::PLIST);
		button->setTitleColor(Color3B(235, 241, 242));
		break;
	}

	button->setScale9Enabled(true);
	button->setTitleText(title);
	button->setTitleFontSize(12);
	Label* label = button->getTitleRenderer();
	Utils::enableBoldForLabel(label);
	button->addClickEventListener([callback, this](Ref* sender) {
		if(callback)
			callback(this);
		this->dismiss();
        sSoundMgr->play(SOUND_BUTTON);
	});

	if (m_buttons[type])
		CC_SAFE_RELEASE_NULL(m_buttons[type]);

	m_buttons[type] = button;
	CC_SAFE_RETAIN(button);
}


NS_END
