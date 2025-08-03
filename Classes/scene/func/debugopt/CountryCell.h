#ifndef __COUNTRY_CELL_H__
#define __COUNTRY_CELL_H__

#include "extensions/cocos-ext.h"

#include "common/Common.h"
#include "game/LocaleMgr.h"

USING_NS_CC;
using namespace cocos2d::ui;

NS_BEGIN

class CountryCell : public extension::TableViewCell
{
public:
	CountryCell();
	~CountryCell();

	static CountryCell* create(cocos2d::Size const& size);
	bool init(cocos2d::Size const& size);

	void setData(CountryInfo const& data);
	CountryInfo const& getData() const { return m_data; }

	void setHighlighted(bool highlight);
	void setSelected(bool selected);

	virtual void onEnter() override;

private:
	Scale9Sprite* m_highlightBg;
	Label* m_nameLabel;
	Sprite* m_dividerSp;
	Sprite* m_tickSp;
	CountryInfo m_data;
};

NS_END

#endif // __COUNTRY_CELL_H__