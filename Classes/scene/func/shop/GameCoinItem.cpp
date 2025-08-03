#include "GameCoinItem.h"


NS_BEGIN

GameCoinItem::GameCoinItem()
{
}

GameCoinItem::~GameCoinItem() 
{
}

bool GameCoinItem::init(Size const& size)
{
	if (!Widget::init())
		return false;

	this->setContentSize(size);

	return true;
}

void GameCoinItem::reloadData()
{
}

void GameCoinItem::setLastItem(bool isLastItem) 
{
}

void GameCoinItem::reset() 
{
}

Sprite* GameCoinItem::clonePicture() 
{ 
	return nullptr; 
}

NS_END
