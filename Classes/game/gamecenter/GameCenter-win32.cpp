#include "GameCenter-win32.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)

#include "LocalPlayer.h"


NS_BEGIN

GameCenterImpl::GameCenterImpl() :
    m_localPlayer(new LocalPlayer())
{
}

GameCenterImpl::~GameCenterImpl()
{
    CC_SAFE_DELETE(m_localPlayer);
}

void GameCenterImpl::authLocalPlayer()
{
	m_localPlayer->loadDataAsync([this](bool ret) {
		if (!m_listeners.empty())
		{
			AuthResponse resp;
			if(ret)
				resp.state = AUTH_STATE_SUCCESS;
			else
			{
				resp.state = AUTH_STATE_FAIL;
				resp.error.code = GAMECENTER_ERROR_LOAD_DATA_FAILED;
			}
			for(GameCenterListener* listen: m_listeners)
			    listen->onAuthFinished(AuthOperation::SIGN_IN, resp);
		}
	});
}

LocalPlayer *GameCenterImpl::getLocalPlayer() const
{
	return m_localPlayer;
}

void GameCenterImpl::setShowAuthDialog(bool isShowAuthDialog)
{
}

void GameCenterImpl::showLeaderboard(std::string const &leaderboardID)
{
}

bool GameCenterImpl::reportScores(std::string const &leaderboardID)
{
	return false;
}

void GameCenterImpl::addListener(GameCenterListener* listener)
{
    auto it = std::find(m_listeners.begin(), m_listeners.end(), listener);
    if(it == std::end(m_listeners))
        m_listeners.push_back(listener);
}

void GameCenterImpl::removeListener(GameCenterListener* listener)
{
    m_listeners.erase(std::remove(m_listeners.begin(), m_listeners.end(), listener), m_listeners.end());
}


void GameCenterImpl::setEnteringForeground(bool isEnteringForeground)
{
}


NS_END

#endif // CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
