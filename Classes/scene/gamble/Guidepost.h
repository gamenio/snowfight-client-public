#ifndef __GUIDEPOST_H__
#define __GUIDEPOST_H__

#include "common/Common.h"
#include "game/entities/DataUnitLocator.h"
#include "Gamble.h"


USING_NS_CC;


NS_BEGIN


class Guidepost : public Node, public Gamble
{
public:
	enum class BrightState
	{
		NORMAL,
		GRAY
	};

	static Guidepost* createWithData(DataLocatorObject* data);

	Guidepost();
	virtual ~Guidepost();

	virtual bool initWithData(DataLocatorObject* data);

	void update(float delta) override;

	void onActivated() override;
	void onInactivated() override;
	bool isActive() const override { return m_isActive; }

	void cleanUpdateMask() override;
	DataLocatorObject* getData() const override { return m_data; }

private:
	cocos2d::Point intersectionOnRect(cocos2d::Point const& p, cocos2d::Rect const& rect);

	void setBrightState(BrightState state);

	void initLocatorObject();
	void updateLocatorObject(bool force);

	void initWithUnitLocator(DataUnitLocator* data);
	void updateWithUnitLocator(DataUnitLocator* data, bool force);

	bool m_isActive;
	DataLocatorObject* m_data;

	BrightState m_brightState;
	Sprite* m_mainSp;
};

NS_END

#endif // __GUIDEPOST_H__
