#ifndef __BASIC_PACKET_H__
#define __BASIC_PACKET_H__

#include <memory>

#include <google/protobuf/message_lite.h>

#include "common/Common.h"
#include "MessageBuffer.h"

NS_BEGIN

class PacketException : public std::exception
{
public:
	PacketException() noexcept { }
	explicit PacketException(std::string const& what) noexcept : m_message(what) { }
	~PacketException() noexcept { }

	char const* what() const noexcept { return m_message.c_str(); }

private:
	std::string m_message;
};

template<uint16 NUM_MSG_TYPES>
class BasicPacket
{
public:

	enum
	{
		// Data header byte length = Body (2 bytes) + Opcode (2 bytes)
		HEADER_BYTE_SIZE = 4,

		// Invalid opcode
		INVALID_OPCODE = 0,

		// Default buffer size
		DEFAULT_BUFFER_SIZE = 1024
	};

	BasicPacket() :
		m_buffer(nullptr),
		m_bufferSize(0),
		m_bodyBytes(0),
		m_opcode(INVALID_OPCODE),
		m_message(nullptr),
        m_timestamp(0)
	{

	}

	explicit BasicPacket(uint16 opcode) :
		m_buffer(nullptr),
		m_bufferSize(0),
		m_bodyBytes(0),
		m_opcode(opcode),
		m_message(nullptr),
        m_timestamp(0)
	{
	}

	template<typename MESSAGE>
	BasicPacket(uint16 opcode, MESSAGE&& message) :
		m_buffer(nullptr),
		m_bufferSize(0),
		m_bodyBytes(static_cast<uint16>(message.ByteSizeLong())),
		m_opcode(opcode),
		m_message(new MESSAGE(std::move(message))),
        m_timestamp(0)
	{
	}

	BasicPacket(BasicPacket&& right) :
		m_buffer(nullptr),
		m_bufferSize(0),
		m_bodyBytes(0),
		m_opcode(INVALID_OPCODE),
		m_message(nullptr),
        m_timestamp(0)
	{
		this->move(right);
	}

	BasicPacket& operator=(BasicPacket&& right)
	{
		if (this != &right)
			this->move(right);

		return *this;
	}

	BasicPacket(BasicPacket const& right) = delete;
	BasicPacket& operator=(BasicPacket const& right) = delete;

	~BasicPacket()
	{
		this->deallocBuffer();
		if (m_message)
		{
			delete m_message;
			m_message = nullptr;
		}
	}

	// Parses the packet with the specified protocol message.
	// A PacketException is thrown if parsing fails.
	void unpack(google::protobuf::MessageLite& message) const
	{
		bool noerror = true;
		if (this->hasBody())
			noerror = message.ParseFromArray(this->getBodyPointer(), this->getBodyBytes());

		if (!noerror)
			throw PacketException(cocos2d::StringUtils::format("Message(%s) to parsing data failed.", message.GetTypeName().c_str()));
	}

	template<typename MESSAGE>
	void setMessage(uint16 opcode, MESSAGE&& message)
	{
		if (m_message)
		{
			delete m_message;
			m_message = nullptr;
		}
		m_message = new MESSAGE(std::move(message));
		m_bodyBytes = static_cast<uint16>(m_message->ByteSizeLong());
		m_opcode = opcode;
	}

	// Write data to the message buffer.
	void write(MessageBuffer& buff)
	{
		NS_ASSERT(buff.getRemainingSpace() >= this->getByteSize());

		this->encodeHeader(buff);

		if (this->hasBody())
			this->encodeBody(buff);
	}

	// Read header data from MessageBuffer.
	void decodeHeader(MessageBuffer& buff)
	{
		uint8* readPtr = buff.getReadPointer();
		uint16 size = 0;
		uint16 opcode = 0;

		size |= ((*(readPtr++) << 8) & 0xFF00);
		size |= (*(readPtr++) & 0xFF);
		opcode |= ((*(readPtr++) << 8) & 0xFF00);
		opcode |= (*(readPtr++) & 0xFF);

		m_bodyBytes = size;
		m_opcode = opcode;
		buff.readCompleted(HEADER_BYTE_SIZE);
	}

	void readBody(MessageBuffer& buff)
	{
		if (!hasBody())
			return;

		this->allocBufferIfNeeded(this->getBodyBytes());
		std::memcpy(this->getBodyPointer(), buff.getReadPointer(), this->getBodyBytes());

		buff.readCompleted(this->getBodyBytes());
	}

	uint16 getOpcode() const { return m_opcode; }
	uint16 getBodyBytes() const { return m_bodyBytes; }
	bool hasBody() const { return m_bodyBytes > 0; }

	uint32 getByteSize() const { return HEADER_BYTE_SIZE + m_bodyBytes; }
    
    void setTimestamp(NSTime timestamp) { m_timestamp = timestamp; }
    NSTime getTimestamp() const { return m_timestamp; }

private:
	void encodeHeader(MessageBuffer& buff)
	{
		NS_ASSERT(m_opcode != INVALID_OPCODE);

		uint8* writePtr = buff.getWritePointer();

		*(writePtr++) = 0xFF & (m_bodyBytes >> 8);
		*(writePtr++) = 0xFF & m_bodyBytes;

		*(writePtr++) = 0xFF & (m_opcode >> 8);
		*(writePtr++) = 0xFF & m_opcode;

		buff.writeCompleted(HEADER_BYTE_SIZE);
	}

	void encodeBody(MessageBuffer& buff)
	{
		NS_ASSERT(m_message != nullptr);

		uint8* writePtr = buff.getWritePointer();
		m_message->SerializeWithCachedSizesToArray(writePtr);
		buff.writeCompleted(m_bodyBytes);
	}


	void move(BasicPacket& right)
	{
		std::swap(m_buffer, right.m_buffer);
		std::swap(m_bufferSize, right.m_bufferSize);
		std::swap(m_bodyBytes, right.m_bodyBytes);
		std::swap(m_opcode, right.m_opcode);
		std::swap(m_message, right.m_message);
        std::swap(m_timestamp, right.m_timestamp);
	}

	void allocBufferIfNeeded(uint16 size)
	{
		if (m_buffer && m_bufferSize >= size)
			return;

		this->deallocBuffer();

		uint16 realSize = DEFAULT_BUFFER_SIZE;
		if (size > realSize)
			realSize = ((size + DEFAULT_BUFFER_SIZE - 1) / DEFAULT_BUFFER_SIZE) * DEFAULT_BUFFER_SIZE;

		m_bufferSize = realSize;
		m_buffer = new uint8[realSize];
	}

	void deallocBuffer()
	{
		if (m_buffer)
		{
			delete[] m_buffer;
			m_buffer = nullptr;
		}
	}

	uint8* getBodyPointer() const { return m_buffer; }

	uint8* m_buffer;
	uint16 m_bufferSize;

	uint16 m_bodyBytes;
	uint16 m_opcode;
	google::protobuf::MessageLite* m_message;
    NSTime m_timestamp;
};

NS_END

#endif // __BASIC_PACKET_H__