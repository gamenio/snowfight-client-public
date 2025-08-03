#include "SILClippingNode.h"

#include "base/CCStencilStateManager.h"
#include "SILCommon.h"

NS_BEGIN

SILClippingNode* SILClippingNode::create()
{
	SILClippingNode* ret = new (std::nothrow) SILClippingNode();
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

SILClippingNode* SILClippingNode::create(Node* stencil)
{
	SILClippingNode* ret = new (std::nothrow) SILClippingNode();
	if (ret && ret->init(stencil))
	{
		ret->autorelease();
	}
	else
	{
		CC_SAFE_DELETE(ret);
	}

	return ret;
}

SILClippingNode::SILClippingNode() :
	m_isSilhouetted(true)
{
}

SILClippingNode::~SILClippingNode()
{
}

bool SILClippingNode::init()
{
	return init(nullptr);
}

bool SILClippingNode::init(Node* stencil)
{
	if (!ClippingNode::init(stencil))
		return false;

	return true;
}

void SILClippingNode::setSilhouetted(bool silhouetted)
{
	m_isSilhouetted = silhouetted;
}

void SILClippingNode::visit(Renderer* renderer, Mat4 const& parentTransform, uint32_t parentFlags)
{
	if (!_visible || !hasContent())
		return;

	if (!m_isSilhouetted && (parentFlags & FLAGS_RENDER_SILHOUETTE) != 0)
		return;

	uint32_t flags = processParentFlags(parentTransform, parentFlags);

	// IMPORTANT:
	// To ease the migration to v3.0, we still support the Mat4 stack,
	// but it is deprecated and your code should not rely on it
	Director* director = Director::getInstance();
	CCASSERT(nullptr != director, "Director is null when setting matrix stack");
	director->pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
	director->loadMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW, _modelViewTransform);

	//Add group command
	if ((flags & FLAGS_SILHOUETTE_RENDER_MASK) != 0)
	{
		m_maskGroupCommand.init(_globalZOrder);
		renderer->addCommand(&m_maskGroupCommand);
		renderer->pushGroup(m_maskGroupCommand.getRenderQueueID());
	}
	else if ((flags & FLAGS_SILHOUETTE_RENDER_EFFECT) != 0)
	{
		m_effectGroupCommand.init(_globalZOrder);
		renderer->addCommand(&m_effectGroupCommand);
		renderer->pushGroup(m_effectGroupCommand.getRenderQueueID());
	}
	else
	{
		_groupCommand.init(_globalZOrder);
		renderer->addCommand(&_groupCommand);
		renderer->pushGroup(_groupCommand.getRenderQueueID());
	}

	_beforeVisitCmd.init(_globalZOrder);
	_beforeVisitCmd.func = CC_CALLBACK_0(StencilStateManager::onBeforeVisit, _stencilStateManager);
	renderer->addCommand(&_beforeVisitCmd);

	auto alphaThreshold = this->getAlphaThreshold();
	if (alphaThreshold < 1)
	{
#if CC_CLIPPING_NODE_OPENGLES
		// since glAlphaTest do not exists in OES, use a shader that writes
		// pixel only if greater than an alpha threshold
		GLProgram *program = GLProgramCache::getInstance()->getGLProgram(GLProgram::SHADER_NAME_POSITION_TEXTURE_ALPHA_TEST_NO_MV);
		GLint alphaValueLocation = glGetUniformLocation(program->getProgram(), GLProgram::UNIFORM_NAME_ALPHA_TEST_VALUE);
		// set our alphaThreshold
		program->use();
		program->setUniformLocationWith1f(alphaValueLocation, alphaThreshold);
		// we need to recursively apply this shader to all the nodes in the stencil node
		// FIXME: we should have a way to apply shader to all nodes without having to do this
		setProgram(_stencil, program);
#endif

	}
	_stencil->visit(renderer, _modelViewTransform, flags & ~FLAGS_RENDER_SILHOUETTE);

	_afterDrawStencilCmd.init(_globalZOrder);
	_afterDrawStencilCmd.func = CC_CALLBACK_0(StencilStateManager::onAfterDrawStencil, _stencilStateManager);
	renderer->addCommand(&_afterDrawStencilCmd);

	int i = 0;
	bool visibleByCamera = isVisitableByVisitingCamera();

	if (!_children.empty())
	{
		sortAllChildren();
		// draw children zOrder < 0
		for (auto size = _children.size(); i < size; ++i)
		{
			auto node = _children.at(i);

			if (node && node->getLocalZOrder() < 0)
				node->visit(renderer, _modelViewTransform, flags);
			else
				break;
		}
		// self draw
		if (visibleByCamera)
			this->draw(renderer, _modelViewTransform, flags);

		for (auto it = _children.cbegin() + i, itCend = _children.cend(); it != itCend; ++it)
			(*it)->visit(renderer, _modelViewTransform, flags);
	}
	else if (visibleByCamera)
	{
		this->draw(renderer, _modelViewTransform, flags);
	}

	_afterVisitCmd.init(_globalZOrder);
	_afterVisitCmd.func = CC_CALLBACK_0(StencilStateManager::onAfterVisit, _stencilStateManager);
	renderer->addCommand(&_afterVisitCmd);

	renderer->popGroup();

	director->popMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
}

NS_END