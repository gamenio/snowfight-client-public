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

	// Update the session. If false is returned, session resource release is allowed.
	virtual bool update(float delta) = 0;

	// Close the session immediately. Messages in the receive queue will be ignored.
	virtual void close() = 0;
	virtual bool isClosed() const = 0;
	// Wait for the messages in the receive queue to be processed, then close the session.
	virtual void closeDelayed() { }
	virtual bool isClosing() const { return false; }

protected:
	SessionType m_type;
};

NS_END

#endif // __SESSION_H__
