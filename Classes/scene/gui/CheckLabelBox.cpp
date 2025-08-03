#include "CheckLabelBox.h"

#include "scene/Utils.h"

NS_BEGIN

// 帧名称
#define	FRAMENAME_CHECKBOX_ON		"checkbox_on.png"
#define	FRAMENAME_CHECKBOX_OFF		"checkbox_off.png"

#define CONTENT_HEIGHT		25

CheckLabelBox* CheckLabelBox::create(std::string const& title)
{
	auto ret = new (std::nothrow) CheckLabelBox();
	if (ret && ret->init(FRAMENAME_CHECKBOX_OFF,
		FRAMENAME_CHECKBOX_OFF,
		FRAMENAME_CHECKBOX_ON,
		FRAMENAME_CHECKBOX_OFF,
		FRAMENAME_CHECKBOX_ON,
		title,
        ui::Widget::TextureResType::PLIST))
	{
		ret->autorelease();
	}
	else
	{
		CC_SAFE_DELETE(ret);
	}

	return ret;
}

CheckLabelBox* CheckLabelBox::create(std::string const& backGround,
	std::string const& cross,
	std::string const& title,
	ui::Widget::TextureResType texType)
{
	auto ret = new (std::nothrow) CheckLabelBox();
	if (ret && ret->init(backGround,
		"",
		cross,
		"",
		"",
		title,
		texType))
	{
		ret->autorelease();
	}
	else
	{
		CC_SAFE_DELETE(ret);
	}

	return ret;
}

CheckLabelBox* CheckLabelBox::create(std::string const& backGround,
	std::string const& backGroundSelected,
	std::string const& cross,
	std::string const& backGroundDisabled,
	std::string const& frontCrossDisabled,
	std::string const& title,
	ui::Widget::TextureResType texType)
{
	auto ret = new (std::nothrow) CheckLabelBox();
	if (ret && ret->init(backGround,
		backGroundSelected,
		cross,
		backGroundDisabled,
		frontCrossDisabled,
		title,
		texType))
	{
		ret->autorelease();
	}
	else
	{
		CC_SAFE_DELETE(ret);
	}

	return ret;
}

bool CheckLabelBox::init(std::string const& backGround,
	std::string const& backGroundSelected,
	std::string const& cross,
	std::string const& backGroundDisabled,
	std::string const& frontCrossDisabled,
	std::string const& title,
	ui::Widget::TextureResType texType)
{
	if (!Node::init())
		return false;

	m_touchListener = EventListenerTouchOneByOne::create();
	CC_SAFE_RETAIN(m_touchListener);
	m_touchListener->setSwallowTouches(true);
	m_touchListener->onTouchBegan = CC_CALLBACK_2(CheckLabelBox::onTouchBegan, this);
	m_touchListener->onTouchMoved = CC_CALLBACK_2(CheckLabelBox::onTouchMoved, this);
	m_touchListener->onTouchEnded = CC_CALLBACK_2(CheckLabelBox::onTouchEnded, this);
	m_touchListener->onTouchCancelled = CC_CALLBACK_2(CheckLabelBox::onTouchCancelled, this);
	getEventDispatcher()->addEventListenerWithSceneGraphPriority(m_touchListener, this);

	m_checkBox = ui::CheckBox::create(backGround,
		backGroundSelected,
		cross,
		backGroundDisabled,
		frontCrossDisabled,
		texType);
	m_checkBox->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
	m_checkBox->ignoreContentAdaptWithSize(false);
	m_checkBox->addEventListener([this](Ref* sender, ui::CheckBox::EventType eventType) {
		if (m_checkLabelBoxCallback)
			m_checkLabelBoxCallback(this, eventType == ui::CheckBox::EventType::SELECTED ? EventType::SELECTED : EventType::UNSELECTED);
	});
	this->addChild(m_checkBox);


	m_label = Label::createWithSystemFont(title, DEFAULT_SYSTEM_FONT, 13);
	m_label->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
	m_label->setTextColor(Color4B(150, 129, 56, 255));
	Utils::enableBoldForLabel(m_label);
	this->addChild(m_label);

	this->layoutElements();

	return true;
}

CheckLabelBox::CheckLabelBox() :
	m_touchListener(nullptr),
	m_hitted(false),
	m_hittedByCamera(nullptr),
	m_checkBox(nullptr),
	m_label(nullptr)
{
}

CheckLabelBox::~CheckLabelBox()
{
	if (m_touchListener)
	{
		getEventDispatcher()->removeEventListener(m_touchListener);
		CC_SAFE_RELEASE_NULL(m_touchListener);
	}

	m_hittedByCamera = nullptr;

	m_checkBox = nullptr;
	m_label = nullptr;
}


bool CheckLabelBox::onTouchBegan(Touch* touch, Event* unusedEvent)
{
	m_hitted = false;
	if (isVisible() && m_checkBox->isEnabled())
	{
		Point touchLoc = touch->getLocation();
		auto camera = Camera::getVisitingCamera();
		if (hitTest(touchLoc, camera, nullptr))
		{
			m_hittedByCamera = camera;
			m_hitted = true;
		}
	}
	return m_hitted;
}

void CheckLabelBox::onTouchMoved(Touch* touch, Event* unusedEvent)
{
	Point touchLoc = touch->getLocation();
	m_hitted = hitTest(touchLoc, m_hittedByCamera, nullptr);
}

void CheckLabelBox::onTouchEnded(Touch* touch, Event* unusedEvent)
{
	if (m_hitted)
	{
		m_checkBox->setSelected(!this->isSelected());
		if (m_checkLabelBoxCallback)
			m_checkLabelBoxCallback(this, this->isSelected() ? EventType::SELECTED : EventType::UNSELECTED);
	}

	m_hitted = false;
}

void CheckLabelBox::onTouchCancelled(Touch* touch, Event* unusedEvent)
{
	m_hitted = false;
}

void CheckLabelBox::addEventListener(CheckLabelBoxCallback const& callback)
{
	m_checkLabelBoxCallback = callback;
}

void CheckLabelBox::layoutElements()
{
	float nextX = 0;

	m_checkBox->setPosition(Vec2(nextX, (CONTENT_HEIGHT - m_checkBox->getBoundingBox().size.height) / 2));
	nextX = m_checkBox->getBoundingBox().getMaxX();

	m_label->setPosition(Vec2(nextX + 2, (CONTENT_HEIGHT - m_label->getBoundingBox().size.height) / 2));
	nextX = m_label->getBoundingBox().getMaxX();

	this->setContentSize(Size(nextX, CONTENT_HEIGHT));
}


bool CheckLabelBox::hitTest(Vec2 const& pt, Camera const* camera, Vec3* p) const
{
	Rect rect;
	rect.size = getContentSize();
	return isScreenPointInRect(pt, camera, getWorldToNodeTransform(), rect, p);
}

NS_END

