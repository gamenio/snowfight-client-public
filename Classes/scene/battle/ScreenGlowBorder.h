#ifndef __SCREEN_GLOW_BORDER_H__
#define __SCREEN_GLOW_BORDER_H__

#include "ui/UIScale9Sprite.h"

#include "common/Common.h"

USING_NS_CC;

NS_BEGIN

class ScreenGlowBorder : public Node
{
public:
	ScreenGlowBorder();
	~ScreenGlowBorder();

	static ScreenGlowBorder* create();
	bool init() override;

private:
	void initElements();
};


NS_END

#endif // __SCREEN_GLOW_BORDER_H__
