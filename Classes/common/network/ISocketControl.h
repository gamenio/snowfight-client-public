#ifndef __I_SOCKET_CONTROL__
#define __I_SOCKET_CONTROL__

#include "common/Common.h"

NS_BEGIN

class ISocketControl
{
public:
	ISocketControl() { }
	virtual ~ISocketControl() { }

	virtual void start(std::string const& address, uint16 port) = 0;
	virtual void restart() = 0;

	virtual bool isStopped() const = 0;
	virtual void stop() = 0;
};


NS_END

#endif // __I_SOCKET_CONTROL__
