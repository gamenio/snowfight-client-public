#include "ModalDialog.h"

#include "../SoundMgr.h"
#include "scene/Utils.h"

using namespace cocos2d::ui;

NS_BEGIN


// ModalDialog

#define ACTION_TAG_APPEAR					1
#define ACTION_TAG_DISAPPEAR				2

// MessageDialog

#define FRAME_SIZE							Size(280, 165)

#define BUTTON_MIN_WIDTH                    100
#define BUTTON_TITLE_MARGIN_LEFTRIGHT       15
#define BUTTON_PANEL_HEIGHT					36
#define BUTTON_PANEL_MARGIN_LEFTRIGHT       9
#define BUTTON_PANEL_MARGIN_BOTTOM			32

#define TITLE_BAR_MIN_WIDTH					163
#define TITLE_BAR_MAX_WIDTH					224
#define TITLE_BAR_HEIGHT					42
#define TITLE_MARGIN_LEFTRIGHT				27

#define CONTENT_MARGIN_LEFTRIGHT			30

#define LOCAL_ZORDER_BACKGROUND				1
#define LOCAL_ZORDER_CONTENT				2
#define LOCAL_ZORDER_TITLE					3
#define LOCAL_ZORDER_BUTTONPANEL			4

ModalDialog* ModalDialog::create(cocos2d::Size const& frameSize)
{
	auto ret = new (std::nothrow) ModalDialog();
	if (ret && ret->init(frameSize))
	{
		ret->autorelease();
	}
	else
	{
		CC_SAFE_DELETE(ret);
	}
	return ret;
}

bool ModalDialog::init(Size const& frameSize)
{
	if (!LayerColor::initWithColor(Color4B(0, 0, 0, 125)))
		return false;

	auto touchListener = EventListenerTouchOneByOne::create();
	touchListener->setSwallowTouches(true);

	touchListener->onTouchBegan = CC_CALLBACK_2(ModalDialog::onTouchBegan, this);
	touchListener->onTouchMoved = CC_CALLBACK_2(ModalDialog::onTouchMoved, this);
	touchListener->onTouchEnded = CC_CALLBACK_2(ModalDialog::onTouchEnded, this);
	touchListener->onTouchCancelled = CC_CALLBACK_2(ModalDialog::onTouchCancelled, this);
	this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(touchListener, this);

	auto keyboardListener = EventListenerKeyboard::create();
	keyboardListener->onKeyReleased = CC_CALLBACK_2(ModalDialog::onKeyReleased, this);
	this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(keyboardListener, this);

	auto visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();

	m_mainPanel = Node::create();
	m_mainPanel->setAnchorPoint(Point::ANCHOR_MIDDLE);
	m_mainPanel->setPosition(origin.x + visibleSize.width / 2, origin.y + visibleSize.height / 2);
	m_mainPanel->setContentSize(frameSize);
	this->addChild(m_mainPanel);

	return true;
}

ModalDialog::ModalDialog() :
	m_owner(nullptr),
	m_mainPanel(nullptr),
	m_titleBar(nullptr),
	m_content(nullptr),
	m_background(nullptr),
	m_buttonPanel(nullptr)
{
}

ModalDialog::~ModalDialog()
{
	this->stopAllActions();

	m_owner = nullptr;
	m_mainPanel = nullptr;
	m_titleBar = nullptr;
	m_content = nullptr;
	m_background = nullptr;
	m_buttonPanel = nullptr;
}

void ModalDialog::setTitleBar(Node* titleBar)
{
	if (m_titleBar)
	{
		m_titleBar->removeFromParent();
		m_titleBar = nullptr;
	}
	m_titleBar = titleBar;
	m_mainPanel->addChild(m_titleBar, LOCAL_ZORDER_TITLE);
}

void ModalDialog::setContent(Node* content)
{
	if (m_content)
	{
		m_content->removeFromParent();
		m_content = nullptr;
	}
	m_content = content;
	m_mainPanel->addChild(m_content, LOCAL_ZORDER_CONTENT);
}

void ModalDialog::setButtonPanel(Node* buttonPanel)
{
	if (m_buttonPanel)
	{
		m_buttonPanel->removeFromParent();
		m_buttonPanel = nullptr;
	}
	m_buttonPanel = buttonPanel;
	m_mainPanel->addChild(m_buttonPanel, LOCAL_ZORDER_BUTTONPANEL);
}

void ModalDialog::setBackground(Sprite* background)
{
	if (m_background)
	{
		m_background->removeFromParent();
		m_background = nullptr;
	}
	m_background = background;
	m_background->setContentSize(m_mainPanel->getContentSize());
	m_background->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
	m_mainPanel->addChild(m_background, LOCAL_ZORDER_BACKGROUND);
}

void ModalDialog::setButtons(Vector<ui::Widget*> const& m_buttons)
{
	if (!m_buttonPanel)
		return;

	m_buttonPanel->removeAllChildren();

	Size panelSize = m_buttonPanel->getContentSize();
	float gridMaxWidth = panelSize.width / m_buttons.size();
	float maxHeight = panelSize.height;
	float nextX = 0;
	for (auto it = m_buttons.begin(); it != m_buttons.end(); ++it)
	{
		auto button = *it;
		button->setAnchorPoint(Point::ANCHOR_MIDDLE);
		button->setPosition(Vec2(nextX + gridMaxWidth / 2, maxHeight / 2));
        button->setContentSize(Size(MIN(button->getContentSize().width, gridMaxWidth), button->getContentSize().height));
		m_buttonPanel->addChild(button);

		nextX = nextX + gridMaxWidth;
	}
}


bool ModalDialog::onTouchBegan(Touch* touch, Event* event)
{
	return true;
}

void ModalDialog::onTouchEnded(Touch* touch, Event* event)
{
}

void ModalDialog::onTouchCancelled(Touch* touch, Event* event)
{
}

void ModalDialog::onTouchMoved(Touch* touch, Event* event)
{
}

void ModalDialog::onKeyReleased(EventKeyboard::KeyCode keyCode, Event *event)
{
	switch (keyCode)
	{
		case cocos2d::EventKeyboard::KeyCode::KEY_BACK:
			event->stopPropagation();
			break;
		default:
			break;
	}
}


void ModalDialog::show()
{
	auto scene = Director::getInstance()->getRunningScene();
    TransitionScene* transitionScene = dynamic_cast<TransitionScene* >(scene);
    if(transitionScene)
        scene = transitionScene->getInScene();
    
	NS_ASSERT(scene != nullptr);
    if(scene->isRunning())
        this->showIn(scene);
    else
    {
        this->retain();
        scene->setonEnterTransitionDidFinishCallback([scene, this](){
            this->showIn(scene);
            this->release();
        });
    }
}


void ModalDialog::dismiss()
{
	m_mainPanel->stopActionByTag(ACTION_TAG_APPEAR);

	this->onWillDisappear();

	ScaleBy* scale = ScaleBy::create(0.1f, 0.0f);
	CallFunc* callFunc = CallFunc::create([this]() {
		this->onDisappeared();
		this->removeFromParent();
	});
	Sequence* seq = Sequence::create(scale, callFunc, nullptr);
	seq->setTag(ACTION_TAG_DISAPPEAR);
	m_mainPanel->runAction(seq);
}

ModalDialog* ModalDialog::getDialogByName(std::string const& name)
{
	auto scene = Director::getInstance()->getRunningScene();
	NS_ASSERT(scene != nullptr);
	return dynamic_cast<ModalDialog*>(scene->getChildByName(name));
}

void ModalDialog::showIn(Scene* scene)
{
    NS_ASSERT_LOG(scene->isRunning(), "Dialog cannot be shown in scene that are not running.");
    scene->addChild(this);

	this->onWillAppear();
    
    m_mainPanel->setScale(0.1f);
    ScaleTo* scale = ScaleTo::create(0.25f, 1.0f);
    EaseBackOut* ease = EaseBackOut::create(scale);
	CallFunc* callFunc = CallFunc::create([this]() {
		this->onAppeared();
	});
	Sequence* seq = Sequence::create(ease, callFunc, nullptr);
	seq->setTag(ACTION_TAG_APPEAR);
    m_mainPanel->runAction(seq);
}

MessageDialog* MessageDialog::create()
{
	auto ret = new (std::nothrow) MessageDialog();
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

bool MessageDialog::init()
{
	if (!ModalDialog::init(FRAME_SIZE))
		return false;

	Node* titleBar = Node::create();
	titleBar->setPosition(FRAME_SIZE.width / 2, FRAME_SIZE.height);
	this->setTitleBar(titleBar);

	m_titleLabel = Label::createWithSystemFont(sLocaleMgr->getString("message_dialog_title"), DEFAULT_SYSTEM_FONT, 15);
	m_titleLabel->setPosition(Vec2(0, 2));
	m_titleLabel->setHorizontalAlignment(TextHAlignment::CENTER);
	m_titleLabel->setAnchorPoint(Point::ANCHOR_MIDDLE);
	m_titleLabel->setTextColor(Color4B(244, 240, 237, 255));
	Utils::enableBoldForLabel(m_titleLabel);

	m_titleBg = Scale9Sprite::createWithSpriteFrameName("form_title_bg.png");
	m_titleBg->setAnchorPoint(Point::ANCHOR_MIDDLE);

	titleBar->addChild(m_titleBg);
	titleBar->addChild(m_titleLabel);

	m_messageLabel = Label::createWithSystemFont("MessageLabel", DEFAULT_SYSTEM_FONT, 15);
	m_messageLabel->setTextColor(Color4B(150, 129, 56, 255));
	Utils::enableBoldForLabel(m_messageLabel);
	m_messageLabel->setPosition(FRAME_SIZE.width / 2, FRAME_SIZE.height / 2 + 18);
	m_messageLabel->setAnchorPoint(Point::ANCHOR_MIDDLE);
	this->setContent(m_messageLabel);

	Node* buttonPanel = Node::create();
	buttonPanel->setPosition(FRAME_SIZE.width / 2, BUTTON_PANEL_MARGIN_BOTTOM);
	buttonPanel->setIgnoreAnchorPointForPosition(false);
	buttonPanel->setAnchorPoint(Point::ANCHOR_MIDDLE);
	buttonPanel->setContentSize(Size(FRAME_SIZE.width - BUTTON_PANEL_MARGIN_LEFTRIGHT * 2, BUTTON_PANEL_HEIGHT));
	this->setButtonPanel(buttonPanel);

	Scale9Sprite* background = Scale9Sprite::createWithSpriteFrameName("form_bg.png");
	this->setBackground(background);

	return true;
}

MessageDialog::MessageDialog() :
	m_titleBg(nullptr),
	m_titleLabel(nullptr),
	m_messageLabel(nullptr),
	m_menu(nullptr)
{
	for (int32 i = 0; i < BUTTON_TYPES; ++i)
		m_buttons[i] = nullptr;
}

MessageDialog::~MessageDialog()
{
	m_titleBg = nullptr;
	m_titleLabel = nullptr;
	m_messageLabel = nullptr;
	m_menu = nullptr;

	for (int32 i = 0; i < BUTTON_TYPES; ++i)
		if (m_buttons[i])
			CC_SAFE_RELEASE_NULL(m_buttons[i]);

}

void MessageDialog::setTitle(std::string const& title)
{
	m_titleLabel->setString(title);
	float maxWidth = TITLE_BAR_MAX_WIDTH - TITLE_MARGIN_LEFTRIGHT * 2;
	if (m_titleLabel->getContentSize().width > maxWidth)
	{
		float scale = maxWidth / m_titleLabel->getContentSize().width;
		m_titleLabel->setScale(scale);
	}
}

void MessageDialog::setMessage(std::string const& message)
{
	m_messageLabel->setString(message);
	float maxMsgWidth = FRAME_SIZE.width - CONTENT_MARGIN_LEFTRIGHT * 2;
	if(m_messageLabel->getContentSize().width > maxMsgWidth)
		m_messageLabel->setWidth(maxMsgWidth);
    
}


void MessageDialog::addTickButton(DialogCallback const& callback, ButtonType type)
{
	Button* tickBtn = nullptr;
	switch (type)
	{
	case BUTTON_NEGATIVE:
		tickBtn = Button::create("btn_tick_red.png", "", "", Widget::TextureResType::PLIST);
		break;
	default: // BUTTON_POSITIVE | BUTTON_NEUTRAL
		tickBtn = Button::create("btn_tick_green.png", "", "", Widget::TextureResType::PLIST);
		break;
	}

	tickBtn->addClickEventListener([callback, this](Ref* sender) {
		if (callback)
			callback(this);
		this->dismiss();
        sSoundMgr->play(SOUND_BUTTON);
	});

	if (m_buttons[type])
		CC_SAFE_RELEASE_NULL(m_buttons[type]);
	m_buttons[type] = tickBtn;
	CC_SAFE_RETAIN(tickBtn);
}

void MessageDialog::addCrossButton(DialogCallback const& callback, ButtonType type)
{
	Button* crossBtn = nullptr;
	switch (type)
	{
	case BUTTON_NEGATIVE:
		crossBtn = Button::create("btn_cross_red.png", "", "", Widget::TextureResType::PLIST);
		break;
	default: // BUTTON_POSITIVE | BUTTON_NEUTRAL
		crossBtn = Button::create("btn_cross_green.png", "", "", Widget::TextureResType::PLIST);
		break;
	}

	crossBtn->addClickEventListener([callback, this](Ref* sender) {
		if (callback)
			callback(this);
		this->dismiss();
        sSoundMgr->play(SOUND_BUTTON);
	});

	if (m_buttons[type])
		CC_SAFE_RELEASE_NULL(m_buttons[type]);
	m_buttons[type] = crossBtn;
	CC_SAFE_RETAIN(crossBtn);
}

void MessageDialog::onEnter()
{
	ModalDialog::onEnter();

	Vector<ui::Widget*> buttons;
	for (int32 i = 0; i < BUTTON_TYPES; ++i)
		if (m_buttons[i])
			buttons.pushBack(m_buttons[i]);

	if (buttons.empty())
	{
		this->setButton(BUTTON_POSITIVE, sLocaleMgr->getString("message_dialog_positive_button"), nullptr);
		buttons.pushBack(m_buttons[BUTTON_POSITIVE]);
	}

	this->setButtons(buttons);
	this->layoutTitleBar();
}

void MessageDialog::setButton(ButtonType type, std::string const& title, DialogCallback const& callback)
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
    float wrapContentWidth = label->getContentSize().width + BUTTON_TITLE_MARGIN_LEFTRIGHT * 2;
	button->setContentSize(Size(MAX(wrapContentWidth, BUTTON_MIN_WIDTH), BUTTON_PANEL_HEIGHT));
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

void MessageDialog::layoutTitleBar()
{
	m_titleBg->setContentSize(Size(MAX(TITLE_BAR_MIN_WIDTH, m_titleLabel->getBoundingBox().size.width + TITLE_MARGIN_LEFTRIGHT * 2), TITLE_BAR_HEIGHT));
}


NS_END
