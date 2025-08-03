#include "SILLabel.h"

#include "scene/GLProgramManager.h"
#include "2d/CCFontAtlasCache.h"
#include "SILSprite.h"
#include "SILCommon.h"

NS_BEGIN

//  ETC1 ALPHA supports, for LabelType::BMFONT & LabelType::CHARMAP
static Texture2D* _getTexture(Label* label)
{
	auto fontAtlas = label->getFontAtlas();
	Texture2D* texture = nullptr;
	if (fontAtlas != nullptr) {
		auto textures = fontAtlas->getTextures();
		if (!textures.empty()) {
			texture = textures.begin()->second;
		}
	}
	return texture;
}

SILLabel* SILLabel::createWithSystemFont(std::string const& text, std::string const& font, float fontSize, Size const& dimensions, TextHAlignment hAlignment, TextVAlignment vAlignment)
{
	auto ret = new (std::nothrow) SILLabel(hAlignment, vAlignment);
	if (ret)
	{
		ret->setSystemFontName(font);
		ret->setSystemFontSize(fontSize);
		ret->setDimensions(dimensions.width, dimensions.height);
		ret->setString(text);
		ret->autorelease();

		return ret;
	}

	return nullptr;
}

SILLabel* SILLabel::createWithBMFont(std::string const& bmfontPath, std::string const& text, TextHAlignment const& hAlignment, int maxLineWidth, Vec2 const& imageOffset)
{
	auto ret = new (std::nothrow) SILLabel(hAlignment);
	if (ret && ret->setBMFontFilePath(bmfontPath, imageOffset))
	{
		ret->setMaxLineWidth(maxLineWidth);
		ret->setString(text);
		ret->autorelease();

		return ret;
	}

	delete ret;
	return nullptr;
}

SILLabel::SILLabel(TextHAlignment hAlignment, TextVAlignment vAlignment) :
	Label(hAlignment, vAlignment),
	m_isSilhouetted(true),
	m_effectOpacity(SILHOUETTE_EFFECT_OPACITY_DEFAULT),
	m_screenTexture(nullptr)
{
}

SILLabel::~SILLabel()
{
	CC_SAFE_RELEASE_NULL(m_screenTexture);
}

void SILLabel::enableShadow(Color4B const& shadowColor, Size const& offset, int blurRadius)
{
	_shadowEnabled = true;
	_shadowDirty = true;

	_shadowOffset.width = offset.width;
	_shadowOffset.height = offset.height;
	//TODO: support blur for shadow

	_shadowColor3B.r = shadowColor.r;
	_shadowColor3B.g = shadowColor.g;
	_shadowColor3B.b = shadowColor.b;
	_shadowOpacity = shadowColor.a;

	if (!_systemFontDirty && !_contentDirty && _textSprite)
	{
		auto fontDef = _getFontDefinition();
		if (_shadowNode)
		{
			if (shadowColor != _shadowColor4F)
			{
				_shadowNode->release();
				_shadowNode = nullptr;
				createShadowSpriteForSystemFont(fontDef);
			}
			else
			{
				_shadowNode->setPosition(_shadowOffset.width, _shadowOffset.height);
			}
		}
		else
		{
			createShadowSpriteForSystemFont(fontDef);
		}
	}

	_shadowColor4F.r = shadowColor.r / 255.0f;
	_shadowColor4F.g = shadowColor.g / 255.0f;
	_shadowColor4F.b = shadowColor.b / 255.0f;
	_shadowColor4F.a = shadowColor.a / 255.0f;

	if (_currentLabelType == LabelType::BMFONT || _currentLabelType == LabelType::CHARMAP)
	{
		setGLProgramState(GLProgramState::getOrCreateWithGLProgramName(_shadowEnabled ? GLProgram::SHADER_NAME_POSITION_TEXTURE_COLOR : GLProgram::SHADER_NAME_POSITION_TEXTURE_COLOR_NO_MVP, _getTexture(this)));
	}
}

void SILLabel::updateContent()
{
	if (_systemFontDirty)
	{
		if (_fontAtlas)
		{
			_batchNodes.clear();
			CC_SAFE_RELEASE_NULL(_reusedLetter);
			FontAtlasCache::releaseFontAtlas(_fontAtlas);
			_fontAtlas = nullptr;
		}

		_systemFontDirty = false;
	}

	CC_SAFE_RELEASE_NULL(_textSprite);
	CC_SAFE_RELEASE_NULL(_shadowNode);
	bool updateFinished = true;

	if (_fontAtlas)
	{
		std::u32string utf32String;
		if (StringUtils::UTF8ToUTF32(_utf8Text, utf32String))
		{
			_utf32Text = utf32String;
		}

		computeHorizontalKernings(_utf32Text);
		updateFinished = alignText();
	}
	else
	{
		auto fontDef = _getFontDefinition();
		createSpriteForSystemFont(fontDef);
		if (_shadowEnabled)
		{
			createShadowSpriteForSystemFont(fontDef);
		}
	}

	if (_underlineNode)
	{
		_underlineNode->clear();

		if (_numberOfLines)
		{
			// This is the logic for TTF fonts
			const float charheight = (_textDesiredHeight / _numberOfLines);
			_underlineNode->setLineWidth(charheight / 6);

			// atlas font
			for (int i = 0; i < _numberOfLines; ++i)
			{
				float offsety = 0;
				if (_strikethroughEnabled)
					offsety += charheight / 2;
				// FIXME: Might not work with different vertical alignments
				float y = (_numberOfLines - i - 1) * charheight + offsety;

				// Github issue #15214. Uses _displayedColor instead of _textColor for the underline.
				// This is to have the same behavior of SystemFonts.
				_underlineNode->drawLine(Vec2(_linesOffsetX[i], y), Vec2(_linesWidth[i] + _linesOffsetX[i], y), Color4F(_displayedColor));
			}
		}
		else if (_textSprite)
		{
			// ...and is the logic for System fonts
			float y = 0;
			const auto spriteSize = _textSprite->getContentSize();
			_underlineNode->setLineWidth(spriteSize.height / 6);

			if (_strikethroughEnabled)
				// FIXME: system fonts don't report the height of the font correctly. only the size of the texture, which is POT
				y += spriteSize.height / 2;
			// FIXME: Might not work with different vertical alignments
			_underlineNode->drawLine(Vec2(0, y), Vec2(spriteSize.width, y), Color4F(_textSprite->getDisplayedColor()));
		}
	}

	if (updateFinished) {
		_contentDirty = false;
	}

#if CC_LABEL_DEBUG_DRAW
	_debugDrawNode->clear();
	Vec2 vertices[4] =
	{
		Vec2::ZERO,
		Vec2(_contentSize.width, 0),
		Vec2(_contentSize.width, _contentSize.height),
		Vec2(0, _contentSize.height)
	};
	_debugDrawNode->drawPoly(vertices, 4, true, Color4F::WHITE);
#endif
}

void SILLabel::setSilhouetted(bool silhouetted)
{
	if (m_isSilhouetted != silhouetted)
	{
		m_isSilhouetted = silhouetted;

		if (_textSprite)
			(dynamic_cast<SilhouettableProtocol*>(_textSprite))->setSilhouetted(silhouetted);
		if (_shadowNode)
			(dynamic_cast<SilhouettableProtocol*>(_shadowNode))->setSilhouetted(silhouetted);
		if (_reusedLetter)
			(dynamic_cast<SilhouettableProtocol*>(_reusedLetter))->setSilhouetted(silhouetted);
	}
}

void SILLabel::setEffectOpacity(GLubyte opacity)
{
	if (m_effectOpacity != opacity)
	{
		m_effectOpacity = opacity;
		if (_textSprite)
			(dynamic_cast<SilhouettableProtocol*>(_textSprite))->setEffectOpacity(opacity);
		if (_shadowNode)
			(dynamic_cast<SilhouettableProtocol*>(_shadowNode))->setEffectOpacity(opacity);
		if (_reusedLetter)
			(dynamic_cast<SilhouettableProtocol*>(_reusedLetter))->setEffectOpacity(opacity);
	}
}

void SILLabel::setScreenTexture(Texture2D* texture)
{
	if(m_screenTexture != texture)
	{
		CC_SAFE_RETAIN(texture);
		CC_SAFE_RELEASE(m_screenTexture);
		m_screenTexture = texture;
		if (_textSprite)
			(dynamic_cast<SilhouettableProtocol*>(_textSprite))->setScreenTexture(texture);
		if (_shadowNode)
			(dynamic_cast<SilhouettableProtocol*>(_shadowNode))->setScreenTexture(texture);
		if(_reusedLetter)
			(dynamic_cast<SilhouettableProtocol*>(_reusedLetter))->setScreenTexture(texture);
	}
}

void SILLabel::visit(Renderer* renderer, Mat4 const& parentTransform, uint32_t parentFlags)
{
	if (!_visible)
		return;

	if (!m_isSilhouetted && (parentFlags & FLAGS_RENDER_SILHOUETTE) != 0)
		return;

	Label::visit(renderer, parentTransform, parentFlags);
}

void SILLabel::draw(Renderer* renderer, Mat4 const& transform, uint32_t flags)
{
	if (_batchNodes.empty() || _lengthOfString <= 0)
	{
		return;
	}
	// Don't do calculate the culling if the transform was not updated
	bool transformUpdated = flags & FLAGS_TRANSFORM_DIRTY;
#if CC_USE_CULLING
	auto visitingCamera = Camera::getVisitingCamera();
	auto defaultCamera = Camera::getDefaultCamera();
	if (visitingCamera == defaultCamera) {
		_insideBounds = (transformUpdated || visitingCamera->isViewProjectionUpdated()) ? renderer->checkVisibility(transform, _contentSize) : _insideBounds;
	}
	else
	{
		_insideBounds = renderer->checkVisibility(transform, _contentSize);
	}

	if (_insideBounds)
#endif
	{
		if (!_shadowEnabled && (_currentLabelType == LabelType::BMFONT || _currentLabelType == LabelType::CHARMAP))
		{
			for (auto&& it : _letters)
			{
				it.second->updateTransform();
			}
			// ETC1 ALPHA supports for BMFONT & CHARMAP
			auto textureAtlas = _batchNodes.at(0)->getTextureAtlas();
			auto texture = textureAtlas->getTexture();
			SILSprite* sprite = static_cast<SILSprite*>(_reusedLetter);

			if ((flags & FLAGS_SILHOUETTE_RENDER_MASK) != 0)
			{
				m_maskQuadCommand.init(_globalZOrder,
					texture, 
					sprite->getMaskGLProgramState(),
					_blendFunc,
					textureAtlas->getQuads(),
					textureAtlas->getTotalQuads(),
					transform,
					flags);
				renderer->addCommand(&m_maskQuadCommand);
			}
			else if ((flags & FLAGS_SILHOUETTE_RENDER_EFFECT) != 0)
			{
				m_effectQuadCommand.init(_globalZOrder,
					texture, 
					sprite->getEffectGLProgramState(),
					_blendFunc,
					textureAtlas->getQuads(),
					textureAtlas->getTotalQuads(),
					transform,
					flags);
				renderer->addCommand(&m_effectQuadCommand);
			}
			else
			{
				_quadCommand.init(_globalZOrder,
					texture, 
					getGLProgramState(),
					_blendFunc,
					textureAtlas->getQuads(),
					textureAtlas->getTotalQuads(),
					transform,
					flags);
				renderer->addCommand(&_quadCommand);
			}
		}
		else
		{
			_customCommand.init(_globalZOrder, transform, flags);
			_customCommand.func = CC_CALLBACK_0(SILLabel::onDraw, this, transform, transformUpdated);

			renderer->addCommand(&_customCommand);
		}
	}
}

void SILLabel::setFontAtlas(FontAtlas* atlas, bool distanceFieldEnabled, bool useA8Shader)
{
	if (atlas)
	{
		_systemFontDirty = false;
	}

	if (atlas == _fontAtlas)
		return;

	CC_SAFE_RETAIN(atlas);
	if (_fontAtlas)
	{
		_batchNodes.clear();
		FontAtlasCache::releaseFontAtlas(_fontAtlas);
	}
	_fontAtlas = atlas;

	if (_reusedLetter == nullptr)
	{
		SILSprite* sprite = SILSprite::create();
		sprite->setOpacityModifyRGB(_isOpacityModifyRGB);
		sprite->retain();
		sprite->setAnchorPoint(Vec2::ANCHOR_TOP_LEFT);

		sprite->setEffectOpacity(m_effectOpacity);
		sprite->setScreenTexture(m_screenTexture);
		sprite->setSilhouetted(m_isSilhouetted);

		_reusedLetter = sprite;
	}

	if (_fontAtlas)
	{
		_lineHeight = _fontAtlas->getLineHeight();
		_contentDirty = true;
		_systemFontDirty = false;
	}
	_useDistanceField = distanceFieldEnabled;
	_useA8Shader = useA8Shader;

	if (_currentLabelType != LabelType::TTF)
	{
		_currLabelEffect = LabelEffect::NORMAL;
		updateShaderProgram();
	}
}

void SILLabel::createSpriteForSystemFont(FontDefinition const& fontDef)
{
	_currentLabelType = LabelType::STRING_TEXTURE;

	auto texture = new (std::nothrow) Texture2D;
	texture->initWithString(_utf8Text.c_str(), fontDef);

	SILSprite* sprite = SILSprite::createWithTexture(texture);
	//set camera mask using label's camera mask, because _textSprite may be null when setting camera mask to label
	sprite->setCameraMask(getCameraMask());
	sprite->setGlobalZOrder(getGlobalZOrder());
	sprite->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
	this->setContentSize(sprite->getContentSize());
	texture->release();
	if (_blendFuncDirty)
	{
		sprite->setBlendFunc(_blendFunc);
	}

	sprite->retain();
	sprite->updateDisplayedColor(_displayedColor);
	sprite->updateDisplayedOpacity(_displayedOpacity);

	sprite->setEffectOpacity(m_effectOpacity);
	sprite->setScreenTexture(m_screenTexture);
	sprite->setSilhouetted(m_isSilhouetted);

	_textSprite = sprite;
}

void SILLabel::createShadowSpriteForSystemFont(FontDefinition const& fontDef)
{
	SILSprite* sprite = nullptr;
	if (!fontDef._stroke._strokeEnabled && fontDef._fontFillColor == _shadowColor3B
		&& (fontDef._fontAlpha == _shadowOpacity))
	{
		sprite = SILSprite::createWithTexture(_textSprite->getTexture());
	}
	else
	{
		FontDefinition shadowFontDefinition = fontDef;
		shadowFontDefinition._fontFillColor.r = _shadowColor3B.r;
		shadowFontDefinition._fontFillColor.g = _shadowColor3B.g;
		shadowFontDefinition._fontFillColor.b = _shadowColor3B.b;
		shadowFontDefinition._fontAlpha = _shadowOpacity;

		shadowFontDefinition._stroke._strokeColor = shadowFontDefinition._fontFillColor;
		shadowFontDefinition._stroke._strokeAlpha = shadowFontDefinition._fontAlpha;

		auto texture = new (std::nothrow) Texture2D;
		texture->initWithString(_utf8Text.c_str(), shadowFontDefinition);
		sprite = SILSprite::createWithTexture(texture);
		texture->release();
	}

	if (sprite)
	{
		if (_blendFuncDirty)
		{
			sprite->setBlendFunc(_blendFunc);
		}
		sprite->setCameraMask(getCameraMask());
		sprite->setGlobalZOrder(getGlobalZOrder());
		sprite->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
		sprite->setPosition(_shadowOffset.width, _shadowOffset.height);

		sprite->retain();
		sprite->updateDisplayedColor(_displayedColor);
		sprite->updateDisplayedOpacity(_displayedOpacity);

		sprite->setEffectOpacity(m_effectOpacity);
		sprite->setScreenTexture(m_screenTexture);
		sprite->setSilhouetted(m_isSilhouetted);

		_shadowNode = sprite;
	}
}

NS_END