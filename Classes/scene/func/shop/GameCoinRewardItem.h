#ifndef __GAME_ICON_REWARD_ITEM_H__
#define __GAME_ICON_REWARD_ITEM_H__

#include "cocos/ui/UIButton.h"
#include "cocos/ui/UIScale9Sprite.h"

#include "common/Common.h"
#include "game/ShopMgr.h"
#include "ShopRewardButton.h"
#include "GameCoinItem.h"

USING_NS_CC;

NS_BEGIN

class GameCoinRewardItem : public GameCoinItem
{
public:
	GameCoinRewardItem();
	~GameCoinRewardItem();

	static GameCoinRewardItem* create(cocos2d::Size const& size);
	bool init(cocos2d::Size const& size) override;

	void setData(GameCoinTemplate const* data) override;
	GameCoinTemplate const* getData() const  override { return m_data; }

	bool isTimeToWatchAd() const { return m_rewardBtn->isTimeUp(); }
	bool isTimerEnabled() const { return m_rewardBtn->isTimerEnabled();}
	void setWatchAdHintEnabled(bool enabled) { m_rewardBtn->setTimeUpHintEnabled(enabled); }

	void reloadData() override;

	void setLastItem(bool isLastItem) override;
	void reset() override;
	Sprite* clonePicture() override;

	void setTouchEnabled(bool enabled) override;
	void onEnter() override;

private:
	GameCoinTemplate const* m_data;

	Sprite* m_dividerSp;
	ui::Widget* m_productGroup;
	Sprite* m_picSp;
	ui::Scale9Sprite* m_bgSp;
	Label* m_amountLabel;
	Label* m_nameLabel;
	ShopRewardButton* m_rewardBtn;
};

NS_END

#endif // __GAME_ICON_REWARD_ITEM_H__
