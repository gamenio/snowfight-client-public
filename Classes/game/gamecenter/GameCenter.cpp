#include "GameCenter.h"

NS_BEGIN

GameCenter::GameCenter() :
    m_gameCenterImpl(nullptr)
{
    m_gameCenterImpl = new GameCenterImpl();
}

GameCenter::~GameCenter()
{
    CC_SAFE_DELETE(m_gameCenterImpl);
}

GameCenter* GameCenter::instance()
{
	static GameCenter instance;
	return &instance;
}


void GameCenter::authLocalPlayer()
{
    if(m_gameCenterImpl)
        m_gameCenterImpl->authLocalPlayer();
}

LocalPlayer* GameCenter::getLocalPlayer() const
{
    if(m_gameCenterImpl)
        return m_gameCenterImpl->getLocalPlayer();
    return nullptr;
}

void GameCenter::setShowAuthDialog(bool isShowAuthDialog)
{
    if(m_gameCenterImpl)
        m_gameCenterImpl->setShowAuthDialog(isShowAuthDialog);
}

void GameCenter::showLeaderboard(std::string const& leaderboardID)
{
    if(m_gameCenterImpl)
        m_gameCenterImpl->showLeaderboard(leaderboardID);
}

bool GameCenter::reportScores(std::string const& leaderboardID)
{
    if(m_gameCenterImpl)
        return m_gameCenterImpl->reportScores(leaderboardID);
    return false;
}

void GameCenter::addListener(GameCenterListener* listener)
{
    if(m_gameCenterImpl)
        m_gameCenterImpl->addListener(listener);
}

void GameCenter::removeListener(GameCenterListener* listener)
{
    if(m_gameCenterImpl)
        m_gameCenterImpl->removeListener(listener);
}

void GameCenter::setEnteringForeground(bool isEnteringForeground)
{
    if(m_gameCenterImpl)
        m_gameCenterImpl->setEnteringForeground(isEnteringForeground);
}

#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID

bool GameCenter::startAuthorizationUI()
{
    if(m_gameCenterImpl)
        return m_gameCenterImpl->startAuthorizationUI();

    return false;
}

void GameCenter::signOut()
{
    if(m_gameCenterImpl)
        m_gameCenterImpl->signOut();
}

#endif // CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID

NS_END

