#ifndef __LOCKED_MAP_H__
#define __LOCKED_MAP_H__

#include <unordered_map>
#include <mutex>

#include "common/Common.h"


NS_BEGIN

template<typename K, typename V, typename ContainerType = std::unordered_map<K, V>>
class LockedMap
{
public:
	typedef typename ContainerType::iterator iterator;
	typedef typename ContainerType::const_iterator const_iterator;

	iterator begin() { return m_container.begin(); }
	const_iterator begin() const { return m_container.begin(); }

	iterator end() { return m_container.end(); }
	const_iterator end() const { return m_container.end(); }

	// Get a value stored to result by Key.
	// Returns true if the key is found, false otherwise.
	bool get(K const& key, V& result)
	{
		std::lock_guard<std::mutex> lock(m_mutex);

		if (m_container.find(key) == m_container.end())
			return false;

		result = m_container.at(key);
		return true;
	}

	void clear()
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_container.clear();
	}

	void remove(K const& key)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_container.erase(key);
	}

	void insert(K const& key, V const& value)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_container[key] = value;
	}

	void insert(K const& key, V&& value)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_container[key] = std::move(value);
	}

	bool empty()
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		return m_container.empty();
	}

private:
	std::mutex m_mutex;
	ContainerType m_container;
};

NS_END

#endif //__LOCKED_MAP_H__