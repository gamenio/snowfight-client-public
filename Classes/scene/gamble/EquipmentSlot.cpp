#include "EquipmentSlot.h"

#include "game/World.h"
#include "scene/SoundMgr.h"

NS_BEGIN

#define FRAMENAME_EQUIP_SLOT_BG_NORMAL				"equip_slot_bg_normal.png"
#define FRAMENAME_EQUIP_SLOT_SHOES					"equip_slot_shoes.png"
#define FRAMENAME_EQUIP_SLOT_GLOVES					"equip_slot_gloves.png"
#define FRAMENAME_EQUIP_SLOT_HAT					"equip_slot_hat.png"
#define FRAMENAME_EQUIP_SLOT_SNOWBALL_MAKER			"equip_slot_snowball_maker.png"
#define FRAMENAME_EQUIP_SLOT_JACKET					"equip_slot_jacket.png"

#define EQUIP_SLOT_BG_LEVEL_FRAME_FORMAT			"equip_slot_bg_level%d.png"

#define LOCAL_ZORDER_PROP							1

EquipmentSlot* EquipmentSlot::create(SlotType type)
{
	auto ret = new (std::nothrow) EquipmentSlot();
	if (ret && ret->init(type))
	{
		ret->autorelease();
	}
	else
	{
		CC_SAFE_DELETE(ret);
	}

	return ret;
}

EquipmentSlot::EquipmentSlot() :
	m_type(SNOWBALL_MAKER),
	m_bgSp(nullptr),
	m_fgSp(nullptr),
	m_prop(nullptr)
{
}

EquipmentSlot::~EquipmentSlot()
{
	m_bgSp = nullptr;
	m_fgSp = nullptr;
	m_prop = nullptr;;
}

bool EquipmentSlot::init(SlotType type)
{
	if (!Node::init())
		return false;

	m_type = type;

	this->setCascadeOpacityEnabled(true);
	this->setIgnoreAnchorPointForPosition(false);

	m_bgSp = SILSprite::createWithSpriteFrameName(FRAMENAME_EQUIP_SLOT_BG_NORMAL);
	m_bgSp->setSilhouetted(false);
	m_bgSp->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
	this->addChild(m_bgSp);

	m_fgSp = SILSprite::create();
	m_fgSp->setSilhouetted(false);
	m_fgSp->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
	this->addChild(m_fgSp);
	switch (m_type)
	{
	case SNOWBALL_MAKER:
		m_fgSp->setSpriteFrame(FRAMENAME_EQUIP_SLOT_SNOWBALL_MAKER);
		break;
	case GLOVES:
		m_fgSp->setSpriteFrame(FRAMENAME_EQUIP_SLOT_GLOVES);
		break;
	case HAT:
		m_fgSp->setSpriteFrame(FRAMENAME_EQUIP_SLOT_HAT);
		break;
	case JACKET:
		m_fgSp->setSpriteFrame(FRAMENAME_EQUIP_SLOT_JACKET);
		break;
	case SHOES:
		m_fgSp->setSpriteFrame(FRAMENAME_EQUIP_SLOT_SHOES);
		break;
	default:
		break;
	}

	this->setContentSize(m_bgSp->getContentSize());

	return true;
}

void EquipmentSlot::update(float delta)
{
}

void EquipmentSlot::setProp(CarriedProp* prop)
{
	if (m_prop != prop)
	{
		this->removeProp();
		m_prop = prop;
		if (prop)
		{
			prop->setAnchorPoint(Point::ANCHOR_MIDDLE);
			prop->setPosition(this->getContentSize() / 2);
			this->addChild(prop, LOCAL_ZORDER_PROP);
			sSoundMgr->play(SOUND_ITEM_EQUIP);
		}
	}
}

void EquipmentSlot::removeProp()
{
	if (!m_prop)
		return;

	m_prop->removeFromParentAndCleanup(true);
	m_prop = nullptr;

	this->clearAfterPropInactivated();
}

void EquipmentSlot::onPropActivated()
{
	NS_ASSERT_LOG(m_prop, "No prop for slot");
	m_prop->onActivated();

	m_fgSp->setVisible(false);

	DataCarriedItem* data = this->m_prop->getData();
	m_bgSp->setSpriteFrame(StringUtils::format(EQUIP_SLOT_BG_LEVEL_FRAME_FORMAT, data->getLevel()));
}

void EquipmentSlot::onPropInactivated()
{
	NS_ASSERT_LOG(m_prop, "No prop for slot");
	if(m_prop)
		m_prop->onInactivated();

	this->clearAfterPropInactivated();
}

void EquipmentSlot::clearAfterPropInactivated()
{
	m_fgSp->setVisible(true);
	m_bgSp->setSpriteFrame(FRAMENAME_EQUIP_SLOT_BG_NORMAL);
}

NS_END

