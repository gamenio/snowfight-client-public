#include "GameObject.h"


NS_BEGIN

GameObject::GameObject():
	m_isActive(false),
	m_gameMapLayer(nullptr)
{
}


GameObject::~GameObject()
{
	m_gameMapLayer = nullptr;
}


bool GameObject::init(void)
{
	return Node::init();
}

void GameObject::update(float delta)
{
	if (!m_isActive)
		return;

	Node::update(delta);
}

void GameObject::onActivated()
{
	m_isActive = true;
}

void GameObject::onInactivated()
{
	if (!m_isActive)
		return;

	m_isActive = false;

	this->cleanUpdateMask();
}

void GameObject::cleanUpdateMask()
{
	if (this->getData())
		this->getData()->clearUpdateFlags();
}


NS_END
