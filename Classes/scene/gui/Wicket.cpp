#include "Wicket.h"

#include "../SoundMgr.h"
#include "scene/Utils.h"

using namespace cocos2d::ui;

NS_BEGIN

#define ACTION_TAG_APPEAR					1
#define ACTION_TAG_DISAPPEAR				2

#define TITLE_BAR_MIN_WIDTH					163
#define TITLE_BAR_HEIGHT						42
#define TITLE_MARGIN_LEFTRIGHT				27

#define CONTENT_MARGIN_TOP					21
#define CONTENT_MARGIN_LEFTRIGHT			13
#define CONTENT_MARGIN_BOTTOM				27

#define BUTTON_MIN_WIDTH					81
#define BUTTON_HEIGHT						33
#define BUTTON_TITLE_MARGIN_LEFTRIGHT       15
#define BUTTON_MARGIN_LEFTRIGHT				6
#define BUTTON_PANEL_HEIGHT					33
#define BUTTON_PANEL_MARGIN_LEFTRIGHT       13
#define BUTTON_PANEL_MARGIN_BOTTOM			32

#define LOCAL_ZORDER_CONTENT				1
#define LOCAL_ZORDER_TITLE					2
#define LOCAL_ZORDER_BUTTONPANEL			3

#define BACKGROUND_COLOR					Color4B(0, 0, 0, 0)

Wicket::Wicket() :
	m_owner(nullptr),
	m_firstLayout(true),
	m_touchListener(nullptr),
	m_mainPanel(nullptr),
	m_title(nullptr),
	m_content(nullptr),
	m_background(nullptr),
	m_buttonPanel(nullptr),
	m_buttonAlignment(BUTTON_ALIGNMENT_AVERAGE),
	m_closeBtn(nullptr),
	m_resultCode(RESULT_CODE_OK)
{
}

Wicket::~Wicket()
{
	this->getEventDispatcher()->removeEventListener(m_touchListener);
	CC_SAFE_RELEASE_NULL(m_touchListener);

	this->stopAllActions();

	m_owner = nullptr;
	m_mainPanel = nullptr;
	m_title = nullptr;
	m_content = nullptr;
	m_background = nullptr;
	m_buttonPanel = nullptr;
	m_closeBtn = nullptr;
}

bool Wicket::init(Node* owner, Size const& frameSize, std::string const& titleText, bool showCloseButton, bool showButtonPanel,  ButtonAlignment buttonAlignment)
{
	if (!LayerColor::initWithColor(BACKGROUND_COLOR))
		return false;

	Node* titleBar = Node::create();
	titleBar->setPosition(frameSize.width / 2, frameSize.height);

	Label* titleLabel = Label::createWithSystemFont(titleText, DEFAULT_SYSTEM_FONT, 15);
	titleLabel->setPosition(Vec2(0, 2));
	titleLabel->setHorizontalAlignment(TextHAlignment::CENTER);
	titleLabel->setAnchorPoint(Point::ANCHOR_MIDDLE);
	titleLabel->setTextColor(Color4B(244, 240, 237, 255));
	Utils::enableBoldForLabel(titleLabel);


	Scale9Sprite* titleBg = Scale9Sprite::createWithSpriteFrameName("form_title_bg.png");
	titleBg->setContentSize(Size(MAX(TITLE_BAR_MIN_WIDTH, titleLabel->getContentSize().width + TITLE_MARGIN_LEFTRIGHT * 2), TITLE_BAR_HEIGHT));
	titleBg->setAnchorPoint(Point::ANCHOR_MIDDLE);

	titleBar->addChild(titleBg);
	titleBar->addChild(titleLabel);

	Button* closeBtn = nullptr;
	if (showCloseButton)
	{
		closeBtn = Button::create("btn_wicket_close.png", "", "", Widget::TextureResType::PLIST);
		closeBtn->setAnchorPoint(Point::ANCHOR_MIDDLE);
		closeBtn->setPosition(Vec2(frameSize.width - 4.0f, frameSize.height - 3.0f));
	}

	Node* buttonPanel = nullptr;
	if (showButtonPanel)
	{
		buttonPanel = Node::create();
		buttonPanel->setIgnoreAnchorPointForPosition(false);
		buttonPanel->setPosition(frameSize.width / 2, BUTTON_PANEL_MARGIN_BOTTOM);
		buttonPanel->setContentSize(Size(frameSize.width - BUTTON_PANEL_MARGIN_LEFTRIGHT * 2, BUTTON_PANEL_HEIGHT));
		buttonPanel->setAnchorPoint(Point::ANCHOR_MIDDLE);
	}

	Scale9Sprite* background = Scale9Sprite::createWithSpriteFrameName("form_bg.png");
	this->initForm(owner, frameSize, titleBar, buttonPanel, buttonAlignment, background, closeBtn);

	return true;
}

bool Wicket::init(Node* owner, Size const& frameSize, Node* title, Node* buttonPanel, ButtonAlignment buttonAlignment, Sprite* background, Button* closeBtn)
{
	if (!LayerColor::initWithColor(BACKGROUND_COLOR))
		return false;

	this->initForm(owner, frameSize, title, buttonPanel, buttonAlignment, background, closeBtn);

	return true;
}

void Wicket::setButtons(Vector<Button*> const& m_buttons)
{
	if (!m_buttonPanel)
		return;

	m_buttonPanel->removeAllChildren();
	for (auto it = m_buttons.begin(); it != m_buttons.end(); ++it)
	{
		Button* button = *it;
		m_buttonPanel->addChild(button);
	}

	layoutIfNeeded();
}

void Wicket::addButton(ButtonType buttonType, std::string const& title, WicketButtonClickCallback const& callback)
{
	if (!m_buttonPanel)
		return;

	Button* button = nullptr;
	switch (buttonType)
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
	button->setTitleFontSize(14);
	button->setTitleText(title);
	Label* label = button->getTitleRenderer();
	Utils::enableBoldForLabel(label);
	float wrapContentWidth = label->getContentSize().width + BUTTON_TITLE_MARGIN_LEFTRIGHT * 2;
	button->setContentSize(Size(MAX(wrapContentWidth, BUTTON_MIN_WIDTH), BUTTON_PANEL_HEIGHT));
	button->addClickEventListener([callback, this](Ref* sender) {
		if (callback)
			callback(this);
        
        sSoundMgr->play(SOUND_BUTTON);
	});
	m_buttonPanel->addChild(button);

	layoutIfNeeded();
}

void Wicket::addTickButton(WicketButtonClickCallback const& callback, ButtonType type)
{
	if (!m_buttonPanel)
		return;


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
        sSoundMgr->play(SOUND_BUTTON);
	});

	m_buttonPanel->addChild(tickBtn);

	layoutIfNeeded();
}

void Wicket::addCrossButton(WicketButtonClickCallback const& callback, ButtonType type)
{
	if (!m_buttonPanel)
		return;


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
        sSoundMgr->play(SOUND_BUTTON);
	});

	m_buttonPanel->addChild(crossBtn);

	layoutIfNeeded();
}

void Wicket::setContent(Node* node)
{
	if (m_content)
	{
		m_content->removeFromParent();
		m_content = nullptr;
	}
	m_content = node;

	Size frameSize = m_mainPanel->getBoundingBox().size;
	m_content->setIgnoreAnchorPointForPosition(false);
	m_content->setAnchorPoint(Point::ANCHOR_MIDDLE_TOP);
	m_content->setPosition(Vec2(frameSize.width / 2, frameSize.height - CONTENT_MARGIN_TOP));
	if(m_buttonPanel)
		m_content->setContentSize(Size(frameSize.width - CONTENT_MARGIN_LEFTRIGHT * 2, m_content->getBoundingBox().getMaxY() - m_buttonPanel->getBoundingBox().getMaxY()));
	else
		m_content->setContentSize(Size(frameSize.width - CONTENT_MARGIN_LEFTRIGHT * 2, m_content->getBoundingBox().getMaxY() - CONTENT_MARGIN_BOTTOM));
	

	m_mainPanel->addChild(m_content, LOCAL_ZORDER_CONTENT);
}


bool Wicket::onTouchBegan(Touch* touch, Event* event)
{
	return true;
}

void Wicket::onTouchEnded(Touch* touch, Event* event)
{
}

void Wicket::onTouchCancelled(Touch* touch, Event* event)
{
}

void Wicket::onTouchMoved(Touch* touch, Event* event)
{
}

void Wicket::onKeyReleased(EventKeyboard::KeyCode keyCode, Event *event)
{
 	switch (keyCode)
	{
		case cocos2d::EventKeyboard::KeyCode::KEY_BACK:
			event->stopPropagation();
			this->close();
			break;
		default:
			break;
	}
}


void Wicket::onEnter()
{
	LayerColor::onEnter();

	if (m_firstLayout)
		this->layoutElements();
}

void Wicket::show()
{
	auto scene = Director::getInstance()->getRunningScene();
	NS_ASSERT(scene != nullptr);
	scene->addChild(this);

	m_touchListener->setSwallowTouches(true);

	m_mainPanel->setScale(0.1f);
	ScaleTo* scale = ScaleTo::create(0.25f, 1.0f);
	EaseBackOut* ease = EaseBackOut::create(scale);
	ease->setTag(ACTION_TAG_APPEAR);
	m_mainPanel->runAction(ease);
}

void Wicket::close()
{
	m_touchListener->setSwallowTouches(false);
    this->onClosed();

	if (m_resultCallback)
		m_resultCallback(m_resultCode, m_data);

	m_mainPanel->stopActionByTag(ACTION_TAG_APPEAR);

	ScaleBy* scale = ScaleBy::create(0.1f, 0.0f);
	CallFunc* callFunc = CallFunc::create([this]() {
		this->removeFromParent();
	});
	Sequence* seq = Sequence::create(scale, callFunc, nullptr);
	seq->setTag(ACTION_TAG_DISAPPEAR);
	m_mainPanel->runAction(seq);
}

Wicket* Wicket::getWicketByName(std::string const& name)
{
	auto scene = Director::getInstance()->getRunningScene();
	NS_ASSERT(scene != nullptr);
	return dynamic_cast<Wicket*>(scene->getChildByName(name));
}

void Wicket::setResult(ResultCode resultCode, ValueMapIntKey const& data)
{
	m_resultCode = resultCode;
	m_data = data;
}

void Wicket::initForm(Node* owner, cocos2d::Size const& frameSize, Node* title, Node* buttonPanel, ButtonAlignment buttonAlignment, Sprite* background, ui::Button* closeBtn)
{
	m_owner = owner;

	m_title = title;
	m_buttonPanel = buttonPanel;
	m_buttonAlignment = buttonAlignment;
	m_background = background;
	m_closeBtn = closeBtn;

	m_touchListener = EventListenerTouchOneByOne::create();
	CC_SAFE_RETAIN(m_touchListener);
	m_touchListener->onTouchBegan = CC_CALLBACK_2(Wicket::onTouchBegan, this);
	m_touchListener->onTouchMoved = CC_CALLBACK_2(Wicket::onTouchMoved, this);
	m_touchListener->onTouchEnded = CC_CALLBACK_2(Wicket::onTouchEnded, this);
	m_touchListener->onTouchCancelled = CC_CALLBACK_2(Wicket::onTouchCancelled, this);
	this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(m_touchListener, this);

	auto keyboardListener = EventListenerKeyboard::create();
	keyboardListener->onKeyReleased = CC_CALLBACK_2(Wicket::onKeyReleased, this);
	this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(keyboardListener, this);

	auto visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();

	m_mainPanel = Node::create();
	m_mainPanel->setIgnoreAnchorPointForPosition(false);
	m_mainPanel->setContentSize(frameSize);
	m_mainPanel->setAnchorPoint(Point::ANCHOR_MIDDLE);
	m_mainPanel->setPosition(origin.x + visibleSize.width / 2, origin.y + visibleSize.height / 2);
	this->addChild(m_mainPanel);

	// Background
	m_background->setContentSize(m_mainPanel->getContentSize());
	m_background->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
	m_mainPanel->addChild(m_background);

	// Close button
	if (m_closeBtn)
	{
		m_closeBtn->addClickEventListener(CC_CALLBACK_1(Wicket::buttonCloseCallback, this));
		m_mainPanel->addChild(m_closeBtn);
	}

	// Title
	m_mainPanel->addChild(m_title, LOCAL_ZORDER_TITLE);

	// Button panel
	if (m_buttonPanel)
		m_mainPanel->addChild(m_buttonPanel, LOCAL_ZORDER_BUTTONPANEL);
}

void Wicket::buttonCloseCallback(Ref* sender)
{
	this->close();
    sSoundMgr->play(SOUND_BUTTON);
}

void Wicket::alignButtons()
{
	if (!m_buttonPanel)
		return;


	Vector<Node*> const& buttons = m_buttonPanel->getChildren();
	Size panelSize = m_buttonPanel->getContentSize();

	if (m_buttonAlignment == BUTTON_ALIGNMENT_AVERAGE)
	{
		float gridMaxWidth = panelSize.width / buttons.size();
		float maxHeight = panelSize.height;
		float nextX = 0;
		for (auto it = buttons.begin(); it != buttons.end(); ++it)
		{
			Button* button = dynamic_cast<Button*>(*it);
			if (!button)
				continue;
			button->setAnchorPoint(Point::ANCHOR_MIDDLE);
			button->setPosition(Vec2(nextX + gridMaxWidth / 2, maxHeight / 2));

			nextX = nextX + gridMaxWidth;
		}
	}
	else // BUTTON_ALIGNMENT_RIGHT
	{
		int32 nextX = panelSize.width;
		float maxHeight = panelSize.height;
		for (ssize_t i = buttons.size() - 1; i >= 0; --i)
		{
			Button* button = dynamic_cast<Button*>(buttons.at(i));
			if (!button)
				continue;

			nextX -= BUTTON_MARGIN_LEFTRIGHT;
			button->setAnchorPoint(Point::ANCHOR_MIDDLE_RIGHT);
			button->setPosition(Point(nextX, maxHeight / 2));

			nextX = nextX - button->getContentSize().width;
		}

	}
}


void Wicket::layoutIfNeeded()
{
	if (!m_firstLayout)
		this->layoutElements();
}

void Wicket::layoutElements()
{
	this->alignButtons();

	m_firstLayout = false;
}

NS_END
