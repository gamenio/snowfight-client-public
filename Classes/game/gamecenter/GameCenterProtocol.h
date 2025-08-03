#ifndef __GAME_CENTER_PROTOCOL_H__
#define __GAME_CENTER_PROTOCOL_H__

#include "common/Common.h"

USING_NS_CC;

NS_BEGIN

class LocalPlayer;

enum GameCenterErrorCode
{
    GAMECENTER_ERROR_NONE,
    GAMECENTER_ERROR_UNKNOWN,					// 由于未知错误，无法完成请求的操作。
    GAMECENTER_ERROR_CANCELED,					// 用户已取消或禁用所请求的操作
    GAMECENTER_ERROR_NOT_AUTHENTICATED,			// 由于未对本地玩家进行身份验证，因此无法完成请求的操作。
    GAMECENTER_ERROR_COMMUNICATIONS_FAILURE,	// 由于与服务器通信时出错，无法完成请求的操作。
    GAMECENTER_ERROR_LOAD_DATA_FAILED,			// 加载玩家数据失败。
    GAMECENTER_ERROR_SAVE_DATA_FAILED,			// 保存玩家数据失败。
    GAMECENTER_ERROR_ACCOUNT_CHANGED,           // 玩家切换了游戏账号。

#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS
    GAMECENTER_ERROR_INVALID_CREDENTIALS,			// 由于用户名或密码不正确，无法完成请求的操作。
    GAMECENTER_ERROR_AUTHENTICATION_IN_PROGRESS,	// 无法完成请求的操作，因为本地玩家已在进行身份验证。
    GAMECENTER_ERROR_USER_DENIED,					// 无法完成请求的操作，因为用户拒绝了该操作。

#elif CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
    GAMECENTER_ERROR_INTERRUPTED,					// 一个没有运行完成的阻塞调用在等待时被中断。
    GAMECENTER_ERROR_TIMEOUT,						// 在等待结果时超时。
    GAMECENTER_ERROR_GPS_VERSION_UPDATE_REQUIRED,	// 已安装的Google Play服务版本已过期。
    GAMECENTER_ERROR_PLAYER_INVALIDED,              // 玩家是无效的。当玩家无效时gpg::Player中的数据将不可用，详见gpg::Player:Valid()

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
    AUTH_STATE_SUCCESS,                     // 玩家登录成功
    AUTH_STATE_FAIL,                        // 登录失败
};

struct AuthResponse
{
    AuthState state;
    // 只有AuthState等于AUTH_STATE_FAIL时才会包含错误信息
    GameCenterError error;
};

enum AuthOperation
{
    SIGN_IN = 1,
    SIGN_OUT = 2 // 目前只有Android平台支持登出
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
    
    // 发起验证本地玩家请求
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
