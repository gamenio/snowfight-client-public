//
// StatValueLabel.h
// snowfight
//
// Created by Luthier on 2019/11/24.
//

#ifndef __STAT_VALUE_LABEL_H__
#define __STAT_VALUE_LABEL_H__

#include "common/Common.h"

USING_NS_CC;


NS_BEGIN

class StatValueLabel: public Label, public ActionTweenDelegate
{
public:
	enum ValueType
	{
		NUMBER,
		PERCENTAGE,
	};

	StatValueLabel();
	~StatValueLabel();

	static StatValueLabel* create(ValueType type);
	bool init(ValueType type);

	void setPrecision(int32 precision) { m_precision = precision; }

	void setValue(float value);
	float getValue() const { return m_value; }

	void setMaximum(float maximum);
	float getMaximum() const { return m_maximum; }

	// ActionTweenDelegate
	void updateTweenAction(float value, const std::string& key) override;

private:
	void updateLabel();

	int32 m_precision;
	ValueType m_type;
	float m_value;
	float m_maximum;
};

NS_END


#endif // __STAT_VALUE_LABEL_H__

