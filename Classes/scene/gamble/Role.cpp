#include "Role.h"


NS_BEGIN


Role::Role() :
	m_data(nullptr)
{
}


Role::~Role()
{
	CC_SAFE_RELEASE_NULL(m_data);
}

bool Role::init()
{
	return GameObject::init();
}

bool Role::initWithData(DataUnit* data)
{
	if (!GameObject::init())
		return false;

	CC_SAFE_RETAIN(data);
	m_data = data;

	return true;
}

NS_END