#include "SmileyBox.h"

#include "scene/SoundMgr.h"
#include "scene/TutorialService.h"

NS_BEGIN

using namespace cocos2d::ui;

#if NS_DEBUG
#define DEBUG_SMILEYS_CLIPPER              0
#endif // NS_DEBUG

#define FRAMENAME_SWITCH			"battle_btn_smiley.png"
#define FRAMENAME_SMILEY_SHADOW		"smiley_shadow.png"

#define NUMBER_OF_SMILEYS			    6
#define SMILEY_FRAMENAME_FORMAT			"smiley%d.png"
#define SMILEY_SIZE					    32.0f
#define SMILEY_PRESSED_ZOOM_SCALE       0.4f

static uint16 sSmileysPosition[NUMBER_OF_SMILEYS] = {
    SMILEY_ANGRY,
    SMILEY_CRY,
    SMILEY_SAD,
    SMILEY_LAUGH,
    SMILEY_DEVIL,
    SMILEY_NAUGHTY,
};

#define ACTION_TAG_POPUP_PANEL_SHOWING			1
#define ACTION_TAG_POPUP_PANEL_HIDING			2

SmileyBox* SmileyBox::create()
{
	auto ret = new (std::nothrow) SmileyBox();
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

bool SmileyBox::init()
{
	if (!Node::init())
		return false;

	this->setIgnoreAnchorPointForPosition(false);
	this->setCascadeOpacityEnabled(true);

	DrawNode* stencil = DrawNode::create();
	stencil->setAnchorPoint(Point::ANCHOR_MIDDLE_RIGHT);
	m_popupPanel = ClippingNode::create(stencil);
	this->addChild(m_popupPanel);

	m_smileyContainer = Node::create();
	m_smileyContainer->setIgnoreAnchorPointForPosition(false);
	m_smileyContainer->setAnchorPoint(Point::ANCHOR_MIDDLE_LEFT);
	m_smileyContainer->setVisible(false);
	for (int32 i = 0; i < NUMBER_OF_SMILEYS; ++i)
	{
        int32 code = sSmileysPosition[i];
		std::string fn = StringUtils::format(SMILEY_FRAMENAME_FORMAT, code);
		Button* smiley = Button::create(fn, "", "", Widget::TextureResType::PLIST);
		smiley->addClickEventListener(CC_CALLBACK_1(SmileyBox::onSmileySelected, this));
        smiley->setZoomScale(SMILEY_PRESSED_ZOOM_SCALE);
		smiley->setTag(code);
		smiley->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
		m_smileyContainer->addChild(smiley);
	}
	m_popupPanel->addChild(m_smileyContainer);

	m_switchBtn = Button::create(FRAMENAME_SWITCH, "", "", Widget::TextureResType::PLIST);
	m_switchBtn->setAnchorPoint(Point::ANCHOR_MIDDLE);
	m_switchBtn->addClickEventListener(CC_CALLBACK_1(SmileyBox::buttonSwitchCallback, this));
	this->addChild(m_switchBtn);

    
    m_debugDrawNode = DrawNode::create();
    this->addChild(m_debugDrawNode);
    
	this->layoutElements();

	return true;
}

SmileyBox::SmileyBox() :
	m_isShowPopupPanel(false),
	m_isPopupPanelEnabled(true),
    m_isEnabled(true),
	m_switchBtn(nullptr),
	m_popupPanel(nullptr),
	m_smileyContainer(nullptr),
    m_debugDrawNode(nullptr)
{
}

SmileyBox::~SmileyBox()
{
	m_switchBtn = nullptr;
	m_popupPanel = nullptr;
	m_smileyContainer = nullptr;
    m_debugDrawNode = nullptr;
}


void SmileyBox::setSelectEventListener(SmileyBoxSelectCallback const& callback)
{
    m_selectCallback = callback;
}

void SmileyBox::setEnabled(bool enabled)
{
    if(m_isEnabled != enabled)
    {
        auto& children = m_smileyContainer->getChildren();
        for (auto it = children.begin(); it != children.end(); ++it)
        {
            Button* smiley = dynamic_cast<Button*>(*it);
            smiley->setEnabled(enabled);
        }
        
        m_switchBtn->setEnabled(enabled);
        m_isEnabled = enabled;
    }
}

void SmileyBox::setPopupPanelTouchEnabled(bool enabled)
{
	if (m_isPopupPanelEnabled != enabled)
	{
		auto& children = m_smileyContainer->getChildren();
		for (auto it = children.begin(); it != children.end(); ++it)
		{
			Button* smiley = dynamic_cast<Button*>(*it);
			smiley->setTouchEnabled(enabled);
		}
		m_isPopupPanelEnabled = enabled;
	}
}

void SmileyBox::layoutElements()
{
	Size contentSize = m_switchBtn->getContentSize();
	m_switchBtn->setPosition(Vec2(contentSize.width / 2, contentSize.height / 2));

    this->arrangeSmileysInContainer();
    m_popupPanel->setPosition(Vec2(m_switchBtn->getBoundingBox().getMidX(), contentSize.height / 2));
    
	DrawNode* stencil = dynamic_cast<DrawNode*>(m_popupPanel->getStencil());
	if (stencil)
	{
		stencil->setContentSize(Size(m_switchBtn->getContentSize().width / 2 + m_smileyContainer->getContentSize().width,
			m_smileyContainer->getContentSize().height * (1.0f + SMILEY_PRESSED_ZOOM_SCALE)));
        stencil->drawSolidRect(Point::ZERO, stencil->getContentSize(), Color4F::BLACK);
        
#if DEBUG_SMILEYS_CLIPPER
		Vec2 origin;
		origin.x = m_popupPanel->getPosition().x + stencil->getBoundingBox().getMinX();
		origin.y = m_popupPanel->getPosition().y + stencil->getBoundingBox().getMinY();
		Vec2 destination = origin + stencil->getBoundingBox().size;
		m_debugDrawNode->drawRect(origin, destination, Color4F::RED);
#endif // DEBUG_SMILEYS_CLIPPER
	}

	this->setContentSize(contentSize);
}

void SmileyBox::arrangeSmileysInContainer()
{
	auto& children = m_smileyContainer->getChildren();
	float nextX = 0.0f;
	for (auto it = children.begin(); it != children.end(); ++it)
	{
		Node* node = *it;
		node->setPosition(Vec2(nextX, 0));
		node->setContentSize(Size(SMILEY_SIZE, SMILEY_SIZE));
		nextX += SMILEY_SIZE;
	}
	m_smileyContainer->setContentSize(Size(nextX, SMILEY_SIZE));
}

void SmileyBox::switchPopupPanel()
{
	if (m_isShowPopupPanel)
	{
		this->hidePopupPanel();
	}
	else
	{
		this->setPopupPanelTouchEnabled(true);
		this->showPopupPanel();
	}
}

void SmileyBox::showPopupPanel()
{
	if (m_smileyContainer->getActionByTag(ACTION_TAG_POPUP_PANEL_HIDING))
		m_smileyContainer->stopActionByTag(ACTION_TAG_POPUP_PANEL_HIDING);

	Vec2 dest(-(m_switchBtn->getContentSize().width / 2 + m_smileyContainer->getContentSize().width), m_smileyContainer->getPositionY());
	EaseSineOut* move = EaseSineOut::create(MoveTo::create(0.15f, dest));
	Sequence* seq = Sequence::create(Show::create(), move, nullptr);
	seq->setTag(ACTION_TAG_POPUP_PANEL_SHOWING);
	m_smileyContainer->runAction(seq);
    
	m_isShowPopupPanel = true;
}

void SmileyBox::hidePopupPanel()
{
	if (m_smileyContainer->getActionByTag(ACTION_TAG_POPUP_PANEL_SHOWING))
		m_smileyContainer->stopActionByTag(ACTION_TAG_POPUP_PANEL_SHOWING);

	Vec2 dest(0, m_smileyContainer->getPositionY());
	EaseSineIn* move = EaseSineIn::create(MoveTo::create(0.15f, dest));
	Sequence* seq = Sequence::create(move, Hide::create(), nullptr);
	seq->setTag(ACTION_TAG_POPUP_PANEL_HIDING);
	m_smileyContainer->runAction(seq);

	m_isShowPopupPanel = false;

}

void SmileyBox::buttonSwitchCallback(Ref* sender)
{
	this->switchPopupPanel();
    sSoundMgr->play(SOUND_BUTTON);

	auto service = TutorialService::getInstance();
	if(service->isEnabled())
		service->triggerEvent(TUTORIAL_EVENT_SMILEY_BOX_TAPPED);
}

void SmileyBox::onSmileySelected(Ref* sender)
{
	if (!m_isShowPopupPanel)
		return;

	Button* smiley = dynamic_cast<Button*>(sender);
	if (!smiley)
		return;

	this->hidePopupPanel();
	this->setPopupPanelTouchEnabled(false);
	sSoundMgr->play(SOUND_BUTTON);

    if(m_selectCallback)
    {
        uint16 code = static_cast<uint16>(smiley->getTag());
        m_selectCallback(this, code);
    }
}


NS_END
