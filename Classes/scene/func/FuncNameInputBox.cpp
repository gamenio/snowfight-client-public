#include "FuncNameInputBox.h"

#include "game/LocaleMgr.h"
#include "../AssetsLoader.h"


NS_BEGIN

#define  INPUTBOX_CONTENT_SIZE				Size(110, 22)

#define FRAMENAME_FUNC_NAMEINPUT_BG				"func_nameinput_bg.png"
#define FRAMENAME_FUNC_NAMEINPUT_FG				"func_nameinput_fg.png"

FuncNameInputBox::FuncNameInputBox() :
	m_inputBox(nullptr)
{
}

FuncNameInputBox::~FuncNameInputBox()
{
	m_inputBox = nullptr;
}

FuncNameInputBox* FuncNameInputBox::create()
{
	FuncNameInputBox* node = new (std::nothrow) FuncNameInputBox();
	if (node && node->init())
	{
		node->autorelease();
		return node;
	}
	CC_SAFE_DELETE(node);
	return nullptr;
}

bool FuncNameInputBox::init()
{
	if (!Node::init())
		return false;

	this->setIgnoreAnchorPointForPosition(false);

	Sprite* bgSp = Sprite::createWithSpriteFrameName(FRAMENAME_FUNC_NAMEINPUT_BG);
	bgSp->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
	this->addChild(bgSp);

	Size contentSize = bgSp->getContentSize();

	m_inputBox = InputBox::create(INPUTBOX_CONTENT_SIZE);
	m_inputBox->setAnchorPoint(Point::ANCHOR_MIDDLE);
	m_inputBox->setPosition(Vec2(contentSize.width / 2, contentSize.height / 2));
	m_inputBox->setMiddlePlaceholderFontColor(Color4B(190, 190, 191, 255));
	m_inputBox->setMiddlePlaceholderFont(DEFAULT_SYSTEM_FONT, 12);
	m_inputBox->setMiddlePlaceHolder(sLocaleMgr->getString("func_tips_enter_nickname").c_str());
	m_inputBox->setTextHorizontalAlignment(TextHAlignment::CENTER);
	m_inputBox->setFontSize(12);
	m_inputBox->setFontColor(Color3B(231, 225, 220));
	m_inputBox->setReturnType(ui::EditBox::KeyboardReturnType::DONE);
	m_inputBox->setInputMode(ui::EditBox::InputMode::SINGLE_LINE);
	m_inputBox->setMaxContentLength(12);
	this->addChild(m_inputBox);

	Sprite* fgSp = Sprite::createWithSpriteFrameName(FRAMENAME_FUNC_NAMEINPUT_FG);
	fgSp->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
	this->addChild(fgSp);

	this->setContentSize(contentSize);

	return true;
}

void FuncNameInputBox::setText(std::string const& text)
{
	m_inputBox->setText(text.c_str());
}

std::string FuncNameInputBox::getText() const
{
	return m_inputBox->getText();
}

void FuncNameInputBox::setEnabled(bool enabled)
{
	m_inputBox->setEnabled(enabled);
}

void FuncNameInputBox::setEditEventListener(InputBoxEditCallback const& callback)
{
	m_inputBox->setEditEventListener(callback);
}

NS_END
