#include "game/client/AuthSession.h"


#include "game/client/protocol/pb/LogonChallenge.pb.h"
#include "game/client/protocol/pb/LogonResult.pb.h"
#include "game/client/protocol/pb/RealmList.pb.h"

#include "game/client/protocol/AuthOpcode.h"
#include "game/client/WorldSocketMgr.h"
#include "game/World.h"


NS_BEGIN


void AuthSession::handleLogonResult(AuthPacket& packet)
{
	LogonResult message;
	packet.unpack(message);
	switch (message.result())
	{
	case LogonResult::AUTH_OK:
		m_world->notifyLogonSucceeded();
		break;
	default:
	{
		this->close();
		LogonResult::ErrorCode errorCode = message.error_code();
		m_world->notifyLogonFailed(errorCode);
		break;
	}
	}
}

void AuthSession::sendLogonChallenge(LogonConfig const& config)
{
	LogonChallenge message;
	message.set_version(config.version);
	message.set_build(config.build);
	message.set_device_model(config.deviceModel);
	message.set_os(config.os);
	message.set_required_capabilities(config.requiredCapabilities);
	message.set_platform(config.platform);
	message.set_lang(config.lang);
	message.set_timezone(config.timezone);
	message.set_country(config.country);
	message.set_playerid(config.playerId);
	message.set_original_playerid(config.originalPlayerId);
	message.set_network_type(config.networkType);
	message.set_channel_id(config.channelId);
	AuthPacket packet(auth::CMSG_LOGON_CHALLENGE, std::move(message));
	this->sendPacket(std::move(packet));
}

void AuthSession::handleRealmList(AuthPacket& packet)
{
	RealmList realmList;
	packet.unpack(realmList);

	// Realm sort (in ascending order of PopulationLevel)
	std::vector<Realm> orderedRealms(realmList.result().begin(), realmList.result().end());
	std::sort(orderedRealms.begin(), orderedRealms.end(), [](Realm const& a, Realm const& b)
	{
		return a.population_level() < b.population_level();
	});

	// Get the Realm with the smallest PopulationLevel value
	auto it = orderedRealms.begin();
	if (it != orderedRealms.end())
	{
		Realm const& realm = *it;

//        uint32 flag = realm.flag();
//        int32 timezone = realm.timezone();
//        float populationLevel = realm.population_level();

		m_world->notifyFetchRealmListSucceeded(realm);

		std::string address = realm.address();
		// Realm configuration requirements point to the authentication server address
		if (address.empty() ||  address == "0.0.0.0")
			address = m_socket->getHostAddress().to_string();

		uint16 port = (uint16)realm.port();

		this->close();
		m_world->startWorldSocket(address, port);
	}
	else
	{
		this->close();
		m_world->notifyFetchRealmlistFailed();
	}

}

void AuthSession::sendGetRealmList()
{
	AuthPacket packet(auth::CMSG_GET_REALM_LIST);
	this->sendPacket(std::move(packet));

}




NS_END
