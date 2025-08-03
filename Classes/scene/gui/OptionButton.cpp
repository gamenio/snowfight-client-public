#include "OptionButton.h"

#include "ui/UIScale9Sprite.h"

#include "../SoundMgr.h"
#include "../Utils.h"

NS_BEGIN

using namespace cocos2d::ui;

//static const int32 NORMAL_RENDERER_Z = (-2);
static const int32 PRESSED_RENDERER_Z = (-2);
//static const int32 DISABLED_RENDERER_Z = (-2);
static const int32 TITLE_RENDERER_Z = (-1);
static const int32 ICON_IMAGE_RENDERER_Z = (-1);

#define FRAMENAME_BACKGROUND_NORMAL				"option_btn_normal_bg.png"
#define FRAMENAME_BACKGROUND_PRESSED			"option_btn_pressed_bg.png"

OptionButton* OptionButton::create(Size const& contentSize, std::string const& title, std::string const& iconImage, TextureResType texType)
{
	OptionButton* pRet = new OptionButton();
	if (pRet && pRet->init(contentSize, title, iconImage, texType))
	{
		pRet->autorelease();
		return pRet;
	}
	else
	{
		CC_SAFE_DELETE(pRet);
		return nullptr;
	}
}

bool OptionButton::init(Size const& contentSize, std::string const& title, std::string const& iconImage, TextureResType texType)
{
	if (!ui::Widget::init())
		return false;

	m_texType = texType;

	this->ignoreContentAdaptWithSize(false);
	this->setContentSize(contentSize);

	m_titleRender->setString(title);
	m_titleRender->setSystemFontName(DEFAULT_SYSTEM_FONT);
	m_titleRender->setSystemFontSize(11);
	m_titleRender->setAnchorPoint(Point::ANCHOR_MIDDLE_LEFT);

	if(!iconImage.empty())
		this->loadTextureIconImage(iconImage);

	m_backgroundNormalRender->initWithSpriteFrameName(FRAMENAME_BACKGROUND_NORMAL);
	m_backgroundNormalRender->setAnchorPoint(Point::ANCHOR_MIDDLE_LEFT);

	m_backgroundPressedRender->initWithSpriteFrameName(FRAMENAME_BACKGROUND_PRESSED);
	m_backgroundPressedRender->setAnchorPoint(m_backgroundNormalRender->getAnchorPoint());

	return true;
}

OptionButton::OptionButton() :
	m_titleRender(nullptr),
	m_iconImageRender(nullptr),
	m_backgroundPressedRender(nullptr),
	m_backgroundNormalRender(nullptr),
	m_texType(TextureResType::PLIST),
	m_rendersAdaptDirty(true)
{
	setTouchEnabled(true);
}

OptionButton::~OptionButton()
{
}

void OptionButton::setTitle(std::string const& title)
{
	m_titleRender->setString(title);
	m_rendersAdaptDirty = true;
}

void OptionButton::setIconImage(std::string const& iconImage)
{
	this->loadTextureIconImage(iconImage);
	m_rendersAdaptDirty = true;
}

void OptionButton::onSizeChanged()
{
	Widget::onSizeChanged();

	m_rendersAdaptDirty = true;
}

void OptionButton::adaptRenderers()
{
	ui::Widget::adaptRenderers();

	if (!m_rendersAdaptDirty)
		return;

	Size contentSize = this->getContentSize();

	m_iconImageRender->setPosition(Vec2(3.0f, contentSize.height / 2));
	float titleX = m_iconImageRender->getBoundingBox().getMaxX() + 5.0f;
	m_titleRender->setPosition(Vec2(titleX, contentSize.height / 2));
	m_titleRender->enableWrap(false);
	m_titleRender->setWidth(contentSize.width - titleX - 18);

	m_backgroundNormalRender->setPosition(0, contentSize.height / 2);
	m_backgroundNormalRender->setContentSize(Size(contentSize.width, contentSize.height));
	m_backgroundPressedRender->setPosition(m_backgroundNormalRender->getPosition());
	m_backgroundPressedRender->setContentSize(m_backgroundNormalRender->getContentSize());

	m_rendersAdaptDirty = false;
}

void OptionButton::initRenderer()
{
	m_iconImageRender = Sprite::create();
	m_titleRender = Label::create();

	m_backgroundPressedRender = ui::Scale9Sprite::create();
	m_backgroundPressedRender->setRenderingType(ui::Scale9Sprite::RenderingType::SLICE);

	m_backgroundNormalRender = ui::Scale9Sprite::create();
	m_backgroundNormalRender->setRenderingType(ui::Scale9Sprite::RenderingType::SLICE);

	addProtectedChild(m_titleRender, TITLE_RENDERER_Z, -1);
	addProtectedChild(m_iconImageRender, ICON_IMAGE_RENDERER_Z, -1);
	addProtectedChild(m_backgroundPressedRender, PRESSED_RENDERER_Z, -1);
	addProtectedChild(m_backgroundNormalRender, PRESSED_RENDERER_Z, -1);
}

void OptionButton::onPressStateChangedToNormal()
{
	m_backgroundPressedRender->setVisible(false);
	m_backgroundNormalRender->setVisible(true);
	// m_iconImageRender->setColor(Color3B::WHITE);
	updateTitleRenderColor(Color4B(128, 109, 66, 255));

}

void OptionButton::onPressStateChangedToPressed()
{
	m_backgroundPressedRender->setVisible(true);
	m_backgroundNormalRender->setVisible(false);
	// m_iconImageRender->setColor(Color3B::GRAY);
	updateTitleRenderColor(Color4B(112, 94, 58, 255));
}

void OptionButton::onPressStateChangedToDisabled()
{
}

void OptionButton::releaseUpEvent()
{
    Widget::releaseUpEvent();
    sSoundMgr->play(SOUND_BUTTON);
}

void OptionButton::loadTextureIconImage(std::string const& image)
{
	switch (m_texType)
	{
	case TextureResType::LOCAL:
		m_iconImageRender->initWithFile(image);
		break;
	case TextureResType::PLIST:
		m_iconImageRender->initWithSpriteFrameName(image);
		break;
	default:
		break;
	}
	m_iconImageRender->setAnchorPoint(Point::ANCHOR_MIDDLE_LEFT);

}

void OptionButton::updateTitleRenderColor(Color4B const& color)
{
	m_titleRender->setTextColor(color);
	Utils::enableBoldForLabel(m_titleRender);
}


NS_END
