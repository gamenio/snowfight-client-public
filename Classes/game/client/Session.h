#ifndef __SESSION_H__
#define __SESSION_H__

#include "common/Common.h"

NS_BEGIN

enum SessionType
{
	SESSION_TYPE_AUTH,
	SESSION_TYPE_WORLD
};

class AuthSession;
class WorldSession;

class Session
{
public:
	Session()
	{
	}

	virtual ~Session() 
	{ 
	}

	SessionType getType() const { return m_type; }
	AuthSession* asAuthSession() { if (m_type == SESSION_TYPE_AUTH) return reinterpret_cast<AuthSession*>(this); else return nullptr; }
	WorldSession* asWorldSession() { if (m_type == SESSION_TYPE_WORLD) return reinterpret_cast<WorldSession*>(this); else return nullptr; }

	// 更新会话，如果返回false则允许释放会话资源
	virtual bool update(float delta) = 0;

	// 立即关闭会话。接收队列中的消息将被忽略
	virtual void close() = 0;
	virtual bool isClosed() const = 0;
	// 等待接收队列中的消息处理完毕后关闭会话
	virtual void closeDelayed() { }
	virtual bool isClosing() const { return false; }

protected:
	SessionType m_type;
};

NS_END

#endif // __SESSION_H__
