//
// StatusBar.h
// snowfight
// 
// Created by Luthier on 11/27/2019.
//

#ifndef __STATUS_BARS_H__
#define __STATUS_BARS_H__

#include "ui/UIScale9Sprite.h"

#include "common/Common.h"
#include "game/GameConfig.h"
#include "game/entities/DataPlayer.h"

USING_NS_CC;


NS_BEGIN

class StatusBar: public Node
{
public:
    static StatusBar* create();

    StatusBar();
    ~StatusBar();

	bool init() override;
	void update(float delta) override;

	void setData(DataPlayer* data);

private:
	void initMoneybag();
	void initXPProgress();

	void setHead(uint32 displayId);
	void setMoney(int32 money, bool animated);
	void setExperience(int32 xp, int32 nextLevelXP);
	void setLevel(int32 level);

	void startIncomeAnimation();

	DataPlayer* m_data;

	Sprite* m_headSp;
	Label* m_levelLabel;
	ProgressTimer* m_xpProg;
	Label* m_xpLabel;

	Sprite* m_moneybagSp;
	Label* m_moneyLabel;

};


NS_END


#endif // __STATUS_BAR_H__

