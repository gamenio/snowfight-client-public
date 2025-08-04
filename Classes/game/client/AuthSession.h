#ifndef __AUTH_SESSION_H__
#define __AUTH_SESSION_H__

#include "common/Common.h"
#include "common/containers/LockedQueue.h"
#include "common/network/BasicPacket.h"
#include "common/network/ConnectivityHelper.h"
#include "common/utils/MessageDispatcher.h"
#include "game/MessageEnum.h"
#include "AuthSocket.h"
#include "Session.h"

NS_BEGIN

class World;

class AuthSession: public Session
{
public:
	enum RequiredCapabilities
	{
		REQUIRES_DISABLE_REGION_MAPPING_WITH_GEOIP = 0x00000001
	};

	struct LogonConfig
	{
		LogonConfig() :
			version(0),
			build(0),
			deviceModel(""),
			os(""),
			lang(""),
			timezone(0),
			platform(CC_PLATFORM_UNKNOWN),
			country(""),
			requiredCapabilities(0),
			playerId(""),
			originalPlayerId(""),
			networkType(NETWORK_TYPE_UNKNOWN),
			channelId(0)
		{
		}

		uint32 version;
		uint32 build;
		std::string deviceModel;
		std::string os;
		std::string lang;
		int32 timezone;
		uint32 platform;
		std::string country;
		uint32 requiredCapabilities;
		std::string playerId;
		std::string originalPlayerId;
		int32 networkType;
		uint32 channelId;
	};

	AuthSession(std::shared_ptr<AuthSocket> const& socket);
	~AuthSession();

	virtual bool update(float delta) override;

	void closeDelayed() override;
	bool isClosed() const override { return m_isClosed; }
	// Close session now
	void close() override;

	void addToRecvQueue(AuthPacket&& packet);
	void sendPacket(AuthPacket&& packet);

	// LogonHandler
	void handleLogonResult(AuthPacket& packet);
	void sendLogonChallenge(LogonConfig const& config);
	void handleRealmList(AuthPacket& packet);
	void sendGetRealmList();

private:
	void handleMessage(Message const& message);

	std::shared_ptr<AuthSocket> m_socket;
	LockedQueue<AuthPacket> m_recvQueue;

	bool m_isClosed;
	bool m_isClosing;

	World* m_world;
	SocketMgr<AuthSocket, World>* m_socketMgr;
};

NS_END

#endif // __AUTH_SESSION_H__

