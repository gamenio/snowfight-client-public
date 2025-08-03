#include "HeroCatwalk.h"


NS_BEGIN

#define	CONTENT_SIZE				Size(180, 120)				
#define CATWALK_CONTAINER_SIZE		Size(120, 120)

HeroCatwalk::HeroCatwalk() :
	m_catwalk(nullptr)
{
}

HeroCatwalk::~HeroCatwalk()
{
	m_catwalk = nullptr;
}

HeroCatwalk* HeroCatwalk::create()
{
	HeroCatwalk* node = new (std::nothrow) HeroCatwalk();
	if (node && node->init())
	{
		node->autorelease();
		return node;
	}
	CC_SAFE_DELETE(node);
	return nullptr;
}


bool HeroCatwalk::init()
{
	if (!Node::init())
		return false;

	this->setContentSize(CONTENT_SIZE);
	m_catwalk = Catwalk::create(CONTENT_SIZE, CATWALK_CONTAINER_SIZE,  0.1f, 1.0f, 3, 1.0f);
	m_catwalk->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
	m_catwalk->setSelectEventListener(std::bind(&HeroCatwalk::catwalkSelectCallback, this,
		std::placeholders::_1,
		std::placeholders::_2,
		std::placeholders::_3,
		std::placeholders::_4));
	m_catwalk->setBeginScrollEventListener(std::bind(&HeroCatwalk::catwalkBeginScrollCallback, this, 
		std::placeholders::_1, 
		std::placeholders::_2,
		std::placeholders::_3, 
		std::placeholders::_4, 
		std::placeholders::_5));
	this->addChild(m_catwalk);


	return true;
}

HeroUnit* HeroCatwalk::getHeroUnitByProductId(std::string const& productId)
{
    auto const& units = m_catwalk->getUnitList();
    for(auto it = units.begin(); it != units.end(); ++it)
    {
        HeroUnit* hero = (HeroUnit*)*it;
        if(hero->getHeroTemplate()->productId == productId)
            return hero;
    }
    return nullptr;
}

HeroUnit* HeroCatwalk::getHeroUnitByHeroId(HeroID heroId)
{
	auto const& units = m_catwalk->getUnitList();
	for (auto it = units.begin(); it != units.end(); ++it)
	{
		HeroUnit* hero = (HeroUnit*)*it;
		if (hero->getHeroTemplate()->id == heroId)
			return hero;
	}
	return nullptr;
}

void HeroCatwalk::setSelectedHero(HeroID heroId, bool animated)
{
	auto const& units = m_catwalk->getUnitList();
	int32 nUnits = static_cast<int32>(units.size());
	for (int32 i = 0; i < nUnits; ++i)
	{
		HeroUnit* hero = (HeroUnit*)units.at(i);
		if (hero->getHeroTemplate()->id == heroId)
		{
			m_catwalk->setSelected(i, animated);
			break;
		}
	}
}

void HeroCatwalk::setSelectEventListener(HeroCatwalkSelectCallback const& callback)
{
	m_selectCallback = callback;
}

void HeroCatwalk::setBeginScrollEventListener(HeroCatwalkBeginScrollCallback const& callback)
{
    m_beginScrollCallback = callback;
}

void HeroCatwalk::catwalkSelectCallback(Ref* sender, CatwalkUnit* selectedUnit, int32 prevSelIndex, int32 selectedIndex)
{
	//CCLOG("selected index: %d", selectedIndex);
	HeroUnit* unit = (HeroUnit*)selectedUnit;
	unit->activate();

	if (m_selectCallback)
		m_selectCallback(this, unit->getHeroTemplate());
}

void HeroCatwalk::catwalkBeginScrollCallback(Ref* sender, CatwalkUnit* prevUnit, CatwalkUnit* expectedUnit, int32 prevSelIndex, int32 expectedSelIndex)
{
	//CCLOG("begin scroll prevSelIndex:%d expectedSelIndex:%d", prevSelIndex, expectedSelIndex);
	HeroUnit* unit = (HeroUnit*)prevUnit;
	unit->inactivate();
    
    if(m_beginScrollCallback)
        m_beginScrollCallback(this);
}

void HeroCatwalk::setupHeroes()
{
	Vector<CatwalkUnit*> units;

	auto heroTmplList = sShopMgr->getHeroTemplateList();
	for (auto it = heroTmplList->begin(); it != heroTmplList->end(); ++it)
	{
		HeroTemplate const* tmpl = &(*it);
		HeroUnit* unit = HeroUnit::create(tmpl);
		units.pushBack(unit);
	}

	m_catwalk->setUnitList(units);
}

NS_END

