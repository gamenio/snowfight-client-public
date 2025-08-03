#include "TutorialView.h"

#include "game/GameConfig.h"
#include "game/LocaleMgr.h"
#include "scene/SoundMgr.h"
#include "scene/Utils.h"

NS_BEGIN

using namespace cocos2d::ui;

#define BUBBLE_PADDING							13.0f

#define TEXT_MARGIN_TOP							8.f
#define TEXT_MARGIN_BOTTOM						11.f
#define TEXT_MARGIN_LEFTRIGHT					11.0f

#define JOYSTICK_ANIM_NUMBER_OF_FRAMES			2
#define JOYSTICK_ANIM_DEFAULT_FRAMEINDEX		0
#define JOYSTICK_ANIM_FRAMENAME_FORMAT			"tutorial_joystick%02d.png"
#define JOYSTICK_ANIM_DELAY_PER_FRAME			0.18f

#define FRAMENAME_CHAR							"tutorial_char.png"
#define FRAMENAME_BUBBLE_BG						"battle_bubble_bg.png"
#define FRAMENAME_BUBBLE_ARROW_LEFT				"battle_bubble_arrow_left.png"
#define FRAMENAME_ITEMBOX						"tutorial_itembox.png"
#define FRAMENAME_EQUIPMENT						"tutorial_equipment.png"
#define FRAMENAME_CONSUMABLE_ITEM				"tutorial_consumable_item.png"
#define FRAMENAME_MAGICBEAN						"tutorial_magicbean.png"

#define ZORDER_BUBBLE_ARROW							1
#define ZORDER_BUBBLE_TEXT							2
#define ZORDER_BUBBLE_ICON							3

TutorialView* TutorialView::create()
{
	auto ret = new (std::nothrow) TutorialView();
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

bool TutorialView::init()
{
	if (!Node::init())
		return false;

	this->setIgnoreAnchorPointForPosition(false);

	m_characterSp = Sprite::createWithSpriteFrameName(FRAMENAME_CHAR);
	m_characterSp->setVisible(false);
	this->addChild(m_characterSp);

	m_bubbleSp = Scale9Sprite::createWithSpriteFrameName(FRAMENAME_BUBBLE_BG);
	m_bubbleSp->setVisible(false);
	this->addChild(m_bubbleSp);

	m_arrowSp = Sprite::createWithSpriteFrameName(FRAMENAME_BUBBLE_ARROW_LEFT);
	m_bubbleSp->addChild(m_arrowSp, ZORDER_BUBBLE_ARROW);

	m_iconSp = Sprite::create();
	m_bubbleSp->addChild(m_iconSp, ZORDER_BUBBLE_ICON);

	return true;
}

TutorialView::TutorialView() :
	m_characterStartX(0),
	m_shownProcess(TUTORIAL_PROCESS_NONE),
	m_style(TUTORIAL_STYLE_NONE),
	m_bubbleSp(nullptr),
	m_arrowSp(nullptr),
	m_characterSp(nullptr),
	m_iconSp(nullptr),
	m_iconAnimation(nullptr),
	m_textLabel(nullptr)
{

}

TutorialView::~TutorialView()
{
	CC_SAFE_RELEASE_NULL(m_iconAnimation);

	m_bubbleSp = nullptr;
	m_arrowSp = nullptr;
	m_characterSp = nullptr;
	m_iconSp = nullptr;
	m_textLabel = nullptr;
}

void TutorialView::show(TutorialProcess process)
{
	if (process == TUTORIAL_PROCESS_NONE)
		return;

	if (m_shownProcess != process)
	{
		m_shownProcess = process;
		if (m_style == TUTORIAL_STYLE_ANNOUNCER && m_bubbleSp->isVisible())
		{
			this->startHideAnnouncerBubbleAnimation([this]() {
				this->setupTutorial();
				this->startShowAnimation();
			});
		}
		else
		{
			this->setupTutorial();
			this->startShowAnimation();
		}
	}
}

void TutorialView::hide()
{
	if (m_shownProcess == TUTORIAL_PROCESS_NONE)
		return;

	this->startHideAnimation();
	m_shownProcess = TUTORIAL_PROCESS_NONE;
	m_style = TUTORIAL_STYLE_NONE;
}

void TutorialView::setupTutorial()
{
	m_characterSp->stopAllActions();
	m_bubbleSp->stopAllActions();
	if(m_iconSp->isVisible())
		m_iconSp->stopAllActions();

	if (m_iconAnimation)
		CC_SAFE_RELEASE_NULL(m_iconAnimation);

	switch (m_shownProcess)
	{
	case TUTORIAL_PROCESS_CONTROL_MOVE:
		this->setText(sLocaleMgr->getString("tutorial_control_move_text"));
		this->setTutorialStyle(TUTORIAL_STYLE_ANNOUNCER);
		this->setIconSpriteFrame(FRAMENAME_MAGICBEAN);
		break;
	case TUTORIAL_PROCESS_CONTROL_ATTACK:
		this->setText(sLocaleMgr->getString("tutorial_control_attack_text"));
		this->setTutorialStyle(TUTORIAL_STYLE_ANNOUNCER);
		this->setupIconAnimation(JOYSTICK_ANIM_FRAMENAME_FORMAT, JOYSTICK_ANIM_NUMBER_OF_FRAMES, JOYSTICK_ANIM_DELAY_PER_FRAME, JOYSTICK_ANIM_DEFAULT_FRAMEINDEX);
		break;
	case TUTORIAL_PROCESS_STAMINA_BAR_DESC:
		this->setText(sLocaleMgr->getString("tutorial_stamina_bar_desc_text"));
		this->setTutorialStyle(TUTORIAL_STYLE_ANNOUNCER);
		break;
	case TUTORIAL_PROCESS_PICKUP_ALL_ITEMS:
		this->setText(sLocaleMgr->getString("tutorial_pickup_all_items_text"));
		this->setTutorialStyle(TUTORIAL_STYLE_ANNOUNCER);
		break;
	case TUTORIAL_PROCESS_EQUIPMENT_DESC:
		this->setText(sLocaleMgr->getString("tutorial_equipment_desc_text"));
		this->setTutorialStyle(TUTORIAL_STYLE_ANNOUNCER);
		this->setIconSpriteFrame(FRAMENAME_EQUIPMENT);
		break;
	case TUTORIAL_PROCESS_CONTROL_SUPER_ATTACK:
		this->setText(sLocaleMgr->getString("tutorial_control_supper_attack_text"));
		this->setTutorialStyle(TUTORIAL_STYLE_ANNOUNCER);
		this->setupIconAnimation(JOYSTICK_ANIM_FRAMENAME_FORMAT, JOYSTICK_ANIM_NUMBER_OF_FRAMES, JOYSTICK_ANIM_DELAY_PER_FRAME, JOYSTICK_ANIM_DEFAULT_FRAMEINDEX);
		break;
	case TUTORIAL_PROCESS_USE_ITEM:
		this->setText(sLocaleMgr->getString("tutorial_use_item_text"));
		this->setTutorialStyle(TUTORIAL_STYLE_ANNOUNCER);
		this->setIconSpriteFrame(FRAMENAME_CONSUMABLE_ITEM);
		break;
	case TUTORIAL_PROCESS_FIND_ENEMY:
		this->setText(sLocaleMgr->getString("tutorial_find_enemy_text"));
		this->setTutorialStyle(TUTORIAL_STYLE_ANNOUNCER);
		break;
	case TUTORIAL_PROCESS_OBJECTIVE:
		this->setText(sLocaleMgr->getString("tutorial_objective_text"));
		this->setTutorialStyle(TUTORIAL_STYLE_ANNOUNCER);
		break;
	case TUTORIAL_PROCESS_COLLECT_MAGIC_BEANS:
		this->setText(sLocaleMgr->getString("tutorial_collect_magicbeans_text"));
		this->setTutorialStyle(TUTORIAL_STYLE_ANNOUNCER);
		this->setIconSpriteFrame(FRAMENAME_MAGICBEAN);
		break;
	case TUTORIAL_PROCESS_SHOW_EQUIPMENT_BAR:
		this->setText(sLocaleMgr->getString("tutorial_show_equipment_bar_text"));
		this->setTutorialStyle(TUTORIAL_STYLE_BUBBLE_RIGHT);
		break;
	case TUTORIAL_PROCESS_SEND_EMO:
		this->setText(sLocaleMgr->getString("tutorial_sendemo_text"));
		this->setTutorialStyle(TUTORIAL_STYLE_BUBBLE_RIGHT);
		break;
	default:
		break;
	}

	this->layoutElements();
	this->setupElementsForAction();
}

void TutorialView::setTutorialStyle(TutorialStyle style)
{
	m_style = style;

	m_bubbleSp->setVisible(false);
	m_textLabel->setVisible(false);
	m_iconSp->setVisible(false);
	m_bubbleSp->setScale(1.0f);
	m_bubbleSp->setRotation(0.0f);

	if (m_style == TUTORIAL_STYLE_ANNOUNCER)
	{
		m_characterSp->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
		m_characterSp->setFlippedX(false);

		m_arrowSp->setFlippedX(false);
		m_arrowSp->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);

		m_textLabel->setVisible(true);
		m_textLabel->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);

		m_iconSp->setAnchorPoint(Point::ANCHOR_BOTTOM_RIGHT);
	}
	else
	{
		m_characterSp->setVisible(false);

		m_textLabel->setVisible(true);
		m_textLabel->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);

		m_bubbleSp->setAnchorPoint(Point::ANCHOR_BOTTOM_RIGHT);
		m_arrowSp->setFlippedX(true);
		m_arrowSp->setAnchorPoint(Point::ANCHOR_BOTTOM_RIGHT);
	}
}

void TutorialView::setIconSpriteFrame(std::string const& frameName)
{
	m_iconSp->setVisible(true);
	m_iconSp->setSpriteFrame(frameName);
}

void TutorialView::setupIconAnimation(std::string const& frameNameFormat, int32 numberOfFrames, float delayPerFrame, int32 defaultFrameIndex)
{
	m_iconAnimation = Animation::create();
	CC_SAFE_RETAIN(m_iconAnimation);
	for (int i = 0; i < numberOfFrames; ++i)
	{
		std::string frameName = StringUtils::format(frameNameFormat.c_str(), i);
		m_iconAnimation->addSpriteFrame(SpriteFrameCache::getInstance()->getSpriteFrameByName(frameName));
	}
	m_iconAnimation->setDelayPerUnit(delayPerFrame);

	m_iconSp->setVisible(true);
	m_iconSp->setSpriteFrame(m_iconAnimation->getFrames().at(defaultFrameIndex)->getSpriteFrame());
}

void TutorialView::layoutElements()
{
	if (m_style == TUTORIAL_STYLE_ANNOUNCER)
	{
		m_characterSp->setPosition(0, 6);

		m_textLabel->setPosition(Vec2(BUBBLE_PADDING + TEXT_MARGIN_LEFTRIGHT, BUBBLE_PADDING + TEXT_MARGIN_BOTTOM));
		float bubbleWidth = m_textLabel->getBoundingBox().getMaxX() + BUBBLE_PADDING;
		float bubbleHeight = m_textLabel->getBoundingBox().getMaxY() + TEXT_MARGIN_TOP + BUBBLE_PADDING;
		if (!m_iconSp->isVisible())
			bubbleWidth += TEXT_MARGIN_LEFTRIGHT;
		else
		{
			bubbleWidth += m_iconSp->getBoundingBox().size.width;
			m_iconSp->setPosition(bubbleWidth - BUBBLE_PADDING, BUBBLE_PADDING);
		}
		m_arrowSp->setPosition(0, 10);
		m_bubbleSp->setContentSize(Size(bubbleWidth, bubbleHeight));
		m_bubbleSp->setAnchorPoint(Vec2(0.0f, m_arrowSp->getBoundingBox().getMidY() / m_bubbleSp->getBoundingBox().size.height));
		m_bubbleSp->setPosition(m_characterSp->getBoundingBox().getMaxX() - 20, m_bubbleSp->getBoundingBox().size.height * m_bubbleSp->getAnchorPoint().y);

		this->setContentSize(Size(m_bubbleSp->getBoundingBox().getMaxX(), std::max(m_characterSp->getBoundingBox().getMaxY(), m_bubbleSp->getBoundingBox().getMaxY())));
	}
	else
	{
		m_textLabel->setPosition(Vec2(BUBBLE_PADDING + TEXT_MARGIN_LEFTRIGHT, BUBBLE_PADDING + TEXT_MARGIN_BOTTOM));
		float bubbleWidth = m_textLabel->getBoundingBox().getMaxX() + TEXT_MARGIN_LEFTRIGHT + BUBBLE_PADDING;
		float bubbleHeight = m_textLabel->getBoundingBox().getMaxY() + TEXT_MARGIN_TOP + BUBBLE_PADDING;
		m_arrowSp->setPosition(bubbleWidth, 10);
		m_bubbleSp->setContentSize(Size(bubbleWidth, bubbleHeight));
		m_bubbleSp->setAnchorPoint(Vec2(1.0f, m_arrowSp->getBoundingBox().getMidY() / m_bubbleSp->getBoundingBox().size.height));
		m_bubbleSp->setPosition(bubbleWidth, m_bubbleSp->getBoundingBox().size.height * m_bubbleSp->getAnchorPoint().y);

		this->setContentSize(Size(m_bubbleSp->getBoundingBox().getMaxX(), m_bubbleSp->getBoundingBox().getMaxY()));
	}
}

void TutorialView::setupElementsForAction()
{
	if (m_style == TUTORIAL_STYLE_ANNOUNCER)
	{
		m_characterStartX = -m_characterSp->getBoundingBox().size.width;

		m_textLabel->setVisible(false);
		if(m_iconSp->isVisible())
			m_iconSp->setOpacity(0);

		if (!m_characterSp->isVisible())
			m_characterSp->setPositionX(m_characterStartX);
		m_bubbleSp->setScale(0.0f);
		m_bubbleSp->setRotation(-20.0f);
	}
	else
	{
		m_characterStartX = 0;
		m_textLabel->setVisible(false);
		m_bubbleSp->setScale(0.0f);
		m_bubbleSp->setRotation(20.0f);
	}

}

void TutorialView::startShowAnimation()
{
	if (m_style == TUTORIAL_STYLE_ANNOUNCER)
	{
		m_characterSp->stopAllActions();
		m_bubbleSp->stopAllActions();
		if (m_iconSp->isVisible())
			m_iconSp->stopAllActions();

		Vector<FiniteTimeAction*> bubbleActions;
		if(!m_characterSp->isVisible())
		{
			FiniteTimeAction* move = EaseSineOut::create(MoveTo::create(0.3f, Vec2(0.f, m_characterSp->getPositionY())));
			Sequence* seq = Sequence::create(Show::create(), move, nullptr);
			m_characterSp->runAction(seq);
            sSoundMgr->play(SOUND_CHARACTER_POP);

			bubbleActions.pushBack(DelayTime::create(0.5f));
		}
		else
			bubbleActions.pushBack(DelayTime::create(0.1f));

		NS_ASSERT(!m_bubbleSp->isVisible());
		bubbleActions.pushBack(Show::create());
		FiniteTimeAction* scale = EaseSineOut::create(ScaleTo::create(0.3f, 1.0f));
		FiniteTimeAction* rotate = EaseSineOut::create(RotateTo::create(0.3f, 0.0f));
		CallFunc* startCallfunc = CallFunc::create([this]() {
			sSoundMgr->play(SOUND_BUBBLE_POP);
		});
		bubbleActions.pushBack(Spawn::create(startCallfunc, scale, rotate, nullptr));
		CallFunc* endCallfunc = CallFunc::create([this]() {
			m_textLabel->setVisible(true);
			if(m_iconSp->isVisible())
				m_iconSp->setOpacity(255);
		});
		bubbleActions.pushBack(endCallfunc);
		Sequence* seq = Sequence::create(bubbleActions);
		m_bubbleSp->runAction(seq);

		if (m_iconAnimation)
		{
			Animate* animate = Animate::create(m_iconAnimation);
			Sequence* seq = Sequence::create(animate, animate->reverse(), nullptr);
			RepeatForever* repeat = RepeatForever::create(seq);
			m_iconSp->runAction(repeat);
		}
	}
	else
	{
		m_bubbleSp->stopAllActions();

		FiniteTimeAction* scale = EaseSineOut::create(ScaleTo::create(0.3f, 1.0f));
		FiniteTimeAction* rotate = EaseSineOut::create(RotateTo::create(0.3f, 0.0f));
		CallFunc* callfunc = CallFunc::create([this]() {
			m_textLabel->setVisible(true);
		});
		Sequence* seq = Sequence::create(Show::create(), Spawn::create(scale, rotate, nullptr), callfunc, nullptr);
		m_bubbleSp->runAction(seq);
        sSoundMgr->play(SOUND_BUBBLE_POP);
	}
}

void TutorialView::startHideAnimation()
{
	if (m_style == TUTORIAL_STYLE_ANNOUNCER)
	{
		m_characterSp->stopAllActions();

		this->startHideAnnouncerBubbleAnimation();

		MoveTo* move = MoveTo::create(0.3f, Vec2(m_characterStartX, m_characterSp->getPositionY()));
		EaseSineIn* ease = EaseSineIn::create(move);
		Sequence* seq = Sequence::create(DelayTime::create(0.5f), ease, Hide::create(), nullptr);
		m_characterSp->runAction(seq);
	}
	else
	{
		m_bubbleSp->stopAllActions();
		m_textLabel->setVisible(false);

		FiniteTimeAction* scale = EaseSineOut::create(ScaleTo::create(0.25f, 0.0f));
		FiniteTimeAction* rotate = EaseSineOut::create(RotateTo::create(0.25f, 20.0f));
		Spawn* spawn = Spawn::create(scale, rotate, nullptr);
		Sequence* seq = Sequence::createWithTwoActions(spawn, Hide::create());
		m_bubbleSp->runAction(seq);
	}
}

void TutorialView::startHideAnnouncerBubbleAnimation(std::function<void()> complete)
{
	m_bubbleSp->stopAllActions();
	if (m_iconSp->isVisible())
	{
		m_iconSp->stopAllActions();
		m_iconSp->setOpacity(0);
	}
	m_textLabel->setVisible(false);

	FiniteTimeAction* scale = EaseSineOut::create(ScaleTo::create(0.25f, 0.0f));
	FiniteTimeAction* rotate = EaseSineOut::create(RotateTo::create(0.25f, -20.0f));
	Spawn* spawn = Spawn::create(scale, rotate, nullptr);
	Vector<FiniteTimeAction*> actions;
	actions.pushBack(spawn);
	if (complete)
		actions.pushBack(CallFunc::create(complete));
	actions.pushBack(Hide::create());
	Sequence* seq = Sequence::create(actions);
	m_bubbleSp->runAction(seq);
}

void TutorialView::setText(std::string const& text)
{
	if (m_textLabel)
	{
		m_textLabel->removeFromParentAndCleanup(true);
		m_textLabel = nullptr;
	}
	ValueMap defaults;
	defaults[RichText::KEY_FONT_SIZE] = 12;
	defaults[RichText::KEY_FONT_COLOR_STRING] = "#e6e8ef";
	defaults[RichText::KEY_FONT_FACE] = DEFAULT_SYSTEM_FONT;
	defaults[RichText::KEY_VERTICAL_SPACE] = 1.f;
	std::string xml = "<shadow color=\"#ffffff\" offsetWidth=\"0.4\" offsetHeight=\"0.0\" blurRadius=\"0\">";
	xml += text;
	xml += "</shadow>";

	m_textLabel = RichText::createWithXML(xml, defaults);
	m_textLabel->ignoreContentAdaptWithSize(true);
	m_bubbleSp->addChild(m_textLabel, ZORDER_BUBBLE_TEXT);

	m_textLabel->formatText();
	auto size = m_textLabel->getContentSize();
}


NS_END

