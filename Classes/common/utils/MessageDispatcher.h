#ifndef __MESSAGE_DISPATCHER_H__
#define __MESSAGE_DISPATCHER_H__

#include <functional>

#include "common/Common.h"
#include "common/utils/SharedPtrUtils.h"
#include "common/containers/LockedQueue.h"



NS_BEGIN

class Message
{
public:
	Message() :
		m_what(0),
		m_object(nullptr)
	{
	}

	explicit Message(uint32 what) :
		m_what(what),
		m_object(nullptr)
	{
	}

	template<typename T>
	Message(uint32 what, T&& object) :
		m_what(what),
		m_object(std::make_shared<typename std::remove_reference<T>::type>(std::forward<T>(object)))
	{
	}

	template<typename T>
	Message(uint32 what, T const& object) :
		m_what(what),
		m_object(std::make_shared<T>(object))
	{
	}

	Message(Message&& right):
		m_what(0),
		m_object(nullptr)
	{
		this->moveFrom(right);
	}

	Message(Message const& right):
		m_what(0),
		m_object(nullptr)
	{
		this->copyFrom(right);
	}

	Message& operator=(Message&& right)
	{
		if (this != &right)
			this->moveFrom(right);

		return *this;
	}

	Message& operator=(Message const& right)
	{
		if (this != &right)
			this->copyFrom(right);

		return *this;
	}

	~Message()
	{
	}

	template<typename T>
	void objectAs(T& object) const
	{
		if(m_object)
			object = *shared_ptr_utils::staticPointerCast<T>(m_object);
	}

	uint32 what() const { return m_what; }

private:
	void copyFrom(Message const& right)
	{
		m_what = right.m_what;
		m_object = right.m_object;
	}

	void moveFrom(Message& right)
	{
		std::swap(m_what, right.m_what);
		m_object = std::move(right.m_object);
	}

	uint32 m_what;
	std::shared_ptr<void> m_object;
};


struct Handler
{
	typedef std::function<void(Message const& msg)> Function;

	Handler() :
		target(nullptr) {}

	template<typename TARGET, typename CALLFUNC>
	Handler(TARGET* _target, CALLFUNC&& _callFunc) :
		callFunc(std::bind(std::forward<CALLFUNC>(_callFunc), _target, std::placeholders::_1)),
		target(_target)
	{

	}

	Function callFunc;
	void* target;
};



//
// 发送消息给指定的Handler处理，Handler将在主线程中被调用。
// 类是线程安全的。
//
class MessageDispatcher
{
public:
	typedef std::list<Handler> HandlerSet;
	typedef std::unordered_map<uint32, HandlerSet> HandlersMap;

	MessageDispatcher();
	~MessageDispatcher();

	void post(uint32 whatMessage);
	void post(Message const& message);

	template<typename TARGET, typename CALLFUNC>
	void registerHandler(uint32 what, TARGET* target, CALLFUNC&& callFunc)
	{
		std::lock_guard<std::mutex> lock(m_mutex);

		m_handlersMap[what].emplace_back(target, callFunc);
	}

	void removeHandlers(uint32 what);
	void removeHandlersWithTarget(void* target);

	// 清理所有Handler和队列中未处理的Message
	void clear();

	void dispatch();	

private:
	std::mutex m_mutex;
	HandlersMap m_handlersMap;
	LockedQueue<Message> m_msgQueue;
};

NS_END



#endif //__MESSAGE_DISPATCHER_H__


