#ifndef __ANTIHERO_H__
#define __ANTIHERO_H__

#include "common/Common.h"
#include "Hero.h"

NS_BEGIN

class AntiHero: public Hero
{
public:
	AntiHero();
	virtual ~AntiHero();

	static AntiHero* createWithData(DataUnit* data);
	virtual bool initWithData(DataUnit* data) override;

	void onActivated() override;
	void onInactivated() override;

	void hurl() override;

	virtual void update(float delta) override;

private:
	void applyItem(ItemApplicationTemplate const* appTmpl, int32 duration, int32 remainingTime) override;
	void unapplyItem(ItemApplicationTemplate const* appTmpl) override;

	void updateAlive();
	void updateLevel();
	void updateReceiveDamage();

    bool addFootprint(cocos2d::Point const& pos, float orient) override;

	NSTime m_damagedLastPlayTime;
};

NS_END

#endif //__ANTIHERO_H__

