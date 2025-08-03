//
// Created by Luthier on 2019/9/24.
//

#include "GLProgramManager.h"

#include "Shaders.h"

NS_BEGIN

const char* SHADER_NAME_LIGHTNESS = "ShaderLightness";
const char* SHADER_NAME_SILHOUETTABLE_NODE_MASK = "ShaderSilhouettableNodeMask";
const char* SHADER_NAME_SILHOUETTABLE_NODE_MASK_NO_MVP = "ShaderSilhouettableNodeMask_NoMVP";
const char* SHADER_NAME_SILHOUETTABLE_NODE_EFFECT = "ShaderSilhouettableNodeEffect";
const char* SHADER_NAME_SILHOUETTABLE_NODE_EFFECT_NO_MVP = "ShaderSilhouettableNodeEffect_NoMVP";
const char* SHADER_NAME_DISCARD_ALPHA = "ShaderDiscardAlpha";

const char* UNIFORM_NAME_MASK_COLOR = "u_maskColor";
const char* UNIFORM_NAME_SOURCE_MASK_COLOR = "u_sourceMaskColor";
const char* UNIFORM_NAME_ALPHA_VALUE = "u_alphaValue";
const char* UNIFORM_NAME_PREMULTIPLIED_ALPHA = "u_premultipliedAlpha";
const char* UNIFORM_NAME_SCREEN_PIXEL_SIZE = "u_screenPixelSize";
const char* UNIFORM_NAME_SCREEN_TEXTURE = "u_screenTexture";

enum ShaderType
{
	SHADERTYPE_LIGHTNESS,
	SHADERTYPE_SILHOUETTABLE_NODE_MASK,
	SHADERTYPE_SILHOUETTABLE_NODE_MASK_NO_MVP,
	SHADERTYPE_SILHOUETTABLE_NODE_EFFECT,
	SHADERTYPE_SILHOUETTABLE_NODE_EFFECT_NO_MVP,
	SHADERTYPE_DISCARD_ALPHA,
    SHADERTYPE_MAX,
};

static GLProgramManager* s_instance = nullptr;

GLProgramManager::GLProgramManager() :
    m_backToForegroundListener(nullptr)
{

}

GLProgramManager::~GLProgramManager()
{
    if(m_backToForegroundListener)
        Director::getInstance()->getEventDispatcher()->removeEventListener(m_backToForegroundListener);

    for(auto it = m_programs.begin(); it != m_programs.end();)
    {
        auto p = (*it).second;
        it = m_programs.erase(it);
        p->release();
    }

    CCLOGINFO("deallocing GLProgramManager: %p", this);
}

GLProgramManager* GLProgramManager::getInstance()
{
    if (!s_instance) {
        s_instance = new (std::nothrow) GLProgramManager();
        if (!s_instance->init())
        {
            CC_SAFE_DELETE(s_instance);
        }
    }
    return s_instance;
}

void GLProgramManager::destroyInstance()
{
    CC_SAFE_RELEASE_NULL(s_instance);
}

bool GLProgramManager::init()
{
#if CC_ENABLE_CACHE_TEXTURE_DATA
    m_backToForegroundListener = EventListenerCustom::create(EVENT_RENDERER_RECREATED, [this](EventCustom*)
    {
        reloadGLPrograms();
    });
    Director::getInstance()->getEventDispatcher()->addEventListenerWithFixedPriority(m_backToForegroundListener, -1);
#endif
    return true;
}

void GLProgramManager::loadGLPrograms()
{
    GLProgram* program = nullptr;
    
    program = new (std::nothrow) GLProgram();
    this->loadGLProgram(program, SHADERTYPE_LIGHTNESS);
    m_programs.emplace(SHADER_NAME_LIGHTNESS, program);
    
    program = new (std::nothrow) GLProgram();
    this->loadGLProgram(program, SHADERTYPE_SILHOUETTABLE_NODE_MASK);
	m_programs.emplace(SHADER_NAME_SILHOUETTABLE_NODE_MASK, program);

	program = new (std::nothrow) GLProgram();
	this->loadGLProgram(program, SHADERTYPE_SILHOUETTABLE_NODE_MASK_NO_MVP);
	m_programs.emplace(SHADER_NAME_SILHOUETTABLE_NODE_MASK_NO_MVP, program);

	program = new (std::nothrow) GLProgram();
	this->loadGLProgram(program, SHADERTYPE_SILHOUETTABLE_NODE_EFFECT);
    m_programs.emplace(SHADER_NAME_SILHOUETTABLE_NODE_EFFECT, program);

	program = new (std::nothrow) GLProgram();
	this->loadGLProgram(program, SHADERTYPE_SILHOUETTABLE_NODE_EFFECT_NO_MVP);
	m_programs.emplace(SHADER_NAME_SILHOUETTABLE_NODE_EFFECT_NO_MVP, program);

	program = new (std::nothrow) GLProgram();
	this->loadGLProgram(program, SHADERTYPE_DISCARD_ALPHA);
	m_programs.emplace(SHADER_NAME_DISCARD_ALPHA, program);

}

GLProgram* GLProgramManager::getGLProgram(std::string const& key)
{
    auto it = m_programs.find(key);
    if( it != m_programs.end() )
        return it->second;
    return nullptr;
}

void GLProgramManager::reloadGLPrograms()
{
    GLProgram* program = nullptr;

    program = this->getGLProgram(SHADER_NAME_LIGHTNESS);
    if(program)
    {
        program->reset();
        this->loadGLProgram(program, SHADERTYPE_LIGHTNESS);
    }
    
    program = this->getGLProgram(SHADER_NAME_SILHOUETTABLE_NODE_MASK);
    if(program)
    {
        program->reset();
        this->loadGLProgram(program, SHADERTYPE_SILHOUETTABLE_NODE_MASK);
    }

	program = this->getGLProgram(SHADER_NAME_SILHOUETTABLE_NODE_MASK_NO_MVP);
	if (program)
	{
		program->reset();
		this->loadGLProgram(program, SHADERTYPE_SILHOUETTABLE_NODE_MASK_NO_MVP);
	}

	program = this->getGLProgram(SHADER_NAME_SILHOUETTABLE_NODE_EFFECT);
	if (program)
	{
		program->reset();
		this->loadGLProgram(program, SHADERTYPE_SILHOUETTABLE_NODE_EFFECT);
	}

	program = this->getGLProgram(SHADER_NAME_SILHOUETTABLE_NODE_EFFECT_NO_MVP);
	if (program)
	{
		program->reset();
		this->loadGLProgram(program, SHADERTYPE_SILHOUETTABLE_NODE_EFFECT_NO_MVP);
	}

	program = this->getGLProgram(SHADER_NAME_DISCARD_ALPHA);
	if (program)
	{
		program->reset();
		this->loadGLProgram(program, SHADERTYPE_DISCARD_ALPHA);
	}
}

void GLProgramManager::loadGLProgram(GLProgram* program, int type)
{
    switch (type)
    {
        case SHADERTYPE_LIGHTNESS:
            program->initWithByteArrays(Lightness_vert, Lightness_frag);
            break;
        case SHADERTYPE_SILHOUETTABLE_NODE_MASK:
			program->initWithByteArrays(SilhouettableNodeMask_vert, SilhouettableNodeMask_frag);
            break;
		case SHADERTYPE_SILHOUETTABLE_NODE_MASK_NO_MVP:
			program->initWithByteArrays(SilhouettableNodeMask_noMVP_vert, SilhouettableNodeMask_frag);
			break;
		case SHADERTYPE_SILHOUETTABLE_NODE_EFFECT:
			program->initWithByteArrays(SilhouettableNodeEffect_vert, SilhouettableNodeEffect_frag);
			break;
		case SHADERTYPE_SILHOUETTABLE_NODE_EFFECT_NO_MVP:
			program->initWithByteArrays(SilhouettableNodeEffect_noMVP_vert, SilhouettableNodeEffect_frag);
			break;
		case SHADERTYPE_DISCARD_ALPHA:
			program->initWithByteArrays(DiscardAlpha_vert, DiscardAlpha_frag);
			break;
        default:
            CCLOG("GLProgramManager: %s:%d, error GLProgram type: %d", __FUNCTION__, __LINE__, type);
            return;
    }

    program->link();
    program->updateUniforms();

    CHECK_GL_ERROR_DEBUG();
}

NS_END