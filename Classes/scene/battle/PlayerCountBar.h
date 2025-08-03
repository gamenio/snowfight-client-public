#ifndef __PLAYER_COUNT_BAR_H__
#define __PLAYER_COUNT_BAR_H__

#include "common/Common.h"

USING_NS_CC;

NS_BEGIN

class PlayerCountBar : public Node
{
public:
	static PlayerCountBar* create();

	PlayerCountBar();
	~PlayerCountBar();

	bool init() override;
	void update(float delta) override;

	void setAliveCount(int32 count);

private:
	void setKillCount(int32 count);

	Sprite* m_aliveIconSp;
	Label* m_aliveCountLabel;
	Sprite* m_killedIconSp;
	Label* m_killCountLabel;
};


NS_END

#endif // __PLAYER_COUNT_BAR_H__
