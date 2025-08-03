#ifndef __STAT_UPGRADE_ITEM_H__
#define __STAT_UPGRADE_ITEM_H__

#include "ui/UIButton.h"

#include "common/Common.h"
#include "game/entities/DataUnit.h"

USING_NS_CC;

NS_BEGIN

struct StatField
{
	enum ValueFormat
	{
		NUMBER,
		PERCENTAGE,
	};

	StatField() :
		type(STAT_NONE),
		name(""),
		format(NUMBER),
		value(Value::Null)
	{
	}

	StatField(StatType _type, std::string _name, Value const& _value, ValueFormat _format) :
		type(_type),
		name(_name),
		value(_value),
		format(_format)
	{
	}

	bool isNull() const { return type == STAT_NONE; }

	StatType type;
	std::string name;
	ValueFormat format;
	Value value;
};

enum StatUpgradeType
{
	STAT_UPGRADE_NONE					= 0,
	STAT_UPGRADE_HEALTH,
	STAT_UPGRADE_ATTACK_RANGE,
	STAT_UPGRADE_MOVE_SPEED,
	STAT_UPGRADE_DAMAGE,
	MAX_STAT_UPGRADE_TYPES
};

struct StatUpgradeData
{
	StatUpgradeData() :
		type(STAT_UPGRADE_NONE),
		price(0),
		stage(0),
		maxStage(0),
		isLocked(false),
		isRecommended(false)
	{
	}

	StatUpgradeType type;
	int32 price;
	uint8 stage;
	int16 maxStage;
	bool isLocked;
	bool isRecommended;
	StatField field1;
	StatField field2;
};

class StatUpgradeItem : public ui::Widget
{
public:
	StatUpgradeItem();
	~StatUpgradeItem();

	static StatUpgradeItem* create(cocos2d::Size const& size);
	bool init(cocos2d::Size const& size);

	void setEnabled(bool enabled) override;
	void setLastItem(bool isLastItem);

	ui::Button* getUpgradeButton() const { return m_upgradeBtn; }

	void setData(StatUpgradeData const& data);
	StatUpgradeData const& getData() const { return m_data; }

	void stopAnimation();
	void reset();

	virtual void onEnter() override;

private:
	Label* addIntroText(std::string const& text, Color4B const& color, float fontSize, int32 tag = Node::INVALID_TAG);
	void arrangeIntroTextElements();

	void playRecommendAnimation();
	void initRecommendAnimation();

	void reloadData();
	void onTouchUpgradeGroup(Ref* sender, Widget::TouchEventType eventType);

	StatUpgradeData m_data;

	Sprite* m_iconSp;
	Label* m_name1Label;
	Label* m_value1Label;
	Label* m_name2Label;
	Label* m_value2Label;
	Node* m_introTextContainer;
	Label* m_stageLabel;
	Sprite* m_dividerSp;
	ui::Widget* m_upgradeGroup;
	Sprite* m_coinSp;
	Label* m_priceLabel;
	ui::Button* m_upgradeBtn;
	Sprite* m_lockSp;
};

NS_END

#endif // __STAT_UPGRADE_ITEM_H__
