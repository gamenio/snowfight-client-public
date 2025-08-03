#ifndef __LINKED_LIST_H__
#define __LINKED_LIST_H__

#include <list>

#include "common/Common.h"


NS_BEGIN

template<typename T, typename ContainerType = std::list<T>>
class LinkedList
{
public:
	typedef typename ContainerType::iterator iterator;
	typedef typename ContainerType::const_iterator const_iterator;

	iterator begin() { return m_container.begin(); }
	const_iterator begin() const { return m_container.begin(); }

	iterator end() { return m_container.end(); }
	const_iterator end() const { return m_container.end(); }

	LinkedList() { }

	~LinkedList() { }

	void pushBack(T const& element)
	{
		m_container.push_back(element);
	}

	void remove(T const& element)
	{
		m_container.remove(element);
	}

	void clear()
	{
		m_container.clear();
	}

	bool empty() const { return m_container.empty(); }
	std::size_t size() const { return m_container.size(); }

private:
	ContainerType m_container;
};

NS_END


#endif //__LINKED_LIST_H__