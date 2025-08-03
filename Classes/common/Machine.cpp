#include "Machine.h"

NS_BEGIN

Machine* Machine::instance()
{
	static Machine instance;
	return &instance;
}

bool Machine::isNarrowScreen() const
{
	auto visibleSize = Director::getInstance()->getVisibleSize();
	float aspectRatio = visibleSize.height / visibleSize.width;
	if (aspectRatio > 0.6f)
		return true;

	return false;
}

NS_END
