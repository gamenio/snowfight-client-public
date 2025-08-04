#ifndef __OBJECT_POOLS_H__
#define __OBJECT_POOLS_H__

#include "common/Common.h"

USING_NS_CC;

NS_BEGIN

template<typename T>
class ObjectPoolListener
{
public:
	virtual void onObjectPoolIncreased(std::unordered_set<T*> const& objects) { }
};

//
// Manage and allocate objects used in the game.
// Initialize some objects to reduce creation time. Increase more objects when there are not enough.
//
template<typename T>
class ObjectPool
{
	static ObjectPool* sInstance;
public:
	static ObjectPool* getInstance()
	{
		if (!sInstance)
		{
			sInstance = new (std::nothrow) ObjectPool<T>;
			NS_ASSERT_LOG(sInstance, "FATAL: Not enough memory");
		}
		return sInstance;
	}

	static void destroyInstance()
	{
		CC_SAFE_DELETE(sInstance);
	}

	void init(int32 initialSize = 100, int32 incrementSize = 50)
	{
		m_availablePool.clear();
		m_unavailablePool.clear();

		m_incrementSize = incrementSize;
		this->increase(initialSize);
		m_isInit = true;
	}

	void reset()
	{
		NS_ASSERT(m_isInit);

		for (auto it = m_unavailablePool.begin(); it != m_unavailablePool.end();)
		{
			T* obj = *it;
			obj->reset();
			NS_ASSERT(m_availablePool.find(obj) == m_availablePool.end());
			m_availablePool.insert(obj);
			it = m_unavailablePool.erase(it);
		}
	}

	T* take()
	{
		NS_ASSERT(m_isInit);

		if (m_availablePool.empty())
			this->increase(m_incrementSize);

		auto it = m_availablePool.begin();
		NS_ASSERT(it != m_availablePool.end());
		T* obj = *it;
		m_availablePool.erase(it);
		NS_ASSERT(m_unavailablePool.find(obj) == m_unavailablePool.end());
		m_unavailablePool.insert(obj);

		return obj;
	}

	void put(T* obj)
	{
		NS_ASSERT(m_isInit);

		obj->reset();
		NS_ASSERT(m_availablePool.find(obj) == m_availablePool.end());
		m_availablePool.insert(obj);
		m_unavailablePool.erase(obj);
	}
    
    int32 getAvailableCount() const { return static_cast<int32>(m_availablePool.size()); }
    int32 getUnavailableCount() const { return static_cast<int32>(m_unavailablePool.size()); }

	void setObjectPoolListener(ObjectPoolListener<T>* listener) { m_listener = listener; }
	void removeObjectPoolListener() { m_listener = nullptr; }

private:
	void increase(int32 size)
	{
		for (int i = 0; i < size; i++)
		{
			T* obj = T::create();
			obj->retain();
			m_availablePool.insert(obj);
		}

		if (m_listener)
			m_listener->onObjectPoolIncreased(m_availablePool);
	}
    
	ObjectPool() : 
		m_isInit(false),
		m_incrementSize(0),
		m_listener(nullptr)
	{ 
	}

	~ObjectPool() 
	{
		for (auto it = m_availablePool.begin(); it != m_availablePool.end();)
		{
			T* obj = *it;
			it = m_availablePool.erase(it);
			obj->release();
		}

		for (auto it = m_unavailablePool.begin(); it != m_unavailablePool.end();)
		{
			T* obj = *it;
			it = m_unavailablePool.erase(it);
			obj->release();
		}

		m_listener = nullptr;
	}

	bool m_isInit;
	int32 m_incrementSize;
	ObjectPoolListener<T>* m_listener;

	std::unordered_set<T*> m_availablePool;
	std::unordered_set<T*> m_unavailablePool;
};

template<typename T>
ObjectPool<T>* ObjectPool<T>::sInstance = nullptr;

NS_END

#endif // __OBJECT_POOLS_H__