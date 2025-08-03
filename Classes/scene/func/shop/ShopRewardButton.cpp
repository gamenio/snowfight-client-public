//
//  ShopRewardButton.cpp
//  snowfight
//
//  Created by Luthier on 2020/8/23.
//

#include "ShopRewardButton.h"

#include "ui/UIScale9Sprite.h"

#include "common/utils/TimeUtil.h"
#include "game/gamecenter/GameCenter.h"
#include "game/LocaleMgr.h"
#include "scene/SoundMgr.h"
#include "scene/Utils.h"

using namespace cocos2d::ui;

NS_BEGIN

#define FRAMENAME_SHOP_REWARD_BTN           "shop_reward_btn.png"

#define CONTENT_SIZE			Size(111, 33)

#define LOADING_INDICATOR_SIZE               Size(43, 7)

#define BUTTON_PADDING_LEFT			31
#define BUTTON_PADDING_RIGHT         21

#define ACTION_TAG_HINT				1

static const int32 SECOND_MILLIS	= 1000;
static const int32 MINUTE_MILLIS	= 60 * SECOND_MILLIS;
static const int32 HOUR_MILLIS		= 60 * MINUTE_MILLIS;
static const int32 DAY_MILLIS		= 24 * HOUR_MILLIS;

ShopRewardButton* ShopRewardButton::create()
{
    auto ret = new (std::nothrow) ShopRewardButton();
    if (ret && ret->init())
    {
        ret->autorelease();
    }
    else
    {
        CC_SAFE_DELETE(ret);
    }
    
    return ret;
}

bool ShopRewardButton::init()
{
    if (!Widget::init())
        return false;

	this->setContentSize(CONTENT_SIZE);

	m_defaultTitle = sLocaleMgr->getString("shop_reward_btn_title");;
	Label* titleLabel = Label::createWithSystemFont(m_defaultTitle, DEFAULT_SYSTEM_FONT, 13);
	titleLabel->setTextColor(Color4B(242, 242, 235, 255));
	titleLabel->setAnchorPoint(Point::ANCHOR_MIDDLE);
	Utils::enableBoldForLabel(titleLabel);
	m_button = Button::create(FRAMENAME_SHOP_REWARD_BTN, "", "", Widget::TextureResType::PLIST);
	m_button->setScale9Enabled(true);
    m_button->addClickEventListener(CC_CALLBACK_1(ShopRewardButton::buttonClickCallback, this));
    m_button->setTitleLabel(titleLabel);
	m_button->setContentSize(this->getContentSize());
	m_button->setPosition(Vec2(this->getContentSize().width / 2, this->getContentSize().height / 2));
    this->addChild(m_button);

	m_loadingIndicator = LoadingIndicator::create(LOADING_INDICATOR_SIZE, Color3B(247, 242, 205));
	m_loadingIndicator->setPosition(Vec2(this->getContentSize().width / 2, this->getContentSize().height / 2));
	m_loadingIndicator->setVisible(false);
	this->addChild(m_loadingIndicator);

	this->scheduleUpdate();

    return true;
}

ShopRewardButton::ShopRewardButton() :
    m_button(nullptr),
	m_loadingIndicator(nullptr),
	m_timerEndTime(0),
	m_isTimerEnabled(false),
	m_isTimeUpHintEnabled(false),
	m_isTimeUp(false)
{
    
}

ShopRewardButton::~ShopRewardButton()
{
    m_button = nullptr;
	m_loadingIndicator = nullptr;
}

void ShopRewardButton::updateState()
{
    LocalPlayer* localPlayer = sGameCenter->getLocalPlayer();
    if(localPlayer->getRewardTime() > 0)
    {
        if(sTimeService->getTimeState() == TimeService::TIME_SYNCED)
        {
            int64 diff = (int64)localPlayer->getRewardTime() * 1000 - sTimeService->getCurrentTimeMillis();
            if (diff > 0)
            {
                m_timerEndTime = localPlayer->getRewardTime();
                m_isTimerEnabled = true;
                m_isTimeUp = false;
                this->updateTitleWithTime((int32)diff);
            }
            else
            {
                m_timerEndTime = 0;
                this->timeIsUp();
            }
            
            if(!m_button->isEnabled())
                m_button->setEnabled(true);
            this->setShowLoadingIndicator(false);
        }
        else if(sTimeService->getTimeState() == TimeService::TIME_SYNCING)
        {
            m_timerEndTime = 0;
            m_isTimerEnabled = false;
            m_isTimeUp = false;
            
            if(!m_button->isEnabled())
                m_button->setEnabled(true);
            this->setShowLoadingIndicator(true);
        }
        else // TimeService::TIME_NONE
        {
            m_timerEndTime = 0;
            m_isTimerEnabled = false;
            m_isTimeUp = false;

            if(m_button->isEnabled())
                m_button->setEnabled(false);
            Label* label = m_button->getTitleRenderer();
            label->setString(sLocaleMgr->getString("btn_title_not_available"));
            label->setScale(1.f);
            this->setShowLoadingIndicator(false);
        }
    }
    else
    {
        m_timerEndTime = 0;
        this->timeIsUp();
        
        m_button->setEnabled(true);
        this->setShowLoadingIndicator(false);
    }
}

void ShopRewardButton::startHintAnimation()
{
	if (m_button->getActionByTag(ACTION_TAG_HINT))
		return;

	auto zoomInOut = Sequence::create(EaseSineOut::create(ScaleTo::create(0.2f, 1.1f)), ScaleTo::create(0.05f, 1.0f), nullptr);
	auto repeat = Repeat::create(zoomInOut, 3);
	auto repeatForever = Repeat::create(Sequence::create(repeat, DelayTime::create(2.f), nullptr), pow(2, 30));
	auto action = Sequence::create(DelayTime::create(1.0f), repeatForever, nullptr);
	action->setTag(ACTION_TAG_HINT);
	m_button->runAction(action);
}

void ShopRewardButton::stopHintAnimation()
{
	Action* action = m_button->getActionByTag(ACTION_TAG_HINT);
	if (!action)
		return;

	m_button->stopAction(action);
	m_button->setScale(1.0f);
}


void ShopRewardButton::setTimeUpHintEnabled(bool enabled)
{
	m_isTimeUpHintEnabled = enabled;
	if (!enabled)
		this->stopHintAnimation();
}

void ShopRewardButton::setClickEventListener(RewardTimerButtonClickCallback const& callback)
{
    m_clickCallback = callback;
}

void ShopRewardButton::setTouchEnabled(bool enabled)
{
	Widget::setTouchEnabled(enabled);
	m_button->setTouchEnabled(enabled);
}

void ShopRewardButton::update(float delta)
{
	if (m_isTimerEnabled)
	{
		int64 diff = (int64)m_timerEndTime * 1000 - sTimeService->getCurrentTimeMillis();
		if (diff > 0)
			this->updateTitleWithTime((int32)diff);
		else
            this->timeIsUp();
	}
}

void ShopRewardButton::onEnter()
{
	Widget::onEnter();
}

void ShopRewardButton::buttonClickCallback(Ref* sender)
{
	sSoundMgr->play(SOUND_BUTTON);
	this->stopHintAnimation();

	if (m_clickCallback)
		m_clickCallback(this);
}

void ShopRewardButton::updateTitleWithTime(int32 milliseconds)
{
	int32 remainder = milliseconds;
	int32 days = (int32)(remainder / DAY_MILLIS);
	remainder %= DAY_MILLIS;
	int32 hours = (int32)(remainder / HOUR_MILLIS);
	remainder %= HOUR_MILLIS;
	int32 minutes = (int32)(remainder / MINUTE_MILLIS);
	remainder %= MINUTE_MILLIS;
	int32 seconds = (int32)(remainder / SECOND_MILLIS);

	bool flag = false;
	std::stringstream ss;
	if (days > 0)
	{
		ss << days << sLocaleMgr->getString("shop_reward_time_day")  << " ";
		flag = true;
	}
	if (flag || hours > 0)
	{
		ss << hours << sLocaleMgr->getString("shop_reward_time_hour") << " ";
		flag = true;
	}
	if (flag || minutes > 0)
	{
		ss << minutes << sLocaleMgr->getString("shop_reward_time_minute") << " ";
		flag = true;
	}
	ss << seconds << sLocaleMgr->getString("shop_reward_time_second");

	std::string timeStr = ss.str();
	Label* titleLabel = m_button->getTitleRenderer();
	titleLabel->setString(timeStr);

	this->updateTitleScale();
}

void ShopRewardButton::applyDefaultTitle()
{
	Label* label = m_button->getTitleRenderer();
	label->setString(m_defaultTitle);
	label->setScale(1.f);
}

void ShopRewardButton::updateTitleScale()
{
	Label* titleLabel = m_button->getTitleRenderer();

	float titleScale = 1.0f;
	float titleMaxWidth = m_button->getContentSize().width - BUTTON_PADDING_LEFT - BUTTON_PADDING_RIGHT;
	if (titleLabel->getContentSize().width > titleMaxWidth)
		titleScale = titleMaxWidth / titleLabel->getContentSize().width;

	float buttonScale = m_button->getRendererNormal()->getScale();
	titleLabel->setScale(titleScale * buttonScale);

	float scaledWidth = titleLabel->getContentSize().width * titleScale;
	float midX = m_button->getContentSize().width / 2;
	float minX = midX - scaledWidth * titleLabel->getAnchorPoint().x;
	if (minX < BUTTON_PADDING_LEFT)
		titleLabel->setPosition(Vec2(midX + (BUTTON_PADDING_LEFT - minX), titleLabel->getPosition().y));
	else
		titleLabel->setPosition(Vec2(midX, titleLabel->getPosition().y));
}

void ShopRewardButton::timeIsUp()
{
    if(m_isTimeUp)
        return;
    
    m_isTimerEnabled = false;
    m_isTimeUp = true;
    this->applyDefaultTitle();
    if (m_isTimeUpHintEnabled)
        this->startHintAnimation();
    
}

void ShopRewardButton::setShowLoadingIndicator(bool show)
{
	m_loadingIndicator->setVisible(show);
	m_button->getTitleLabel()->setVisible(!show);
}

NS_END
