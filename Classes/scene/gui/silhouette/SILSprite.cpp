#include "SILSprite.h"

#include "scene/GLProgramManager.h"
#include "SILCommon.h"

NS_BEGIN

SILSprite* SILSprite::create()
{
	SILSprite* sprite = new (std::nothrow) SILSprite();
	if (sprite && sprite->init())
	{
		sprite->autorelease();
		return sprite;
	}
	CC_SAFE_DELETE(sprite);
	return nullptr;
}

SILSprite* SILSprite::create(std::string const& filename)
{
	SILSprite* sprite = new (std::nothrow) SILSprite();
	if (sprite && sprite->initWithFile(filename))
	{
		sprite->autorelease();
		return sprite;
	}
	CC_SAFE_DELETE(sprite);
	return nullptr;
}

SILSprite* SILSprite::create(std::string const& filename, Rect const& rect)
{
	SILSprite* sprite = new (std::nothrow) SILSprite();
	if (sprite && sprite->initWithFile(filename, rect))
	{
		sprite->autorelease();
		return sprite;
	}
	CC_SAFE_DELETE(sprite);
	return nullptr;
}

SILSprite* SILSprite::createWithTexture(Texture2D* texture)
{
	SILSprite* sprite = new (std::nothrow) SILSprite();
	if (sprite && sprite->initWithTexture(texture))
	{
		sprite->autorelease();
		return sprite;
	}
	CC_SAFE_DELETE(sprite);
	return nullptr;
}

SILSprite* SILSprite::createWithTexture(Texture2D* texture, Rect const& rect, bool rotated)
{
	SILSprite* sprite = new (std::nothrow) SILSprite();
	if (sprite && sprite->initWithTexture(texture, rect, rotated))
	{
		sprite->autorelease();
		return sprite;
	}
	CC_SAFE_DELETE(sprite);
	return nullptr;
}

SILSprite* SILSprite::createWithSpriteFrame(SpriteFrame* spriteFrame)
{
	SILSprite* sprite = new (std::nothrow) SILSprite();
	if (sprite && spriteFrame && sprite->initWithSpriteFrame(spriteFrame))
	{
		sprite->autorelease();
		return sprite;
	}
	CC_SAFE_DELETE(sprite);
	return nullptr;
}

SILSprite* SILSprite::createWithSpriteFrameName(std::string const& spriteFrameName)
{
	SpriteFrame* frame = SpriteFrameCache::getInstance()->getSpriteFrameByName(spriteFrameName);

#if COCOS2D_DEBUG > 0
	char msg[256] = { 0 };
	snprintf(msg, sizeof(msg), "Invalid spriteFrameName: %s", spriteFrameName.c_str());
	CCASSERT(frame != nullptr, msg);
#endif

	return createWithSpriteFrame(frame);
}

SILSprite::SILSprite() :
	m_isSilhouetted(true),
	m_screenTexture(nullptr),
	m_effectOpacity(SILHOUETTE_EFFECT_OPACITY_DEFAULT),
	m_effectGLProgramState(nullptr),
	m_maskGLProgramState(nullptr)
{
}

SILSprite::~SILSprite()
{
	CC_SAFE_RELEASE_NULL(m_screenTexture);
	CC_SAFE_RELEASE_NULL(m_maskGLProgramState);
	CC_SAFE_RELEASE_NULL(m_effectGLProgramState);
}

bool SILSprite::initWithTexture(Texture2D* texture, Rect const& rect, bool rotated)
{
	if (!Sprite::initWithTexture(texture, rect, rotated))
		return false;

	GLProgram* glProgram = nullptr;

	glProgram = GLProgramManager::getInstance()->getGLProgram(SHADER_NAME_SILHOUETTABLE_NODE_MASK_NO_MVP);
	m_maskGLProgramState = GLProgramState::create(glProgram);
	CC_SAFE_RETAIN(m_maskGLProgramState);
	m_maskGLProgramState->setUniformVec3(UNIFORM_NAME_MASK_COLOR, SILHOUETTE_NODE_MASK_COLOR);
	m_maskGLProgramState->setNodeBinding(this);

	glProgram = GLProgramManager::getInstance()->getGLProgram(SHADER_NAME_SILHOUETTABLE_NODE_EFFECT_NO_MVP);
	m_effectGLProgramState = GLProgramState::create(glProgram);
	CC_SAFE_RETAIN(m_effectGLProgramState);
	m_effectGLProgramState->setUniformVec3(UNIFORM_NAME_SOURCE_MASK_COLOR, SILHOUETTE_NODE_MASK_COLOR);
	m_effectGLProgramState->setUniformFloat(UNIFORM_NAME_ALPHA_VALUE, m_effectOpacity / 255.0f);
	m_effectGLProgramState->setUniformVec2(UNIFORM_NAME_SCREEN_PIXEL_SIZE, getScreenPixelSize());

	if (m_screenTexture)
		m_effectGLProgramState->setUniformTexture(UNIFORM_NAME_SCREEN_TEXTURE, m_screenTexture);
	if (_texture)
		m_effectGLProgramState->setUniformInt(UNIFORM_NAME_PREMULTIPLIED_ALPHA, _texture->hasPremultipliedAlpha());
	m_effectGLProgramState->setNodeBinding(this);

	return true;
}

void SILSprite::setBatchNode(SpriteBatchNode* spriteBatchNode)
{
	Sprite::setBatchNode(spriteBatchNode);
	if (spriteBatchNode)
		m_effectGLProgramState->setUniformInt(UNIFORM_NAME_PREMULTIPLIED_ALPHA, spriteBatchNode->getTexture()->hasPremultipliedAlpha());
}

void SILSprite::setTexture(Texture2D* texture)
{
	Sprite::setTexture(texture);
	if (texture)
	{
		if(m_effectGLProgramState)
			m_effectGLProgramState->setUniformInt(UNIFORM_NAME_PREMULTIPLIED_ALPHA, texture->hasPremultipliedAlpha());
	}
}

void SILSprite::setSilhouetted(bool silhouetted)
{
	m_isSilhouetted = silhouetted;
}


void SILSprite::setEffectOpacity(GLubyte opacity)
{
	if (m_effectOpacity != opacity)
	{
		m_effectOpacity = opacity;
		if(m_effectGLProgramState)
			m_effectGLProgramState->setUniformFloat(UNIFORM_NAME_ALPHA_VALUE, opacity / 255.0f);
	}
}

void SILSprite::setScreenTexture(Texture2D* texture)
{
	if (m_screenTexture != texture)
	{
		CC_SAFE_RETAIN(texture);
		CC_SAFE_RELEASE_NULL(m_screenTexture);
		m_screenTexture = texture;

		if (m_effectGLProgramState)
			m_effectGLProgramState->setUniformTexture(UNIFORM_NAME_SCREEN_TEXTURE, m_screenTexture);
	}
}

void SILSprite::visit(Renderer* renderer, Mat4 const& parentTransform, uint32_t parentFlags)
{
	if (!_visible)
		return;

	if (!m_isSilhouetted && (parentFlags & FLAGS_RENDER_SILHOUETTE) != 0)
		return;

	Sprite::visit(renderer, parentTransform, parentFlags);
}

void SILSprite::draw(Renderer* renderer, Mat4 const& transform, uint32_t flags)
{
	if (_texture == nullptr)
	{
		return;
	}

#if CC_USE_CULLING
	// Don't calculate the culling if the transform was not updated
	auto visitingCamera = Camera::getVisitingCamera();
	auto defaultCamera = Camera::getDefaultCamera();
	if (visitingCamera == nullptr) {
		_insideBounds = true;
	}
	else if (visitingCamera == defaultCamera) {
		_insideBounds = ((flags & FLAGS_TRANSFORM_DIRTY) || visitingCamera->isViewProjectionUpdated()) ? renderer->checkVisibility(transform, _contentSize) : _insideBounds;
	}
	else
	{
		// XXX: this always return true since
		_insideBounds = renderer->checkVisibility(transform, _contentSize);
	}

	if (_insideBounds)
#endif
	{
		if ((flags & FLAGS_SILHOUETTE_RENDER_MASK) != 0)
		{
			m_maskTrianglesCommand.init(_globalZOrder,
				_texture,
				m_maskGLProgramState,
				_blendFunc,
				_polyInfo.triangles,
				transform,
				flags);

			renderer->addCommand(&m_maskTrianglesCommand);
		}
		else if ((flags & FLAGS_SILHOUETTE_RENDER_EFFECT) != 0)
		{
			m_effectTrianglesCommand.init(_globalZOrder,
				_texture,
				m_effectGLProgramState,
				_blendFunc,
				_polyInfo.triangles,
				transform,
				flags);

			renderer->addCommand(&m_effectTrianglesCommand);
		}
		else
		{
			_trianglesCommand.init(_globalZOrder,
				_texture,
				this->getGLProgramState(),
				_blendFunc,
				_polyInfo.triangles,
				transform,
				flags);

			renderer->addCommand(&_trianglesCommand);
		}

#if CC_SPRITE_DEBUG_DRAW
		_debugDrawNode->clear();
		auto count = _polyInfo.triangles.indexCount / 3;
		auto indices = _polyInfo.triangles.indices;
		auto verts = _polyInfo.triangles.verts;
		for (ssize_t i = 0; i < count; i++)
		{
			//draw 3 lines
			Vec3 from = verts[indices[i * 3]].vertices;
			Vec3 to = verts[indices[i * 3 + 1]].vertices;
			_debugDrawNode->drawLine(Vec2(from.x, from.y), Vec2(to.x, to.y), Color4F::WHITE);

			from = verts[indices[i * 3 + 1]].vertices;
			to = verts[indices[i * 3 + 2]].vertices;
			_debugDrawNode->drawLine(Vec2(from.x, from.y), Vec2(to.x, to.y), Color4F::WHITE);

			from = verts[indices[i * 3 + 2]].vertices;
			to = verts[indices[i * 3]].vertices;
			_debugDrawNode->drawLine(Vec2(from.x, from.y), Vec2(to.x, to.y), Color4F::WHITE);
		}
#endif //CC_SPRITE_DEBUG_DRAW
	}
}

NS_END
