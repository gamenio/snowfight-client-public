#ifndef __SOCKET_H__
#define __SOCKET_H__

#include "asio.hpp"
#include "asio/steady_timer.hpp"

#include "common/Common.h"
#include "common/containers/LockedQueue.h"
#include "common/utils/TimeUtil.h"
#include "BasicPacket.h"
#include "MessageBuffer.h"
#include "NetworkError.h"


NS_BEGIN

#define CONNECTION_TIMEOUT			10000 // ms

//
// Wraps the Asio library to provide functionality for operating sockets, 
// receiving network data, and managing sent data queues.
//
template<typename SOCKET_TYPE, typename PACKET_TYPE>
class Socket: public std::enable_shared_from_this<SOCKET_TYPE>
{
	enum
	{
		// Default size of the message buffer
		MESSAGE_BUFFER_SIZE = 4096,

	};

	typedef Socket<SOCKET_TYPE, PACKET_TYPE> BasicSocket;

public:
    Socket(asio::io_service& ioService):
		m_readBuffer(MESSAGE_BUFFER_SIZE),
		m_isWritingAsync(false),
		m_socket(ioService),
		m_isClosed(true),
		m_hostPort(0),
		m_clientPort(0),
		m_domainResolver(ioService),
		m_connectTimer(ioService),
		m_connTimedOut(false),
        m_connTimeout(CONNECTION_TIMEOUT),
        m_tcpNoDelay(true)
	{ 

	}

	virtual ~Socket() 
	{
	}
    
	// Connect to server
	void connect(std::string const& address, uint16 port)
	{
		this->onReadyToConnect();
  
        asio::error_code error;
        asio::ip::address addr = asio::ip::address::from_string(address, error);
		if (error)
		{
            CCLOG("%s will be resolved as a domain name.", address.c_str());
			this->checkConnectTimeout();
			// Resolve domain name
            asio::ip::tcp::resolver::query query(address, std::to_string(port), asio::ip::tcp::resolver::query::numeric_service);
			auto self(this->shared_from_this());
			m_domainResolver.async_resolve(query, [this, address, self](const asio::error_code& error, asio::ip::tcp::resolver::iterator iter) {
				std::string emsg = error.message();

				if (m_connTimedOut)
					return;

				if (!error)
				{
                    asio::ip::tcp::endpoint hitEp;
                    asio::ip::tcp::resolver::iterator endIt;
                    if(iter != endIt)
                    {
                        hitEp = iter->endpoint();
                        ++iter;
                    }
                    if(!hitEp.address().is_v6())
                    {
						// If more than one address is resolved then an attempt is made to find an ipv6 address from those addresses.
                        for (; iter != endIt; ++iter) {
                            asio::ip::tcp::endpoint ep = iter->endpoint();
                            if(ep.address().is_v6())
                            {
                                hitEp = ep;
                                break;
                            }
                        }
                   }
                    
                    CCLOG("%s has been resolved into IP %s.", address.c_str(), hitEp.address().to_string().c_str());
					this->connectInternal(hitEp);

				}
				else
				{
					asio::error_code ignored;
					m_connectTimer.cancel(ignored);
					this->onError(NetworkError(NetworkError::CONNECTION_RESOLVE_FAILED, emsg));
				}
			});
		}
        else
        {
            this->checkConnectTimeout();
            this->connectInternal(asio::ip::tcp::endpoint(addr, port));
        }
	}

	// Disconnect from the server
	void disconnect()
	{
		asio::error_code ignored;
		m_connectTimer.cancel(ignored);
		m_connTimedOut = false;

		m_domainResolver.cancel();

		bool isClosed = !this->isOpen();
		this->closeSocket();


		if(!isClosed)
			this->onDisconnected();

	}
    
	// Sets the connection timeout. Unit: milliseconds
    void setConnectionTimeout(int32 time) { m_connTimeout = time; }
    
    void setTcpNoDelay(bool noDelay) { m_tcpNoDelay = noDelay; }

	// Get the host address and port number after the connection is established.
	asio::ip::address const& getHostAddress() const { return m_hostAddress; }
	uint16 getHostPort() const { return m_hostPort; }

	// Get the client address and port number after the connection is established.
	asio::ip::address const& getClientAddress() const { return m_clientAddress; }
	uint16 getClientPort() const { return m_clientPort; }

	// Returns the open state of the Socket. The function is thread-safe.
	bool isOpen() { return !m_isClosed; }

	/* Network event callback */
    // Called in the network thread.
	virtual void onReceivedData(PACKET_TYPE&& packet) { }
	virtual void onError(NetworkError&& error){ }
	// Called in the thread where the event occurred.
	virtual void onReadyToConnect() { }
	virtual void onConnected() { }
	virtual void onDisconnected() { }

	virtual void update()
	{
		if (m_packetQueue.empty())
			return;

		try
		{
			MessageBuffer buff(MESSAGE_BUFFER_SIZE);
			PACKET_TYPE packet;

			// Whenever possible, packets are combined into the same MessageBuffer and then sent.
			while (m_packetQueue.next(packet))
			{
				if (buff.getRemainingSpace() < packet.getByteSize()
					&& buff.getActiveSize() > 0)
				{
					addToWriteQueue(std::move(buff));
					buff.resize(MESSAGE_BUFFER_SIZE);
				}

				if (buff.getRemainingSpace() >= packet.getByteSize())
				{
					packet.write(buff);
				}
				// Single packet larger than SEND_BUFFER_SIZE
				else
				{
					MessageBuffer largeBuf(packet.getByteSize());
					packet.write(largeBuf);
					addToWriteQueue(std::move(largeBuf));
				}
			}

			if (buff.getActiveSize() > 0)
			{
				addToWriteQueue(std::move(buff));
			}
		}
		catch (PacketException const& ex)
		{
			m_packetQueue.clear();

			this->closeSocket();
			this->onError(NetworkError(NetworkError::ENCODE_PACKET_FAILED, ex.what()));
		}
	}


	// Add a packet to the queue
	void queuePacket(PACKET_TYPE&& packet)
	{
		m_packetQueue.add(std::move(packet));
	}

protected:
	void closeSocket()
	{
		if (m_isClosed.exchange(true))
			return;

		asio::error_code ignored;
		m_socket.shutdown(asio::socket_base::shutdown_send, ignored);

		asio::error_code error;
		m_socket.close(error);
		if (error)
		{
			std::string emsg = error.message();
			CCLOG("Socket close failed. error(%d):%s", error.value(), emsg.c_str());
		}

		// Clear the read buffer
		m_readBuffer.reset();

		// Clear the write queue
		m_packetQueue.clear();
		std::queue<MessageBuffer> empty;
		m_writeQueue.swap(empty);
		m_isWritingAsync = false;
	}

private:
	void connectInternal(asio::ip::tcp::endpoint ep)
	{
		m_isClosed = false;

        asio::error_code error;
        m_socket.open(ep.protocol(), error);
        if(error)
        {
			asio::error_code ignored;
			m_connectTimer.cancel(ignored);

			this->closeSocket();
            this->onError(NetworkError(NetworkError::CONNECTION_FAILED, error.message()));
            return;
        }
        
		// Sets the socket options
        asio::error_code ignored;
        m_socket.set_option(asio::ip::tcp::no_delay(m_tcpNoDelay),ignored);

		auto self(this->shared_from_this());
		m_socket.async_connect(ep, [this, ep, self](const asio::error_code& error) {
			std::string emsg = error.message();

			if (m_connTimedOut)
				return;
            
			asio::error_code ec;
			m_connectTimer.cancel(ec);

			if (!error)
			{
				m_hostAddress = ep.address();
				m_hostPort = ep.port();
				asio::ip::tcp::endpoint localep = m_socket.local_endpoint(ec);
				if (!ec)
				{
					m_clientAddress = localep.address();
					m_clientPort = localep.port();
				}


				this->onConnected();
				this->readHeader();
			}
			else
			{
				this->closeSocket();
				this->onError(NetworkError(NetworkError::CONNECTION_FAILED, emsg));
			}
		});
	}

    
    void checkConnectTimeout()
    {
        m_connTimedOut = false;
        // Closes the connection after a timeout
        asio::error_code ec;
        m_connectTimer.expires_from_now(std::chrono::milliseconds(m_connTimeout), ec);
        if (!ec)
        {
            auto self(this->shared_from_this());
            m_connectTimer.async_wait([this, self](const asio::error_code& error) {
                if (!error)
                {
					m_domainResolver.cancel();
					this->closeSocket();
                    m_connTimedOut = true;

                    this->onError(NetworkError(NetworkError::CONNECTION_TIMED_OUT, "Connection timed out."));
                }
            });
        }
    }
    

	void readHeader()
	{
		m_readBuffer.reset();

		auto self(this->shared_from_this());
		asio::async_read(m_socket, asio::buffer(m_readBuffer.getWritePointer(), PACKET_TYPE::HEADER_BYTE_SIZE),
			[this, self](const asio::error_code& error, std::size_t bytes_transferred)
		{
			if (m_isClosed)
				return;
            
			if (!error)
			{
				NS_ASSERT(bytes_transferred == PACKET_TYPE::HEADER_BYTE_SIZE);

				m_readBuffer.writeCompleted(static_cast<uint16>(bytes_transferred));
				try
				{
					m_readPacket.decodeHeader(m_readBuffer);

					if (m_readPacket.hasBody())
						this->readBody();
					else
					{
						this->onReceivedData(std::move(m_readPacket));
						this->readHeader();
					}
		
				}
				catch (PacketException const& ex)
				{
					this->closeSocket();
					this->onError(NetworkError(NetworkError::DECODE_PACKET_FAILED, ex.what()));
				}	
			}
			else
			{
				this->closeSocket();
				this->onError(NetworkError(NetworkError::RECV_FAILED, error.message()));

			}


		});
	}

	void readBody()
	{
		m_readBuffer.ensureFreeSpace(m_readPacket.getBodyBytes());

		auto self(this->shared_from_this());
		asio::async_read(m_socket, asio::buffer(m_readBuffer.getWritePointer(), m_readPacket.getBodyBytes()),
			[this, self](const asio::error_code& error, std::size_t bytes_transferred)
		{
			if (m_isClosed)
				return;
            
			if (!error)
			{
				NS_ASSERT(bytes_transferred == m_readPacket.getBodyBytes());

				m_readBuffer.writeCompleted(static_cast<uint16>(bytes_transferred));
				try
				{
					m_readPacket.readBody(m_readBuffer);
					this->onReceivedData(std::move(m_readPacket));
					this->readHeader();
				}
				catch (PacketException const& ex)
				{
					this->closeSocket();
					this->onError(NetworkError(NetworkError::DECODE_PACKET_FAILED, ex.what()));
				}

			}
			else
			{
				this->closeSocket();
				this->onError(NetworkError(NetworkError::RECV_FAILED, error.message(), m_readPacket.getOpcode()));
			}

		});
	}

	void addToWriteQueue(MessageBuffer&& buff)
	{
		m_writeQueue.push(std::move(buff));

		this->processWriteQueue();
	}

	void processWriteQueue()
	{
		if (m_isWritingAsync)
			return;

		m_isWritingAsync = true;

		MessageBuffer& buff = m_writeQueue.front();

		auto self(this->shared_from_this());
		asio::async_write(m_socket, asio::buffer(buff.getReadPointer(), buff.getActiveSize()),
			[this, &buff, self](const asio::error_code& error, std::size_t bytes_transferred)
		{
			if (m_isClosed)
				return;
            
			if (!error)
			{
				m_isWritingAsync = false;

				NS_ASSERT(bytes_transferred == buff.getActiveSize());

				buff.readCompleted(static_cast<uint16>(bytes_transferred));

				m_writeQueue.pop();

				if (!m_writeQueue.empty())
					this->processWriteQueue();
			}
			else
			{
				this->closeSocket();
				this->onError(NetworkError(NetworkError::SEND_FAILED, error.message()));
			}
		});
	}

	PACKET_TYPE m_readPacket;
	MessageBuffer m_readBuffer;

	LockedQueue<PACKET_TYPE> m_packetQueue;
	std::queue<MessageBuffer> m_writeQueue;
	bool m_isWritingAsync;

	asio::ip::tcp::socket m_socket;
	std::atomic<bool> m_isClosed;

	asio::ip::address m_hostAddress;
	uint16 m_hostPort;
	asio::ip::address m_clientAddress;
	uint16 m_clientPort;

	asio::ip::tcp::resolver m_domainResolver;

	asio::steady_timer m_connectTimer;
	bool m_connTimedOut;
    int32 m_connTimeout;
    
    bool m_tcpNoDelay;
};

NS_END

#endif // __SOCKET_H__
