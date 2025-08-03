#ifndef __HERO_CATWALK_H__
#define __HERO_CATWALK_H__

#include "common/Common.h"
#include "scene/gui/Catwalk.h"
#include "HeroUnit.h"

USING_NS_CC;

NS_BEGIN


typedef std::function<void(Ref*, HeroTemplate const*)> HeroCatwalkSelectCallback;
typedef std::function<void(Ref*)> HeroCatwalkBeginScrollCallback;

class HeroCatwalk : public Node
{
public:
	HeroCatwalk();
	~HeroCatwalk();

	static HeroCatwalk* create();
	bool init() override;

	void setupHeroes();
    HeroUnit* getHeroUnitByProductId(std::string const& productId);
	HeroUnit* getHeroUnitByHeroId(HeroID heroId);
	void setSelectedHero(HeroID heroId, bool animated = false);
    
	void setSelectEventListener(HeroCatwalkSelectCallback const& callback);
    void setBeginScrollEventListener(HeroCatwalkBeginScrollCallback const& callback);

private:
	void catwalkSelectCallback(Ref* sender, CatwalkUnit* selectedUnit, int32 prevSelIndex, int32 selectedIndex);
	void catwalkBeginScrollCallback(Ref* sender, CatwalkUnit* prevUnit, CatwalkUnit* expectedUnit, int32 prevSelIndex, int32 expectedSelIndex);

	Catwalk* m_catwalk;
	HeroCatwalkSelectCallback m_selectCallback;
    HeroCatwalkBeginScrollCallback m_beginScrollCallback;
};


NS_END

#endif // __HERO_CATWALK_H__
