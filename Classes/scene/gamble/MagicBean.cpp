#include "MagicBean.h"

#include "game/World.h"
#include "scene/Utils.h"
#include "scene/AssetsLoader.h"

NS_BEGIN

#define FRAMENAME_MAGIC_BEAN				"magic_bean.png"

MagicBean* MagicBean::create()
{
	MagicBean* pRet = new MagicBean();
	if (pRet && pRet->init())
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

MagicBean::MagicBean() :
	m_isLayoutDirty(false),
	m_iconSp(nullptr),
	m_countLabel(nullptr)
{
}

MagicBean::~MagicBean()
{
	m_iconSp = nullptr;
	m_countLabel = nullptr;
}

bool MagicBean::init()
{
	if (!Node::init())
		return false;

	this->setIgnoreAnchorPointForPosition(false);

	m_iconSp = SILSprite::createWithSpriteFrameName(FRAMENAME_MAGIC_BEAN);
	m_iconSp->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
	this->addChild(m_iconSp);

	m_countLabel = SILLabel::createWithBMFont(BMFONT_PIXCELSTYLE, "0");
	m_countLabel->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
	m_countLabel->setColor(Color3B(239, 113, 0));
	m_countLabel->setBMFontSize(9);
	m_countLabel->setAdditionalKerning(4.0f);
	this->addChild(m_countLabel);

	m_isLayoutDirty = true;

	return true;
}

void MagicBean::setCount(int32 count)
{
	m_countLabel->setString(std::to_string(count));

	m_isLayoutDirty = true;
}

void MagicBean::setGlobalZOrder(float globalZOrder)
{
	m_iconSp->setGlobalZOrder(globalZOrder);
	m_countLabel->setGlobalZOrder(globalZOrder);

	Node::setGlobalZOrder(globalZOrder);
}

void MagicBean::visit(Renderer* renderer, Mat4 const& parentTransform, uint32_t parentFlags)
{
	if (m_isLayoutDirty)
	{
		this->layoutElements();
		m_isLayoutDirty = false;
	}

	Node::visit(renderer, parentTransform, parentFlags);
}

void MagicBean::layoutElements()
{
	float nextY = 0;
	float maxWidth = 0;

	m_iconSp->setPosition(0, 0);
	m_countLabel->setPosition(m_iconSp->getBoundingBox().getMaxX() + 1, 2);
	maxWidth = m_countLabel->getBoundingBox().getMaxX();
	nextY = m_iconSp->getBoundingBox().getMaxY();

	this->setContentSize(Size(maxWidth, nextY));
}


NS_END
