#ifndef __SIGNAL_INDICATOR_H__
#define __SIGNAL_INDICATOR_H__

#include "common/Common.h"
#include "common/utils/Timer.h"

USING_NS_CC;

NS_BEGIN

class SignalIndicator : public Node
{
public:
	enum SignalLevel
	{
		SIGNAL_LEVEL_LOST_CONNECTION,
	};

	SignalIndicator();
	~SignalIndicator();

	static SignalIndicator* create();
	bool init() override;

	void onEnter() override;
	void update(float delta) override;

	void show(SignalLevel level);
	void hide();

private:
	Sprite* m_bgSp;
	Sprite* m_levelSp;
	IntervalTimer m_flickerTimer;
};


NS_END

#endif // __SIGNAL_INDICATOR_H__
