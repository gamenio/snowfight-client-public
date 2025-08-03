#ifndef __NETWORK_ERROR_H__
#define __NETWORK_ERROR_H__

#include "common/Common.h"

NS_BEGIN

class NetworkError
{
public:
	enum
	{
		INVALID_OPCODE = 0
	};

	enum ErrorCode
	{
		NONE_ERROR						= 0,
		CONNECTION_RESOLVE_FAILED,
		CONNECTION_TIMED_OUT,
		CONNECTION_FAILED,
        RECV_FAILED,
        SEND_FAILED,
		DECODE_PACKET_FAILED,
		ENCODE_PACKET_FAILED,
	};

	NetworkError() :
		m_message(""),
		m_opcode(INVALID_OPCODE),
		m_errorCode(NONE_ERROR) 
	{ 
	}

	NetworkError(ErrorCode error, std::string const& message, uint16 opcode = INVALID_OPCODE) :
		m_message(message),
		m_opcode(opcode),
		m_errorCode(error)
	{ 
	}

	~NetworkError()
	{
	}

	NetworkError(NetworkError&& right) :
		m_message(""),
		m_opcode(INVALID_OPCODE),
		m_errorCode(NONE_ERROR)
	{
		this->moveFrom(right);
	}

	NetworkError(NetworkError const& right) :
		m_message(""),
		m_opcode(INVALID_OPCODE),
		m_errorCode(NONE_ERROR)
	{
		this->copyFrom(right);
	}

	NetworkError& operator=(NetworkError&& right)
	{
		if (this != &right)
			this->moveFrom(right);

		return *this;
	}

	NetworkError& operator=(NetworkError const& right)
	{
		if (this != &right)
			this->copyFrom(right);

		return *this;
	}


	bool operator!() const { return m_errorCode == NONE_ERROR; }

	bool hasOpcode() { return m_opcode != INVALID_OPCODE; }
	uint16 getOpcode() const { return m_opcode; }

	std::string const& getMessage() const { return m_message; }
	ErrorCode getErrorCode() const { return m_errorCode; }
	std::string getErrorString() const
	{
		std::string error;
		switch (m_errorCode)
		{
		case NONE_ERROR:
			error = "No error";
			break;
		case CONNECTION_RESOLVE_FAILED:
			error = "Unable to resolve hostname";
			break;
		case CONNECTION_TIMED_OUT:
			error = "Connection timed out";
			break;
		case CONNECTION_FAILED:
			error = "Connection failed";
			break;
		case RECV_FAILED:
			error = "Receive data failed";
			break;
		case SEND_FAILED:
			error = "Send data failed";
			break;
		case DECODE_PACKET_FAILED:
			error = "Decode packet failed";
			break;
		case ENCODE_PACKET_FAILED:
			error = "Encode packet failed";
			break;
		default:
			error = "Undefined";
			break;
		}

		return error;
	}

private:
	void copyFrom(NetworkError const& right)
	{
		m_message = right.m_message;
		m_opcode = right.m_opcode;
		m_errorCode = right.m_errorCode;
	}

	void moveFrom(NetworkError& right)
	{
		m_message = std::move(right.m_message);
		std::swap(m_opcode, right.m_opcode);
		std::swap(m_errorCode, right.m_errorCode);
	}

	std::string m_message;
	uint16 m_opcode;
	ErrorCode m_errorCode;
};

NS_END

#endif // __NETWORK_ERROR_H__