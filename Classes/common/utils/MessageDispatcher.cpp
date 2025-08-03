#include "MessageDispatcher.h"


NS_BEGIN

MessageDispatcher::MessageDispatcher()
{
}

MessageDispatcher::~MessageDispatcher()
{
}

void MessageDispatcher::post(uint32 whatMessage)
{
	m_msgQueue.add(Message(whatMessage));
}

void MessageDispatcher::post(Message const& message)
{
	m_msgQueue.add(message);
}

void MessageDispatcher::clear()
{
	m_msgQueue.clear();

	std::lock_guard<std::mutex> lock(m_mutex);
	m_handlersMap.clear();
}

void MessageDispatcher::removeHandlers(uint32 what)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	m_handlersMap.erase(what);
}

void MessageDispatcher::removeHandlersWithTarget(void* target)
{
	std::lock_guard<std::mutex> lock(m_mutex);

	for (auto iter = m_handlersMap.begin(); iter != m_handlersMap.end();)
	{
		HandlerSet& handlers = iter->second;
		for (auto it = handlers.begin(); it != handlers.end();)
		{
			Handler& handler = *it;
			if (handler.target == target)
				it = handlers.erase(it);
			else
				++it;
		}

		if (handlers.empty())
			iter = m_handlersMap.erase(iter);
		else
			++iter;

	}
}


void MessageDispatcher::dispatch()
{
	Message msg;
	while (m_msgQueue.next(msg))
	{
		HandlerSet handlers;

		std::unique_lock<std::mutex> lock(m_mutex);

		auto it = m_handlersMap.find(msg.what());
		if (it != m_handlersMap.end())
			handlers = (*it).second;
		else
			continue;

		lock.unlock();

		for (auto it = handlers.begin(); it != handlers.end(); ++it)
			(*it).callFunc(msg);
	}
}


NS_END