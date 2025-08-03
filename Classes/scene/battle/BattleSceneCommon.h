#ifndef __BATTLE_SCENE_COMMON_H__
#define __BATTLE_SCENE_COMMON_H__

#include "common/Common.h"
#include "game/entities/DataPlayer.h"

USING_NS_CC;

NS_BEGIN

struct PlayerProfile
{
	PlayerProfile() :
		controllerType(ControllerType::CONTROLLER_TYPE_DUAL_STICKS),
		nickname(""),
		lang(""),
		country(""),
		charId(0),
		winSize(cocos2d::Size::ZERO),
		authProof(""),
		serverAddr(""),
		serverPort(0),
		isRealmByRegion(false),
		isSessionTimeoutDisabled(false)
	{ }

	ControllerType controllerType;
	std::string nickname;
	std::string lang;
	std::string country;
	uint32 charId;
	cocos2d::Size winSize;
	std::string authProof;
	std::string serverAddr;
	uint16 serverPort;
	bool isRealmByRegion;
	bool isSessionTimeoutDisabled;
};

PlayerProfile createDefaultPlayerProfile(uint32 charId, std::string const& nickname);

struct RegionalInfo
{
	RegionalInfo() :
		theaterId(0),
		mapId(0),
		combatGrade(0),
		realmName(""),
		isAppReviewModeEnabled(false)
	{
	}

	uint32 theaterId;
	uint32 mapId;
	uint8 combatGrade;
	std::string realmName;
	bool isAppReviewModeEnabled;
};


NS_END

#endif // __BATTLE_SCENE_COMMON_H__
