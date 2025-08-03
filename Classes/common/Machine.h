#ifndef __MACHINE_H__
#define __MACHINE_H__

#include "common/Common.h"

USING_NS_CC;

NS_BEGIN

enum VibrationType
{
	VIBRATION_TYPE_TOUCH_HOLD,
};

class Machine
{
public:
	static Machine* instance();

	std::string getModel() const;
	std::string getOS() const;
    std::string getDeviceGUID() const;

	cocos2d::Rect getSafeAreaRect() const;
	bool isTablet() const;
	bool isNarrowScreen() const;
    
    void setMultipleTouchEnabled(bool enabled);
    bool isAppInstalled(std::string const& packageName) const;

    void vibrate(VibrationType type);
    
private:
	Machine() { }
	~Machine() { }
};

NS_END

#endif //__MACHINE_H__


