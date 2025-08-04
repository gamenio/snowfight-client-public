#ifndef __GAME_CENTER_PROTOCOL_H__
#define __GAME_CENTER_PROTOCOL_H__

#include "common/Common.h"

USING_NS_CC;

NS_BEGIN

class LocalPlayer;

enum GameCenterErrorCode
{
    GAMECENTER_ERROR_NONE,						// No error.
    GAMECENTER_ERROR_UNKNOWN,					// The requested operation could not be completed due to an unknown error.
    GAMECENTER_ERROR_CANCELED,					// The user has canceled or disabled the requested operation.
    GAMECENTER_ERROR_NOT_AUTHENTICATED,			// Since local player have not been authenticated, the requested operation cannot be completed.
    GAMECENTER_ERROR_COMMUNICATIONS_FAILURE,	// The requested operation could not be completed due to an error communicating with the server.
    GAMECENTER_ERROR_LOAD_DATA_FAILED,			// Load player data failed.
    GAMECENTER_ERROR_SAVE_DATA_FAILED,			// Save player data failed.
    GAMECENTER_ERROR_ACCOUNT_CHANGED,           // The player's account has been changed.

#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS
    GAMECENTER_ERROR_INVALID_CREDENTIALS,			// The requested operation cannot be completed because the username or password is incorrect.
    GAMECENTER_ERROR_AUTHENTICATION_IN_PROGRESS,	// The requested operation cannot be completed because the local player is already authenticating.
    GAMECENTER_ERROR_USER_DENIED,					// The requested operation cannot be completed because the user denied it.

#elif CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
    GAMECENTER_ERROR_INTERRUPTED,					// A blocking call was interrupted while waiting and did not run to completion.
    GAMECENTER_ERROR_TIMEOUT,						// Timed out while awaiting the result.
    GAMECENTER_ERROR_GPS_VERSION_UPDATE_REQUIRED,	// The installed version of Google Play services is out of date.
	// The player is invalid. When the player is invalid,
    // the data in gpg::Player will be unavailable. For details, see gpg::Player:Valid().
    GAMECENTER_ERROR_PLAYER_INVALIDED,

#endif

};

struct GameCenterError
{
    GameCenterError() :
    code(GAMECENTER_ERROR_NONE),
    description("")
    { }
    
    GameCenterErrorCode code;
    std::string description;
};

struct ScoreInfo
{
    ScoreInfo() :
        value(0),
        reported(false) { }
    int32 value;
    bool reported;
};

enum AuthState
{                       
    AUTH_STATE_SUCCESS,                     // Player login successful
    AUTH_STATE_FAIL,                        // Login failed
};

struct AuthResponse
{
    AuthState state;
	// Error messages are only included when AuthState equals AUTH_STATE_FAIL
    GameCenterError error;
};

enum AuthOperation
{
    SIGN_IN = 1,
    SIGN_OUT = 2 // Current only Android platform supports logout
};

enum ReportScoreState
{
    REPORT_SCORE_STATE_SUCCESS,
    REPORT_SCORE_STATE_FAIL,
};

struct ReportScoreResponse
{
    ReportScoreState state;
    GameCenterError error;
};

class GameCenterListener
{
public:
    virtual void onReportScoreSuccess() {}
    virtual void onReportScoreFail(GameCenterError const& error) {}

    virtual void onAuthStarted(AuthOperation op) {}
    virtual void onAuthFinished(AuthOperation op, AuthResponse const& response) {}
};

class GameCenterProtocol
{
public:
    virtual ~GameCenterProtocol()
    {
        
    }
    
	// Initiate authenticate local player request
    virtual void authLocalPlayer() = 0;
    virtual LocalPlayer* getLocalPlayer() const = 0;
    virtual void setShowAuthDialog(bool isShowAuthDialog){ }
    
    virtual void showLeaderboard(std::string const& leaderboardID) = 0;
    virtual bool reportScores(std::string const& leaderboardID) = 0;

    virtual void addListener(GameCenterListener* listener) = 0;
    virtual void removeListener(GameCenterListener* listener) = 0;
    
    virtual void setEnteringForeground(bool isEnteringForeground) = 0;

#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID

    virtual bool startAuthorizationUI() = 0;
    virtual void signOut() = 0;

#endif // CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
};

NS_END

#endif // __GAME_CENTER_PROTOCOL_H__
