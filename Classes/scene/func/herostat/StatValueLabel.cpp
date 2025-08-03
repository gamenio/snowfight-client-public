//
// StatValueLabel.cpp
// snowfight
//
// Created by Luthier on 2019/11/24.
//

#include "StatValueLabel.h"
#include "scene/Utils.h"

NS_BEGIN

StatValueLabel::StatValueLabel() :
	m_precision(1),
	m_type(NUMBER),
	m_value(0),
	m_maximum(0)
{
}

StatValueLabel::~StatValueLabel()
{
}

StatValueLabel* StatValueLabel::create(ValueType type)
{
	auto ret = new (std::nothrow) StatValueLabel();
	if (ret && ret->init(type))
	{
		ret->autorelease();
	}
	else
	{
		CC_SAFE_DELETE(ret);
	}

	return ret;
}

bool StatValueLabel::init(ValueType type)
{
	if (!Label::init())
		return false;

	m_type = type;

	this->setSystemFontName(DEFAULT_SYSTEM_FONT);
	this->setSystemFontSize(7);
	this->setTextColor(Color4B(86, 111, 132, 255));
	Utils::enableBoldForLabel(this);
	this->updateLabel();

	return true;
}

void StatValueLabel::setValue(float value)
{
	m_value = value;
	this->updateLabel();
}

void StatValueLabel::setMaximum(float maximum)
{
	m_maximum = maximum;
	this->updateLabel();
}


void StatValueLabel::updateTweenAction(float value, const std::string& key)
{
	this->setValue(value);
}

void StatValueLabel::updateLabel()
{
	switch (m_type)
	{
	case StatValueLabel::NUMBER:
		this->setString(StringUtils::format("%d/%d", (int)m_value, (int)m_maximum));
		break;
	default:
		this->setString(StringUtils::format("%.*f%%", m_precision, m_value * 100));
		break;
	}
}


NS_END

