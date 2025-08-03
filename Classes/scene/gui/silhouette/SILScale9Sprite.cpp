#include "SILScale9Sprite.h"

#include "scene/GLProgramManager.h"
#include "SILCommon.h"

NS_BEGIN

SILScale9Sprite* SILScale9Sprite::create()
{
	SILScale9Sprite* sprite = new (std::nothrow) SILScale9Sprite();
	if (sprite && sprite->init())
	{
		sprite->autorelease();
		return sprite;
	}
	CC_SAFE_DELETE(sprite);
	return nullptr;
}

SILScale9Sprite* SILScale9Sprite::create(std::string const& filename)
{
	return create(Rect::ZERO, filename);
}

SILScale9Sprite* SILScale9Sprite::create(std::string const& filename, Rect const& rect, Rect const& capInsets)
{
	SILScale9Sprite* sprite = new (std::nothrow) SILScale9Sprite();
	if (sprite && sprite->initWithFile(filename, rect, capInsets))
	{
		sprite->autorelease();
		return sprite;
	}
	CC_SAFE_DELETE(sprite);
	return nullptr;
}

SILScale9Sprite* SILScale9Sprite::create(Rect const& capInsets, std::string const& filename)
{
	SILScale9Sprite* ret = new (std::nothrow) SILScale9Sprite();
	if (ret && ret->initWithFile(capInsets, filename))
	{
		ret->autorelease();
		return ret;
	}
	CC_SAFE_DELETE(ret);
	return nullptr;
}

SILScale9Sprite* SILScale9Sprite::create(std::string const& filename, Rect const& rect)
{
	return create(filename, rect, Rect::ZERO);
}

SILScale9Sprite* SILScale9Sprite::createWithSpriteFrame(SpriteFrame* spriteFrame)
{
	return createWithSpriteFrame(spriteFrame, Rect::ZERO);
}

SILScale9Sprite* SILScale9Sprite::createWithSpriteFrame(SpriteFrame* spriteFrame, Rect const& capInsets)
{
	SILScale9Sprite* ret = new (std::nothrow) SILScale9Sprite();
	if (ret && ret->initWithSpriteFrame(spriteFrame, capInsets))
	{
		ret->autorelease();
		return ret;
	}
	CC_SAFE_DELETE(ret);
	return nullptr;
}

SILScale9Sprite* SILScale9Sprite::createWithSpriteFrameName(std::string const& spriteFrameName)
{
	SILScale9Sprite* ret = new (std::nothrow) SILScale9Sprite();
	if (ret && ret->initWithSpriteFrameName(spriteFrameName, Rect::ZERO))
	{
		ret->autorelease();
		return ret;
	}
	CC_SAFE_DELETE(ret);

	log("Could not allocate Scale9Sprite()");
	return nullptr;
}

SILScale9Sprite* SILScale9Sprite::createWithSpriteFrameName(std::string const& spriteFrameName, Rect const& capInsets)
{
	SILScale9Sprite* ret = new (std::nothrow) SILScale9Sprite();
	if (ret && ret->initWithSpriteFrameName(spriteFrameName, capInsets))
	{
		ret->autorelease();
		return ret;
	}
	CC_SAFE_DELETE(ret);
	return nullptr;
}

SILScale9Sprite::SILScale9Sprite() :
	m_isSilhouetted(true),
	m_screenTexture(nullptr),
	m_effectOpacity(SILHOUETTE_EFFECT_OPACITY_DEFAULT),
	m_effectGLProgramState(nullptr)
{
}

SILScale9Sprite::~SILScale9Sprite()
{
	CC_SAFE_RELEASE_NULL(m_screenTexture);
	CC_SAFE_RELEASE_NULL(m_maskGLProgramState);
	CC_SAFE_RELEASE_NULL(m_effectGLProgramState);
}

bool SILScale9Sprite::initWithTexture(Texture2D* texture, Rect const& rect, bool rotated)
{
	if (!Scale9Sprite::initWithTexture(texture, rect, rotated))
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

void SILScale9Sprite::setTexture(Texture2D* texture)
{
	Scale9Sprite::setTexture(texture);
	if (texture)
	{
		if(m_effectGLProgramState)
			m_effectGLProgramState->setUniformInt(UNIFORM_NAME_PREMULTIPLIED_ALPHA, texture->hasPremultipliedAlpha());
	}
}

void SILScale9Sprite::setSilhouetted(bool silhouetted)
{
	m_isSilhouetted = silhouetted;
}


void SILScale9Sprite::setEffectOpacity(GLubyte opacity)
{
	if (m_effectOpacity != opacity)
	{
		m_effectOpacity = opacity;
		if(m_effectGLProgramState)
			m_effectGLProgramState->setUniformFloat(UNIFORM_NAME_ALPHA_VALUE, opacity / 255.0f);
	}
}

void SILScale9Sprite::setScreenTexture(Texture2D* texture)
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

void SILScale9Sprite::visit(Renderer* renderer, Mat4 const& parentTransform, uint32_t parentFlags)
{
	if (!_visible)
		return;

	if (!m_isSilhouetted && (parentFlags & FLAGS_RENDER_SILHOUETTE) != 0)
		return;

	Sprite::visit(renderer, parentTransform, parentFlags);
}

void SILScale9Sprite::draw(Renderer* renderer, Mat4 const& transform, uint32_t flags)
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