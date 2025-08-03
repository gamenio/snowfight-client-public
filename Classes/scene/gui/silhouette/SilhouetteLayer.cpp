#include "SilhouetteLayer.h"

#include "SILSprite.h"
#include "scene/GLProgramManager.h"
#include "SILCommon.h"

NS_BEGIN

#define DEBUG_OFFSCREEN_TEXTURE				0

SilhouetteLayer* SilhouetteLayer::create()
{
	SilhouetteLayer *ret = new (std::nothrow) SilhouetteLayer();
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

bool SilhouetteLayer::init()
{
	if (!Layer::init())
		return false;

	Director* director = Director::getInstance();
	Size const& winSize = director->getWinSize();

	m_offscreenRender = RenderTexture::create(winSize.width, winSize.height, Texture2D::PixelFormat::RGBA8888, GL_DEPTH24_STENCIL8);
	CC_SAFE_RETAIN(m_offscreenRender);
	Sprite* sprite = m_offscreenRender->getSprite();
#if DEBUG_OFFSCREEN_TEXTURE
	sprite->setAnchorPoint(Point::ANCHOR_TOP_RIGHT);
	sprite->setPosition(winSize.width, winSize.height);
	sprite->setScale(0.4f);
#else
	sprite->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
#endif // DEBUG_OFFSCREEN_TEXTURE

	return true;
}

SilhouetteLayer::SilhouetteLayer() :
	m_minGlobalZOrder(0.f),
	m_maxGlobalZOrder(0.f),
	m_offscreenRender(nullptr)
{
}

SilhouetteLayer::~SilhouetteLayer()
{
	CC_SAFE_RELEASE_NULL(m_offscreenRender);
}

void SilhouetteLayer::setCovers(Vector<Node*> const& covers)
{
	m_covers = covers;
}

void SilhouetteLayer::setGlobalZOrderRange(float min, float max)
{
	CC_ASSERT(min <= max);
	m_minGlobalZOrder = min;
	m_maxGlobalZOrder = max;
}

void SilhouetteLayer::onEnter()
{
	Node::onEnter();
}

void SilhouetteLayer::onEnterTransitionDidFinish()
{
	Node::onEnterTransitionDidFinish();
}

void SilhouetteLayer::onExit()
{
	Node::onExit();
}

void SilhouetteLayer::onExitTransitionDidStart()
{
	Node::onExitTransitionDidStart();
}

void SilhouetteLayer::addChild(Node* child)
{
	Layer::addChild(child);

	this->setScreenTextureRecursively(child, m_offscreenRender->getSprite()->getTexture());
}

void SilhouetteLayer::visit(Renderer* renderer, Mat4 const& parentTransform, uint32_t parentFlags)
{
	if (!_visible)
		return;

	uint32_t flags = processParentFlags(parentTransform, parentFlags);

	// IMPORTANT:
	// To ease the migration to v3.0, we still support the Mat4 stack,
	// but it is deprecated and your code should not rely on it
	Director* director = Director::getInstance();
	CCASSERT(nullptr != director, "Director is null when setting matrix stack");
	director->pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
	director->loadMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW, _modelViewTransform);

	if (!_children.empty())
		sortAllChildren();

	this->visitInternal(renderer, flags);

	flags &= ~FLAGS_DIRTY_MASK;
	flags |= FLAGS_SILHOUETTE_RENDER_MASK;

	m_offscreenRender->setGlobalZOrder(m_minGlobalZOrder);
	m_offscreenRender->beginWithClear(0.0f, 0.0f, 0.0f, 1.0f, 0.f, 0);

	this->visitInternal(renderer, flags);

	if (!m_covers.empty())
	{
		for (auto it = m_covers.begin(); it != m_covers.end(); ++it)
		{
			Node* node = *it;
			node->visit(renderer, Mat4::IDENTITY, flags);
		}
	}

	m_offscreenRender->setGlobalZOrder(m_maxGlobalZOrder);
	m_offscreenRender->end();

	flags &= ~FLAGS_SILHOUETTE_RENDER_MASK;
	flags |= FLAGS_SILHOUETTE_RENDER_EFFECT;

	m_groupCommand.init(m_maxGlobalZOrder);
	renderer->addCommand(&m_groupCommand);
	renderer->pushGroup(m_groupCommand.getRenderQueueID());

	this->visitInternal(renderer, flags);
	renderer->popGroup();

#if DEBUG_OFFSCREEN_TEXTURE
	m_offscreenRender->visit(renderer, Mat4::IDENTITY, 0);
#endif // DEBUG_OFFSCREEN_TEXTURE

	director->popMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
}

void SilhouetteLayer::setScreenTextureRecursively(Node* node, Texture2D* texture)
{
	SilhouettableProtocol* silNode = dynamic_cast<SilhouettableProtocol*>(node);
	if (silNode)
		silNode->setScreenTexture(texture);

	auto& children = node->getChildren();
	for (auto* child : children)
	{
		setScreenTextureRecursively(child, texture);
	}
}

void SilhouetteLayer::visitInternal(Renderer* renderer, uint32_t flags)
{
	bool visibleByCamera = isVisitableByVisitingCamera();
	if (!_children.empty())
	{
		int i = 0;
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
}

NS_END
