#include "SILProgressTimer.h"

#include "scene/GLProgramManager.h"
#include "SILCommon.h"

NS_BEGIN

SILProgressTimer* SILProgressTimer::create(Sprite* sp)
{
	SILProgressTimer* progressTimer = new (std::nothrow) SILProgressTimer();
	if (progressTimer && progressTimer->initWithSprite(sp))
	{
		progressTimer->autorelease();
		return progressTimer;
	}

	delete progressTimer;
	return nullptr;
}

SILProgressTimer::SILProgressTimer() :
	m_isSilhouetted(true),
	m_effectOpacity(SILHOUETTE_EFFECT_OPACITY_DEFAULT),
	m_screenTexture(nullptr),
	m_maskGLProgramState(nullptr),
	m_effectGLProgramState(nullptr)
{
}

SILProgressTimer::~SILProgressTimer()
{
	CC_SAFE_RELEASE_NULL(m_maskGLProgramState);
	CC_SAFE_RELEASE_NULL(m_effectGLProgramState);
	CC_SAFE_RELEASE_NULL(m_screenTexture);
}

bool SILProgressTimer::initWithSprite(Sprite* sp)
{
	if (!ProgressTimer::initWithSprite(sp))
		return false;

	GLProgram* glProgram = nullptr;
	glProgram = GLProgramManager::getInstance()->getGLProgram(SHADER_NAME_SILHOUETTABLE_NODE_MASK);
	m_maskGLProgramState = GLProgramState::create(glProgram);
	CC_SAFE_RETAIN(m_maskGLProgramState);
	m_maskGLProgramState->setUniformVec3(UNIFORM_NAME_MASK_COLOR, SILHOUETTE_NODE_MASK_COLOR);
	m_maskGLProgramState->setNodeBinding(this);

	glProgram = GLProgramManager::getInstance()->getGLProgram(SHADER_NAME_SILHOUETTABLE_NODE_EFFECT);
	m_effectGLProgramState = GLProgramState::create(glProgram);
	CC_SAFE_RETAIN(m_effectGLProgramState);
	m_effectGLProgramState->setUniformVec3(UNIFORM_NAME_SOURCE_MASK_COLOR, SILHOUETTE_NODE_MASK_COLOR);
	m_effectGLProgramState->setUniformFloat(UNIFORM_NAME_ALPHA_VALUE, m_effectOpacity / 255.0f);
	m_effectGLProgramState->setUniformVec2(UNIFORM_NAME_SCREEN_PIXEL_SIZE, getScreenPixelSize());

	if (m_screenTexture)
		m_effectGLProgramState->setUniformTexture(UNIFORM_NAME_SCREEN_TEXTURE, m_screenTexture);
	if (_sprite->getTexture())
		m_effectGLProgramState->setUniformInt(UNIFORM_NAME_PREMULTIPLIED_ALPHA, _sprite->getTexture()->hasPremultipliedAlpha());
	m_effectGLProgramState->setNodeBinding(this);

	return true;
}

void SILProgressTimer::setSilhouetted(bool silhouetted)
{
	m_isSilhouetted = silhouetted;
}

void SILProgressTimer::setEffectOpacity(GLubyte opacity)
{
	if (m_effectOpacity != opacity)
	{
		m_effectOpacity = opacity;
		if (m_effectGLProgramState)
			m_effectGLProgramState->setUniformFloat(UNIFORM_NAME_ALPHA_VALUE, opacity / 255.0f);
	}
}

void SILProgressTimer::setScreenTexture(Texture2D* texture)
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

void SILProgressTimer::visit(Renderer* renderer, Mat4 const& parentTransform, uint32_t parentFlags)
{
	if (!_visible)
		return;

	if (!m_isSilhouetted && (parentFlags & FLAGS_RENDER_SILHOUETTE) != 0)
		return;

	ProgressTimer::visit(renderer, parentTransform, parentFlags);
}

void SILProgressTimer::draw(Renderer* renderer, Mat4 const& transform, uint32_t flags)
{
	if (!_vertexData || !_sprite)
		return;

	if ((flags & FLAGS_SILHOUETTE_RENDER_MASK) != 0)
	{
		m_maskCustomCommand.init(_globalZOrder, transform, flags);
		m_maskCustomCommand.func = CC_CALLBACK_0(SILProgressTimer::onDraw, this, transform, flags);
		renderer->addCommand(&m_maskCustomCommand);
	}
	else if ((flags & FLAGS_SILHOUETTE_RENDER_EFFECT) != 0)
	{
		m_effectCustomCommand.init(_globalZOrder, transform, flags);
		m_effectCustomCommand.func = CC_CALLBACK_0(SILProgressTimer::onDraw, this, transform, flags);
		renderer->addCommand(&m_effectCustomCommand);
	}
	else
	{
		_customCommand.init(_globalZOrder, transform, flags);
		_customCommand.func = CC_CALLBACK_0(SILProgressTimer::onDraw, this, transform, flags);
		renderer->addCommand(&_customCommand);
	}
}

void SILProgressTimer::onDraw(Mat4 const& transform, uint32_t flags)
{
	if ((flags & FLAGS_SILHOUETTE_RENDER_MASK) != 0)
		m_maskGLProgramState->apply(transform);
	else if ((flags & FLAGS_SILHOUETTE_RENDER_EFFECT) != 0)
		m_effectGLProgramState->apply(transform);
	else
		this->getGLProgramState()->apply(transform);

	GL::blendFunc(_sprite->getBlendFunc().src, _sprite->getBlendFunc().dst);

	GL::enableVertexAttribs(GL::VERTEX_ATTRIB_FLAG_POS_COLOR_TEX);

	GL::bindTexture2D(_sprite->getTexture());

	glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(_vertexData[0]), &_vertexData[0].vertices);
	glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_TEX_COORD, 2, GL_FLOAT, GL_FALSE, sizeof(_vertexData[0]), &_vertexData[0].texCoords);
	glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(_vertexData[0]), &_vertexData[0].colors);

	if (_type == Type::RADIAL)
	{
		glDrawArrays(GL_TRIANGLE_FAN, 0, _vertexDataCount);
		CC_INCREMENT_GL_DRAWN_BATCHES_AND_VERTICES(1, _vertexDataCount);
	}
	else if (_type == Type::BAR)
	{
		if (!_reverseDirection)
		{
			glDrawArrays(GL_TRIANGLE_STRIP, 0, _vertexDataCount);
			CC_INCREMENT_GL_DRAWN_BATCHES_AND_VERTICES(1, _vertexDataCount);
		}
		else
		{
			glDrawArrays(GL_TRIANGLE_STRIP, 0, _vertexDataCount / 2);
			glDrawArrays(GL_TRIANGLE_STRIP, 4, _vertexDataCount / 2);
			// 2 draw calls
			CC_INCREMENT_GL_DRAWN_BATCHES_AND_VERTICES(2, _vertexDataCount);
		}
	}
}

NS_END