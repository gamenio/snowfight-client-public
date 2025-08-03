//
//  PropertyBar.cpp
//  snowfight
//
//  Created by Luthier on 2020/06/20.
//
#include "PropertyBar.h"

#include "game/gamecenter/GameCenter.h"
#include "game/LocaleMgr.h"
#include "game/nts/TimeService.h"
#include "scene/Utils.h"
#include "scene/SoundMgr.h"
#include "shop/ShopWicket.h"


using namespace cocos2d::ui;

NS_BEGIN

#define FRAMENAME_FUNC_PROPERTYBAR_BTN						"func_propertybar_btn.png"
#define FRAMENAME_FUNC_PROPERTYBAR_BG						"func_propertybar_bg.png"

#define ACTION_TAG_COUNTING			0
#define ACTION_TAG_PLUS_BTN			1

#define INFO_BUBBLE_ARROW_POSITION				57

#define COUNTING_PER_SECOND				1000
#define COUNTING_DURATION_MIN			0.1f
#define COUNTING_DURATION_MAX			0.6f

#define SCHEDULE_KEY_TIPS_DELAYED	"TipsDelayed"

static int32 s_savedPropertyAmount = -1;

PropertyBar* PropertyBar::create()
{
    auto ret = new (std::nothrow) PropertyBar();
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

bool PropertyBar::init()
{
    if (!Widget::init())
        return false;

	this->setTouchEnabled(true);
	this->addTouchEventListener(CC_CALLBACK_2(PropertyBar::onTouchWidget, this));

	SpriteFrame* bgFrame = SpriteFrameCache::getInstance()->getSpriteFrameByName(FRAMENAME_FUNC_PROPERTYBAR_BG);
	this->setContentSize(bgFrame->getOriginalSize());

	Sprite* bgSp = Sprite::createWithSpriteFrame(bgFrame);
	bgSp->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
	this->addChild(bgSp);

    m_plusBtn = Button::create(FRAMENAME_FUNC_PROPERTYBAR_BTN, "", "", Widget::TextureResType::PLIST);
	m_plusBtn->setSwallowTouches(false);
	m_plusBtn->setPropagateTouchEvents(false);
	m_plusBtn->setAnchorPoint(Point::ANCHOR_MIDDLE);
	m_plusBtn->setPosition(Vec2(this->getContentSize().width - m_plusBtn->getContentSize().width / 2, this->getContentSize().height / 2));
    this->addChild(m_plusBtn);

	m_amountLabel = Label::createWithSystemFont("0", DEFAULT_SYSTEM_FONT, 11);
	m_amountLabel->setAnchorPoint(Point::ANCHOR_MIDDLE_LEFT);
	m_amountLabel->setTextColor(Color4B(94, 100, 124, 255));
	Utils::enableBoldForLabel(m_amountLabel);
	m_amountLabel->setPosition(33, this->getContentSize().height / 2 - 1);
	this->addChild(m_amountLabel);

	if (s_savedPropertyAmount < 0)
	{
		int32 property = sGameCenter->getLocalPlayer()->getProperty();
		s_savedPropertyAmount = property;
		this->setAmount(property, false);
	}
	else
		this->setAmount(s_savedPropertyAmount, false);

	m_infoBubble = InfoBubble::create(InfoBubble::ARROW_UP);
	m_infoBubble->setAnchorPoint(Point::ANCHOR_TOP_LEFT);
	m_infoBubble->setArrowPosition(INFO_BUBBLE_ARROW_POSITION);
	this->addChild(m_infoBubble);

	this->scheduleUpdate();

    return true;
}

PropertyBar::PropertyBar() :
	m_amount(0),
	m_isWatchAdReminderEnabled(false),
	m_shownTipsType(TIPS_NONE),
    m_plusBtn(nullptr),
	m_amountLabel(nullptr),
	m_infoBubble(nullptr)
{
    
}

PropertyBar::~PropertyBar()
{
    m_plusBtn = nullptr;
	m_amountLabel = nullptr;
	m_infoBubble = nullptr;
}

void PropertyBar::setWatchAdReminderEnabled(bool enabled)
{
	m_isWatchAdReminderEnabled = enabled;
}

void PropertyBar::setWatchAdReminderDelay(float delay)
{
	m_watchAdReminderDelayTimer.setDuration(delay);
}

void PropertyBar::showFirstWatchAdTips(float delay)
{
	if (m_shownTipsType == TIPS_WATCH_AD_FIRST)
		return;

	this->hideTips();

	m_shownTipsType = TIPS_WATCH_AD_FIRST;
	this->scheduleOnce([this](float dt) {
		m_infoBubble->show(sLocaleMgr->getString("func_tips_watch_ad_first"));
	}, delay, SCHEDULE_KEY_TIPS_DELAYED);
}

void PropertyBar::hideTips()
{
	if (m_shownTipsType == TIPS_NONE)
		return;

	this->unschedule(SCHEDULE_KEY_TIPS_DELAYED);
	m_infoBubble->hide();

	m_shownTipsType = TIPS_NONE;
}

void PropertyBar::updateWatchAdReminder(float delta)
{
	if (!m_isWatchAdReminderEnabled)
		return;

	if (m_watchAdReminderDelayTimer.getDuration() > 0)
		m_watchAdReminderDelayTimer.update(delta);

	if (m_shownTipsType == TIPS_WATCH_AD_REMINDED)
		return;

	if (sTimeService->getTimeState() != TimeService::TIME_SYNCED)
		return;

	LocalPlayer* localPlayer = sGameCenter->getLocalPlayer();
	if(localPlayer->getRewardTime() <= 0)
	    return;

	int64 diff = sTimeService->getCurrentTimeMillis() - localPlayer->getRewardTime() * 1000LL;
	bool isReady = diff >= 0;
	if (isReady && m_watchAdReminderDelayTimer.passed())
	{
		this->hideTips();
		m_shownTipsType = TIPS_WATCH_AD_REMINDED;

		int32 stage = localPlayer->getRewardStage();
		StageGameCoin gameCoin;
		auto tmpl = sShopMgr->getGameCoinTemplateByGameCoinId(GAME_COIN_FREE_GOLDS);
		if(!tmpl->isNull())
        {
            stage = std::min(stage, (int32)(tmpl->stageRewardList.size() - 1));
            gameCoin = tmpl->stageRewardList[stage];
        }
		std::string amountStr = Utils::separateThousands(gameCoin.amount);
		m_infoBubble->show(StringUtils::format(sLocaleMgr->getString("func_tips_watch_ad_reminded").c_str(), amountStr.c_str()));
	}
}

void PropertyBar::updateAmount(bool animated, float delay)
{
	int32 property = sGameCenter->getLocalPlayer()->getProperty();
	if (s_savedPropertyAmount == property)
		return;

	s_savedPropertyAmount = property;
	this->setAmount(property, animated, delay);
}

void PropertyBar::updateTweenAction(float value, const std::string& key)
{
	this->updateAmountLabel((int32)value);
}


void PropertyBar::onEnterTransitionDidFinish()
{
	Widget::onEnterTransitionDidFinish();
}

void PropertyBar::update(float delta)
{
	this->updateWatchAdReminder(delta);
}

void snowfight::PropertyBar::onEnter()
{
	Widget::onEnter();

	m_infoBubble->setPosition(m_plusBtn->getBoundingBox().getMidX() - m_infoBubble->getArrowPosition() * m_infoBubble->getScale(), m_plusBtn->getBoundingBox().getMinY() + 2);
}

void PropertyBar::setScale(float scale)
{
	Widget::setScale(scale);
	// 保持气泡的大小不变
	m_infoBubble->setScale(1.0f / scale);
}

void PropertyBar::setAmount(int32 amount, bool animated, float delay)
{
	if (amount != m_amount)
	{
		if (animated)
		{
			this->startCountingAnimation(m_amount, amount, delay);
			if(amount > m_amount)
				this->startPlusBtnAnimation(delay);
		}
		else
			this->updateAmountLabel(amount);

		m_amount = amount;
	}
}


void PropertyBar::updateAmountLabel(int32 amount)
{
	std::string result = Utils::separateThousands(amount);
	if (result.length() > 7)
		m_amountLabel->setSystemFontSize(9);
	else if (result.length() > 6)
		m_amountLabel->setSystemFontSize(10);
	else
		m_amountLabel->setSystemFontSize(11);

	m_amountLabel->setString(result);
}

void PropertyBar::onTouchWidget(Ref* sender, TouchEventType event)
{
	m_plusBtn->setHighlighted(this->isHighlighted());
	if (event == TouchEventType::ENDED)
	{
		sSoundMgr->play(SOUND_BUTTON);
		if (m_clickCallback)
			m_clickCallback(this);
	}
}

void PropertyBar::startCountingAnimation(int32 start, int32 end, float delay)
{
	if (Action* action = this->getActionByTag(ACTION_TAG_COUNTING))
		this->stopAction(action);

	if(sSoundMgr->isPlaying(SOUND_COINS_POURING))
		sSoundMgr->stop(SOUND_COINS_POURING);

	Vector<FiniteTimeAction*> actions;
	if (delay > 0)
		actions.pushBack(DelayTime::create(delay));

	bool hasSound = false;
	if(start < end)
    {
        hasSound = true;
        actions.pushBack(CallFunc::create([](){
            sSoundMgr->play(SOUND_COINS_POURING, true);
        }));
    }
	int32 diff = std::abs(end - start);
	float dur = (float)diff / COUNTING_PER_SECOND;
	dur = MIN(COUNTING_DURATION_MAX, MAX(dur, COUNTING_DURATION_MIN));
	auto tween = EaseIn::create(ActionTween::create(dur, "", start, end), 0.2f);
	actions.pushBack(tween);
	if(hasSound)
    {
        actions.pushBack(CallFunc::create([](){
            sSoundMgr->stop(SOUND_COINS_POURING);
        }));
    }

	auto action = Sequence::create(actions);
	action->setTag(ACTION_TAG_COUNTING);
	this->runAction(action);
}

void PropertyBar::startPlusBtnAnimation(float delay)
{
	if (Action* action = m_plusBtn->getActionByTag(ACTION_TAG_PLUS_BTN))
		m_plusBtn->stopAction(action);

	Vector<FiniteTimeAction*> actions;
	if (delay > 0)
		actions.pushBack(DelayTime::create(delay));

	actions.pushBack(ScaleTo::create(0.2f, 1.3f));
	actions.pushBack(ScaleTo::create(0.2f, 1.0f));

	auto action = Sequence::create(actions);
	action->setTag(ACTION_TAG_PLUS_BTN);
	m_plusBtn->runAction(action);
}


NS_END
