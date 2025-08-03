#include "BattleSceneCommon.h"

#include "game/GameConfig.h"
#include "game/UserPreferences.h"
#include "game/LocaleMgr.h"
#include "game/ClientConfig.h"
#include "game/entities/DataPlayer.h"

NS_BEGIN

PlayerProfile createDefaultPlayerProfile(uint32 charId, std::string const& nickname)
{
	PlayerProfile profile;
#if USE_DEBUG_OPTION
	profile.authProof = sUserPreferences->isDebugOptionEnabled(DEBUG_OPTION_GAME_MASTER) ? AUTH_PROOF_GM : AUTH_PROOF_PLAYER;
	profile.isRealmByRegion = sUserPreferences->isDebugOptionEnabled(DEBUG_OPTION_REALM_BY_REGION);
	profile.isSessionTimeoutDisabled = sUserPreferences->isDebugOptionEnabled(DEBUG_OPTION_SESSION_TIMEOUT_DISABLED);
	std::string country = sUserPreferences->getCountry();
	if (!country.empty())
		profile.country = country;
	else
		profile.country = sLocaleMgr->getCountryCode();
#else
	profile.authProof = AUTH_PROOF_PLAYER;
	profile.country = sLocaleMgr->getCountryCode();
#endif // USE_DEBUG_OPTION

	profile.controllerType = (ControllerType)sUserPreferences->getControllerType();
	profile.charId = charId;
	profile.serverAddr = AUTH_SERVER_ADDR;
	profile.serverPort = AUTH_SERVER_PORT;
	profile.nickname = nickname;
	profile.lang = sLocaleMgr->getLangTag();
	profile.winSize = Director::getInstance()->getWinSize();

	return profile;
}

NS_END

