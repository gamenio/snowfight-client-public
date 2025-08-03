#include "CatwalkUnit.h"

#include "scene/GLProgramManager.h"

NS_BEGIN

CatwalkUnit* CatwalkUnit::create(std::string const& filename)
{
	CatwalkUnit *sprite = new (std::nothrow) CatwalkUnit();
	if (sprite && sprite->initWithFile(filename))
	{
		sprite->autorelease();
		return sprite;
	}
	CC_SAFE_DELETE(sprite);
	return nullptr;
}

CatwalkUnit* CatwalkUnit::create(std::string const& filename, Rect const& rect)
{
	CatwalkUnit *sprite = new (std::nothrow) CatwalkUnit();
	if (sprite && sprite->initWithFile(filename, rect))
	{
		sprite->autorelease();
		return sprite;
	}
	CC_SAFE_DELETE(sprite);
	return nullptr;
}

CatwalkUnit* CatwalkUnit::createWithTexture(Texture2D* texture)
{
	CatwalkUnit *sprite = new (std::nothrow) CatwalkUnit();
	if (sprite && sprite->initWithTexture(texture))
	{
		sprite->autorelease();
		return sprite;
	}
	CC_SAFE_DELETE(sprite);
	return nullptr;
}

CatwalkUnit* CatwalkUnit::createWithTexture(Texture2D* texture, Rect const& rect, bool rotated)
{
	CatwalkUnit *sprite = new (std::nothrow) CatwalkUnit();
	if (sprite && sprite->initWithTexture(texture, rect, rotated))
	{
		sprite->autorelease();
		return sprite;
	}
	CC_SAFE_DELETE(sprite);
	return nullptr;
}

CatwalkUnit* CatwalkUnit::createWithSpriteFrame(SpriteFrame* spriteFrame)
{
	CatwalkUnit *sprite = new (std::nothrow) CatwalkUnit();
	if (sprite && spriteFrame && sprite->initWithSpriteFrame(spriteFrame))
	{
		sprite->autorelease();
		return sprite;
	}
	CC_SAFE_DELETE(sprite);
	return nullptr;
}

CatwalkUnit* CatwalkUnit::createWithSpriteFrameName(std::string const& spriteFrameName)
{
	SpriteFrame *frame = SpriteFrameCache::getInstance()->getSpriteFrameByName(spriteFrameName);

#if COCOS2D_DEBUG > 0
	char msg[256] = { 0 };
    snprintf(msg, sizeof(msg), "Invalid spriteFrameName: %s", spriteFrameName.c_str());
	CCASSERT(frame != nullptr, msg);
#endif

	return createWithSpriteFrame(frame);
}

void CatwalkUnit::setLightness(float lightness)
{
	m_lightness = lightness;
	GLProgramState* state = this->getGLProgramState();
	state->setUniformFloat("u_lightness", m_lightness);
}

Rect CatwalkUnit::getTrimmedBoundingBox() const
{
	Point offsetPos = this->getOffsetPosition();
	Rect texRect = this->getTextureRect();

	Rect rect(offsetPos.x, offsetPos.y, texRect.size.width, texRect.size.height);
	rect = RectApplyAffineTransform(rect, getNodeToParentAffineTransform());

	return rect;
}

CatwalkUnit::CatwalkUnit() :
	m_lightness(0)
{

}

CatwalkUnit::~CatwalkUnit()
{
}

bool CatwalkUnit::initWithTexture(Texture2D* texture)
{
	CCASSERT(texture != nullptr, "Invalid texture for sprite");

	Rect rect = Rect::ZERO;
	rect.size = texture->getContentSize();

	return initWithTexture(texture, rect);
}

bool CatwalkUnit::initWithTexture(Texture2D* texture, Rect const& rect)
{
	return initWithTexture(texture, rect, false);
}

bool CatwalkUnit::initWithTexture(Texture2D* texture, Rect const& rect, bool rotated)
{
	bool ret = Sprite::initWithTexture(texture, rect, rotated);
	if (ret)
	{
        auto glProgram = GLProgramManager::getInstance()->getGLProgram(SHADER_NAME_LIGHTNESS);
        auto glProgramState = GLProgramState::create(glProgram);
        setGLProgramState(glProgramState);
	}
	return ret;
}

bool CatwalkUnit::initWithSpriteFrame(SpriteFrame* spriteFrame)
{
	CCASSERT(spriteFrame != nullptr, "");

	bool bRet = initWithTexture(spriteFrame->getTexture(), spriteFrame->getRect());
	setSpriteFrame(spriteFrame);

	return bRet;
}


NS_END
