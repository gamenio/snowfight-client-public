//
//  InputBox.cpp
//  snowfight
//
//  Created by Luthier on 18/3/23.
//
//

#include "InputBox.h"

#include "ui/UIEditBox/UIEditBoxImpl-common.h"
#include "common/utils/StringUtility.h"

using namespace cocos2d::ui;

NS_BEGIN

#define MAX_CONTENT_LENGTH_INFINITE		-1

InputBox::InputBox() :
    m_maxContentLength(MAX_CONTENT_LENGTH_INFINITE),
	m_middlePlaceholderLabel(nullptr),
	m_enableMiddlePlaceholder(false)
{
    
}

InputBox::~InputBox()
{
	m_middlePlaceholderLabel = nullptr;
}

InputBox* InputBox::create(Size const& size, std::string const& normal9SpriteBg, TextureResType texType)
{
    InputBox* pRet = new (std::nothrow) InputBox();
    
    if (pRet != nullptr && pRet->init(size, normal9SpriteBg, texType))
    {
        pRet->autorelease();
    }
    else
    {
        CC_SAFE_DELETE(pRet);
    }
    
    return pRet;
}

InputBox* InputBox::create(Size const& size)
{
	InputBox* pRet = new (std::nothrow) InputBox();

	if (pRet != nullptr && pRet->init(size))
	{
		pRet->autorelease();
	}
	else
	{
		CC_SAFE_DELETE(pRet);
	}

	return pRet;
}

bool InputBox::init(Size const& size, std::string const& normal9SpriteBg, TextureResType texType)
{
    bool result = false;
    
    if(ui::EditBox::initWithSizeAndBackgroundSprite(size, normal9SpriteBg, texType))
    {
        this->setDelegate(this);
        result = true;
    }
    
    return result;
}

bool InputBox::init(Size const& size)
{
	bool result = false;

	if (ui::EditBox::initWithSizeAndBackgroundSprite(size, Scale9Sprite::create()))
	{
		this->setDelegate(this);
		result = true;
	}

	return result;
}

void InputBox::onEnter()
{
	ui::EditBox::onEnter();

	if (m_middlePlaceholderLabel)
	{
		std::string text = this->getText();
		m_middlePlaceholderLabel->setVisible(text.empty());
	}
}

void InputBox::editBoxEditingDidBegin(ui::EditBox* editBox)
{
	if (m_middlePlaceholderLabel)
		m_middlePlaceholderLabel->setVisible(false);

    if(m_editCallback)
        m_editCallback(this, INPUTBOX_EVENT_EDITING_DID_BEGIN);
}

void InputBox::editBoxEditingDidEndWithAction(ui::EditBox* editBox, ui::EditBoxDelegate::EditBoxEndAction action)
{
    if (m_maxContentLength != MAX_CONTENT_LENGTH_INFINITE)
    {
        std::string text = editBox->getText();
        int32 count = InputBox::calcContentLength(text);
        if (count > m_maxContentLength)
        {
            std::string str = StringUtility::substring(text, m_maxContentLength);
            editBox->setText(str.c_str());
        }
    }
    
	if (m_middlePlaceholderLabel)
	{
        std::string text = editBox->getText();
		if (text.empty())
			m_middlePlaceholderLabel->setVisible(true);
	}

    if(m_editCallback)
        m_editCallback(this, INPUTBOX_EVENT_EDITING_DID_END);
}

void InputBox::editBoxReturn(ui::EditBox* editBox)
{
    if(m_editCallback)
        m_editCallback(this, INPUTBOX_EVENT_RETURN);
}

int32 InputBox::calcContentLength(std::string const& utf8str)
{
    int32 count = 0;
    
    char const* cstr = utf8str.c_str();
    uint32 pos = 0;
    uint32 len = static_cast<uint32>(strlen(cstr));
    uint8 ch;
    
    while (pos < len)
    {
        ch = cstr[pos];
        uint32 shift = 0;
        uint8 x = 0x80;
        uint8 b = x;
        while ((b & ch) != 0)
        {
            shift++;
            b = x >> shift;
        }
        // 多字节字符
        if (shift > 0)
        {
            count += 2;
            pos += shift;
        }
        // 单字节字符
        else 
        {
            pos++;
            count++;
        }
        
    }
    
    return count;
}

void InputBox::editBoxTextChanged(ui::EditBox* editBox, std::string const& text)
{    
    if (m_maxContentLength != MAX_CONTENT_LENGTH_INFINITE)
    {
        std::string text = editBox->getText();
        int32 count = InputBox::calcContentLength(text);
        if (count > m_maxContentLength)
        {
            std::string str = StringUtility::substring(text, m_maxContentLength);
            editBox->setText(str.c_str());
        }
    }

    if(m_editCallback)
        m_editCallback(this, INPUTBOX_EVENT_TEXT_CHANGED);
}

void InputBox::enableMiddlePlaceholder()
{
	if (m_enableMiddlePlaceholder)
		return;

	m_middlePlaceholderLabel = Label::createWithSystemFont("", DEFAULT_SYSTEM_FONT, 12);
	m_middlePlaceholderLabel->setPosition(this->getContentSize().width / 2, this->getContentSize().height / 2);
	this->addChild(m_middlePlaceholderLabel);

	this->setPlaceHolder("");

	m_enableMiddlePlaceholder = true;
}

void InputBox::setMiddlePlaceholderFontColor(Color4B const& color)
{
	if (!m_middlePlaceholderLabel)
		this->enableMiddlePlaceholder();
	
	m_middlePlaceholderLabel->setTextColor(color);
}

void InputBox::setMiddlePlaceHolder(std::string const& text)
{
	if (!m_middlePlaceholderLabel)
		this->enableMiddlePlaceholder();

	m_middlePlaceholderLabel->setString(text);
}

void InputBox::setMiddlePlaceholderFont(std::string const& systemFont, float fontSize)
{
	if (!m_middlePlaceholderLabel)
		this->enableMiddlePlaceholder();
	
	m_middlePlaceholderLabel->setSystemFontName(systemFont);
	m_middlePlaceholderLabel->setSystemFontSize(fontSize);
}

void InputBox::setText(const char* pText)
{
    EditBox::setText(pText);
    if (m_middlePlaceholderLabel)
        m_middlePlaceholderLabel->setVisible(strlen(pText) == 0);
}

NS_END
