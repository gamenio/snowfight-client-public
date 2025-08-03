#ifndef __SOCKET_MGR__
#define __SOCKET_MGR__

#include "common/Common.h"
#include "NetworkThread.h"
#include "ISocketControl.h"

NS_BEGIN


//
// 创建和管理NetworkThread。
// 创建Socket（例如：WorldSocket）并加入到NetworkThread队列
//
template<typename SOCKET_TYPE, typename SERVICE_TYPE>
class SocketMgr: public ISocketControl
{
public:
	SocketMgr(SERVICE_TYPE* service) :
		m_address(""),
		m_port(0),
		m_service(service),
		m_netThread(nullptr)
	{
	}

	virtual ~SocketMgr()
	{
		CC_SAFE_DELETE(m_netThread);
		m_service = nullptr;
	}

	SERVICE_TYPE* getService() const { return m_service; }

	// 连接服务器并创建会话
	void start(std::string const& address, uint16 port)
	{
		if (m_netThread)
		{
			m_netThread->stop();
			CC_SAFE_DELETE(m_netThread);
		}
		m_netThread = new NetworkThread<SOCKET_TYPE, SERVICE_TYPE>(this);

		m_address = address;
		m_port = port;

		m_netThread->start(address, port);
	}

	// 重新连接服务器并创建会话
	void restart()
	{
		if (m_address.empty() || m_port <= 0)
			return;

		this->start(m_address, m_port);
	}

	// 网络线程是否被停止
	bool isStopped() const 
	{
		if (m_netThread)
			return m_netThread->isStopped();

		return true;
	}

	// 停止网络线程，如果已连接则断开连接
	void stop()
	{
		if (m_netThread)
		{
			m_netThread->stop();
			CC_SAFE_DELETE(m_netThread);
		}
	}

protected:
	std::string m_address;
	uint16 m_port;

	SERVICE_TYPE* m_service;
	NetworkThread<SOCKET_TYPE, SERVICE_TYPE>* m_netThread;
};

NS_END

#endif // __SOCKET_MGR__
