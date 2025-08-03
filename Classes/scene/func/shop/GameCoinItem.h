#ifndef __GAME_COIN_ITEM_H__
#define __GAME_COIN_ITEM_H__

#include "ui/UIWidget.h"

#include "common/Common.h"

USING_NS_CC;

NS_BEGIN

struct GameCoinTemplate;

class GameCoinItem: public ui::Widget
{
public:
	GameCoinItem();
	virtual ~GameCoinItem();

	virtual bool init(cocos2d::Size const& size);

	virtual void setData(GameCoinTemplate const* data) = 0;
	virtual GameCoinTemplate const* getData() const = 0;

	virtual void reloadData();

	virtual void setLastItem(bool isLastItem);
	virtual void reset();
	virtual Sprite* clonePicture();
};

NS_END

#endif // __GAME_COIN_ITEM_H__