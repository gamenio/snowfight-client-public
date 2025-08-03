#include "TutorialLayer.h"

#include "common/Machine.h"
#include "game/World.h"
#include "scene/gamble/MyHero.h"
#include "scene/battle/BattleScene.h"
#include "scene/Utils.h"

NS_BEGIN

#define SCHEDULE_KEY_TUTORIAL_PROCESS				"TutorialProcess"
#define SCHEDULE_KEY_HIDE_TUTORIAL					"HideTutorial"
#define SCHEDULE_KEY_PLAY_NEXT						"PlayNext"

#define TUTORIAL_START_DELAY						2.0f

#define CONTROL_ACTION_TAG_SHOWING					1

TutorialLayer* TutorialLayer::create(BattleScene* scene)
{
	auto ret = new (std::nothrow) TutorialLayer();
	if (ret && ret->init(scene))
	{
		ret->autorelease();
	}
	else
	{
		CC_SAFE_DELETE(ret);
	}

	return ret;
}

TutorialLayer::TutorialLayer() :
	m_scene(nullptr),
	m_safeInsetLeft(0),
	m_safeInsetRight(0),
	m_tutorialView(nullptr),
	m_currTutorialProcess(TUTORIAL_PROCESS_NONE)
{
}

TutorialLayer::~TutorialLayer()
{
	m_tutorialView = nullptr;
	m_scene = nullptr;
}

bool TutorialLayer::init(BattleScene* scene)
{
	if (!Layer::init())
		return false;

	m_scene = scene;

	Rect safeArea = Machine::instance()->getSafeAreaRect();
	m_safeInsetLeft = Utils::getSafeInsetLeft(safeArea);
	m_safeInsetRight = Utils::getSafeInsetRight(safeArea);

	m_tutorialView = TutorialView::create();
	m_tutorialView->setPosition(Point::ZERO);
	m_tutorialView->setVisible(false);
	this->addChild(m_tutorialView);

	return true;
}

void TutorialLayer::startTutorial()
{
	auto localPlayer = sGameCenter->getLocalPlayer();
	if (localPlayer->isTutorialCompleted())
		return;

	if (m_currTutorialProcess != TUTORIAL_PROCESS_NONE)
		return;

	TutorialService* service = TutorialService::getInstance();
	service->setEventListener(this);
	service->setEnabled(true);

	m_currTutorialProcess = localPlayer->getTutorialProcess();
	if (m_currTutorialProcess < TUTORIAL_PROCESS_FIND_ENEMY)
		this->setNextTutorialProcess(TUTORIAL_PROCESS_CONTROL_MOVE, TUTORIAL_START_DELAY, false);
	else if (m_currTutorialProcess < TUTORIAL_PROCESS_COLLECT_MAGIC_BEANS)
		this->setNextTutorialProcess(TUTORIAL_PROCESS_COLLECT_MAGIC_BEANS, TUTORIAL_START_DELAY, false);
	else if (m_currTutorialProcess < TUTORIAL_PROCESS_SHOW_EQUIPMENT_BAR)
		this->setNextTutorialProcess(TUTORIAL_PROCESS_SHOW_EQUIPMENT_BAR, TUTORIAL_START_DELAY, false);
	else if (m_currTutorialProcess < TUTORIAL_PROCESS_SEND_EMO)
		this->setNextTutorialProcess(TUTORIAL_PROCESS_SEND_EMO, TUTORIAL_START_DELAY, false);
}

void TutorialLayer::onTutorialEventTriggered(TutorialEventType eventType)
{
	TutorialService* service = TutorialService::getInstance();
	switch (eventType)
	{
	case TUTORIAL_EVENT_PICKED_UP_ALL_MAGICBEANS:
		this->setNextTutorialProcess(TUTORIAL_PROCESS_CONTROL_ATTACK, 1.0f);
		break;
	case TUTORIAL_EVENT_CONTROL_ATTACK:
		break;
	case TUTORIAL_EVENT_ITEMBOX_OPENED:
		this->setNextTutorialProcess(TUTORIAL_PROCESS_STAMINA_BAR_DESC, 1.0f);
		break;
	case TUTORIAL_EVENT_PICKED_UP_CONSUMABLE_ITEM:
	case TUTORIAL_EVENT_PICKED_UP_EQUIPMENT:
		if (service->isEventTriggered(TUTORIAL_EVENT_PICKED_UP_CONSUMABLE_ITEM) && service->isEventTriggered(TUTORIAL_EVENT_PICKED_UP_EQUIPMENT))
			this->setNextTutorialProcess(TUTORIAL_PROCESS_EQUIPMENT_DESC, 0.0f, false);
		break;
	case TUTORIAL_EVENT_CONTROL_SUPER_ATTACK:
		this->setNextTutorialProcess(TUTORIAL_PROCESS_USE_ITEM, 2.0f);
		break;
	case TUTORIAL_EVENT_ITEM_USED:
		this->setNextTutorialProcess(TUTORIAL_PROCESS_FIND_ENEMY, 1.0f);
		break;
	case TUTORIAL_EVENT_ENEMY_FOUND:
		if(m_currTutorialProcess >= TUTORIAL_PROCESS_FIND_ENEMY)
			this->hideTutorial(1.0f);
		break;
	case TUTORIAL_EVENT_BATTLE_ENDING:
		this->setNextTutorialProcess(TUTORIAL_PROCESS_OBJECTIVE, 0.f, false);
		break;
	case TUTORIAL_EVENT_EQUIPMENT_BUTTON_TAPPED:
		if (m_currTutorialProcess >= TUTORIAL_PROCESS_SHOW_EQUIPMENT_BAR)
			this->hideTutorial(0.f);
		break;
	case TUTORIAL_EVENT_SMILEY_BOX_TAPPED:
		if (m_currTutorialProcess >= TUTORIAL_PROCESS_SEND_EMO)
			this->hideTutorial(0.f);
		break;
	default:
		break;
	}
}

void TutorialLayer::setNextTutorialProcess(TutorialProcess process, float delay, bool isStepByStep)
{
	if (process <= m_currTutorialProcess)
		return;

	if (isStepByStep)
	{
		int32 diff = process - m_currTutorialProcess;
		if (diff != 1)
			return;

		if (this->isScheduled(SCHEDULE_KEY_TUTORIAL_PROCESS))
			return;
	}

	if (this->isScheduled(SCHEDULE_KEY_TUTORIAL_PROCESS))
	{
		CCLOG("TutorialLayer: Tutorial process %d is skipped.", m_currTutorialProcess);
		this->unschedule(SCHEDULE_KEY_TUTORIAL_PROCESS);
	}

	m_currTutorialProcess = process;

	auto callback = [this, process](float) {
		this->showTutorial(process);
		this->onTutorialProcessCompleted(process);
	};
	if (delay > 0)
		this->scheduleOnce(callback, delay, SCHEDULE_KEY_TUTORIAL_PROCESS);
	else
		callback(0.f);
}

void TutorialLayer::onTutorialProcessCompleted(TutorialProcess process)
{
	switch (process)
	{
	case TUTORIAL_PROCESS_CONTROL_MOVE:
		if(MoveStick* stick = m_scene->getGamePad()->getMoveStick())
			stick->setControlGestureEnabled(true, 2.0f);
		break;
	case TUTORIAL_PROCESS_CONTROL_ATTACK:
		if (MoveStick* stick = m_scene->getGamePad()->getMoveStick())
			stick->setControlGestureEnabled(false);
		if (AttackStick* stick = m_scene->getGamePad()->getAttackStick())
		{
			this->showControlWithFadeIn(stick, 0.2f, 0.f);
			stick->setControlGestureEnabled(AttackControlGesture::GESTURE_TYPE_DRAG, 2.0f);
		}
		break;
	case TUTORIAL_PROCESS_STAMINA_BAR_DESC:
		if (AttackStick* stick = m_scene->getGamePad()->getAttackStick())
			stick->setControlGestureEnabled(AttackControlGesture::GESTURE_TYPE_NONE);
		this->setNextTutorialProcess(TUTORIAL_PROCESS_PICKUP_ALL_ITEMS, 8.0f);
		break;
	case TUTORIAL_PROCESS_EQUIPMENT_DESC:
		this->setNextTutorialProcess(TUTORIAL_PROCESS_CONTROL_SUPER_ATTACK, 8.f);
		break;
	case TUTORIAL_PROCESS_CONTROL_SUPER_ATTACK:
		if (AttackStick* stick = m_scene->getGamePad()->getAttackStick())
			stick->setControlGestureEnabled(AttackControlGesture::GESTURE_TYPE_LONG_PRESS, 2.0f);
		break;
	case TUTORIAL_PROCESS_USE_ITEM:
		this->showControlWithSlideIn(m_scene->getInventoryBar(), 0.2f, 0.f, SLIDEIN_FROM_BOTTOM);
		if (AttackStick* stick = m_scene->getGamePad()->getAttackStick())
			stick->setControlGestureEnabled(AttackControlGesture::GESTURE_TYPE_NONE);
		break;
	case TUTORIAL_PROCESS_FIND_ENEMY:
		this->saveTutorialProcess(TUTORIAL_PROCESS_FIND_ENEMY);
		break;
	case TUTORIAL_PROCESS_OBJECTIVE:
		this->saveTutorialProcess(TUTORIAL_PROCESS_OBJECTIVE);
		this->hideTutorial(6.f);
		this->playNext(7.f);
		break;
	case TUTORIAL_PROCESS_COLLECT_MAGIC_BEANS:
		this->saveTutorialProcess(TUTORIAL_PROCESS_COLLECT_MAGIC_BEANS);
		this->hideTutorial(8.f);
		this->showControlWithSlideIn(m_scene->getStatusBar(), 0.2f, 9.f, SLIDEIN_FROM_LEFT);
		this->setNextTutorialProcess(TUTORIAL_PROCESS_SHOW_EQUIPMENT_BAR, 14.f);
		break;
	case TUTORIAL_PROCESS_SHOW_EQUIPMENT_BAR:
		this->saveTutorialProcess(TUTORIAL_PROCESS_SHOW_EQUIPMENT_BAR);
		this->showControlWithFadeIn(m_scene->getEquipmentButton(), 0.2f, 0.f);
		this->hideTutorial(5.f);
		break;
	case TUTORIAL_PROCESS_SEND_EMO:
		this->saveTutorialProcess(TUTORIAL_PROCESS_SEND_EMO);
		this->showControlWithFadeIn(m_scene->getSmileyBox(), 0.2f, 0.f);
		this->hideTutorial(5.f);
		break;
	default:
		break;
	}
}

void TutorialLayer::saveTutorialProcess(TutorialProcess process)
{
	auto localPlayer = sGameCenter->getLocalPlayer();
	localPlayer->setTutorialProcess(process);
	localPlayer->saveDataAsync();
}

void TutorialLayer::showTutorial(TutorialProcess process)
{
	auto visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();
	switch (process)
	{
	case TUTORIAL_PROCESS_CONTROL_MOVE:
	case TUTORIAL_PROCESS_CONTROL_ATTACK:
	case TUTORIAL_PROCESS_STAMINA_BAR_DESC:
	case TUTORIAL_PROCESS_PICKUP_ALL_ITEMS:
	case TUTORIAL_PROCESS_EQUIPMENT_DESC:
	case TUTORIAL_PROCESS_CONTROL_SUPER_ATTACK:
	case TUTORIAL_PROCESS_USE_ITEM:
	case TUTORIAL_PROCESS_FIND_ENEMY:
	case TUTORIAL_PROCESS_OBJECTIVE:
	case TUTORIAL_PROCESS_COLLECT_MAGIC_BEANS:
		m_tutorialView->setPositionX(origin.x + m_safeInsetLeft);
		m_tutorialView->setPositionY(origin.y + visibleSize.height - 85);
		m_tutorialView->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
		break;
	case TUTORIAL_PROCESS_SHOW_EQUIPMENT_BAR:
		m_tutorialView->setPositionX(m_scene->getEquipmentButton()->getBoundingBox().getMinX() + 12);
		m_tutorialView->setPositionY(m_scene->getEquipmentButton()->getBoundingBox().getMidY());
		m_tutorialView->setAnchorPoint(Point::ANCHOR_MIDDLE_RIGHT);
		break;
	case TUTORIAL_PROCESS_SEND_EMO:
		m_tutorialView->setPositionX(m_scene->getSmileyBox()->getBoundingBox().getMinX() + 12);
		m_tutorialView->setPositionY(m_scene->getSmileyBox()->getBoundingBox().getMidY());
		m_tutorialView->setAnchorPoint(Point::ANCHOR_MIDDLE_RIGHT);
		break;
	default:
		break;
	}
	m_tutorialView->setVisible(true);
	m_tutorialView->show(process);
}

void TutorialLayer::hideTutorial(float delay)
{
	if (!m_tutorialView->isShown())
		return;

	if(this->isScheduled(SCHEDULE_KEY_HIDE_TUTORIAL))
		this->unschedule(SCHEDULE_KEY_HIDE_TUTORIAL);

	auto callback = [this](float) {
		m_tutorialView->hide();
	};
	if (delay > 0)
		this->scheduleOnce(callback, delay, SCHEDULE_KEY_HIDE_TUTORIAL);
	else
		callback(0.f);
}

void TutorialLayer::showControlWithSlideIn(Node* control, float duration, float delay, SlideInDirection dir)
{
	if (control->isVisible())
		return;

	if (Action* action = control->getActionByTag(CONTROL_ACTION_TAG_SHOWING))
		control->stopAction(action);

	auto visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();

	Point dest = control->getPosition();
	switch (dir)
	{
	case SLIDEIN_FROM_LEFT:
		control->setPosition(origin.x - control->getContentSize().width, dest.y);
		break;
	case SLIDEIN_FROM_RIGHT:
		control->setPosition(origin.x + visibleSize.width + control->getContentSize().width, dest.y);
		break;
	case SLIDEIN_FROM_BOTTOM:
		control->setPosition(dest.x, origin.y - control->getContentSize().height);
		break;
	default:
		break;
	}
	Vector<FiniteTimeAction*> arrayOfActions;
	if (delay > 0)
		arrayOfActions.pushBack(DelayTime::create(delay));
	arrayOfActions.pushBack(Show::create());
	FiniteTimeAction* move = EaseSineOut::create(MoveTo::create(duration, dest));
	arrayOfActions.pushBack(move);

	Sequence* action = Sequence::create(arrayOfActions);
	action->setTag(CONTROL_ACTION_TAG_SHOWING);
	control->runAction(action);
}

void TutorialLayer::showControlWithFadeIn(Node* control, float duration, float delay)
{
	if (control->isVisible())
		return;

	if (Action* action = control->getActionByTag(CONTROL_ACTION_TAG_SHOWING))
		control->stopAction(action);

	control->setOpacity(0);
	Vector<FiniteTimeAction*> arrayOfActions;
	if (delay > 0)
		arrayOfActions.pushBack(DelayTime::create(delay));
	arrayOfActions.pushBack(Show::create());
	arrayOfActions.pushBack(FadeTo::create(duration, 255));

	Sequence* action = Sequence::create(arrayOfActions);
	action->setTag(CONTROL_ACTION_TAG_SHOWING);
	control->runAction(action);
}

void TutorialLayer::playNext(float delay)
{
	if (this->isScheduled(SCHEDULE_KEY_PLAY_NEXT))
		return;

	auto callback = [this](float) {
		m_scene->stopWorldAndPlayNext();
	};
	if (delay > 0)
		this->scheduleOnce(callback, delay, SCHEDULE_KEY_PLAY_NEXT);
	else
		callback(0.f);
}


NS_END

