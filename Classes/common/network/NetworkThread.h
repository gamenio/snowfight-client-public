#ifndef __NETWORK_THREAD_H__
#define __NETWORK_THREAD_H__

#include "asio.hpp"

#include "common/Common.h"
#include "Socket.h"


NS_BEGIN

#define UPDATE_TIMER_INTERVAL				10 // 毫秒

template<typename T, typename E> class SocketMgr;

//
// 开启一个线程用于异步的网络IO，类非线程安全。
// 
// 注意：在网络线程中的代码块加锁用于与主线程的同步处理，代码块中应避免使用CCLOG()，可能会造成线程死锁
//
template<typename SOCKET_TYPE, typename SERVICE_TYPE>
class NetworkThread
{
public:
	NetworkThread(SocketMgr<SOCKET_TYPE, SERVICE_TYPE>* socketMgr) :
		m_thread(nullptr),
		m_address(""),
		m_port(0),
		m_socket(new SOCKET_TYPE(socketMgr, m_ioService)),
		m_updateTimer(m_ioService),
		m_isStopped(true)

	{
	}

	~NetworkThread()
	{
		this->stop();
	}

	void start(std::string const& address, uint16 port)
	{
        if(!m_isStopped)
            this->stop();
        
        m_isStopped = false;

		m_address = address;
		m_port = port;

		m_thread = new std::thread(&NetworkThread::run, this);
	}

	bool isStopped() const { return m_isStopped; }

	void stop()
	{
		if (m_isStopped.exchange(true))
			return;

		this->awaitThreadToExit();
	}

private:
	void awaitThreadToExit()
	{
		if (m_thread)
		{
			m_thread->join();
			delete m_thread;
			m_thread = nullptr;
		}

	}

	void run()
	{
		if (m_ioService.stopped())
			m_ioService.restart();

		this->scheduleUpdateTimer();

		m_socket->connect(m_address, m_port);

		asio::error_code error;
		m_ioService.run(error);

		m_socket = nullptr;
	}

	void scheduleUpdateTimer()
	{
		asio::error_code ec;
		m_updateTimer.expires_from_now(std::chrono::milliseconds(UPDATE_TIMER_INTERVAL), ec);
		if (!ec)
			m_updateTimer.async_wait(std::bind(&NetworkThread<SOCKET_TYPE, SERVICE_TYPE>::update, this, std::placeholders::_1));
		else
			CCLOG("Schedule update timer failed. msg(%d): %s", ec.value(), ec.message().c_str());
	}

	void update(asio::error_code const& error)
	{
		if (error)
		{
			CCLOG("Update timer error. msg(%d): %s", error.value(), error.message().c_str());
			return;
		}

		if (m_isStopped)
		{
			m_socket->update();
			m_socket->disconnect();
			m_ioService.stop();
			return;
		}

		this->scheduleUpdateTimer();
		m_socket->update();
	}

	std::thread* m_thread;

	std::string m_address;
	uint16 m_port;

	asio::io_service m_ioService;
	std::shared_ptr<SOCKET_TYPE> m_socket;

	asio::steady_timer m_updateTimer;

	std::atomic<bool> m_isStopped;
};

NS_END

#endif // __NETWORK_THREAD_H__
