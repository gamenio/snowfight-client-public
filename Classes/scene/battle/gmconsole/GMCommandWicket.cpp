#include "GMCommandWicket.h"

#include "game/GameConfig.h"
#if USE_DEBUG_OPTION

#include "common/utils/StringUtility.h"
#include "game/World.h"


NS_BEGIN

#define FRAME_SIZE							Size(390, 140)

#define INPUTBOX_MARGIN_TOP					16

#define INPUTBOX_BG_MARGIN_LEFTRIGHT		9
#define INPUTBOX_BG_HEIGHT					25
#define INPUTBOX_MARGIN_LEFT				4
#define INPUTBOX_MARGIN_RIGHT				27

#define FRAMENAME_INPUTBOX_BG					"inputbox_bg.png"
#define FRAMENAME_INPUTBOX_CLEAN				"searchbox_clean.png"

using namespace cocos2d::ui;

#define MAX_NUMBER_OF_HISTORY_CMDS		            15
static std::vector<std::string> sHistoryCmdList;
static int32 sLastSentCmdIndex = -1;

GMCommandWicket* GMCommandWicket::create(Node* owner)
{
	GMCommandWicket *ret = new (std::nothrow) GMCommandWicket();
	if (ret && ret->init(owner))
	{
		ret->autorelease();
		return ret;
	}
	else
	{
		CC_SAFE_DELETE(ret);
		return nullptr;
	}
}

bool GMCommandWicket::init(Node* owner)
{
	if (!Wicket::init(owner, FRAME_SIZE, "GM COMMAND", true, true))
		return false;

	m_main = Node::create();
	this->setContent(m_main);

	this->addNegativeButton("Del", CC_CALLBACK_1(GMCommandWicket::buttonDelCallback, this));
	this->addNeutralButton("Up", CC_CALLBACK_1(GMCommandWicket::buttonPrevCallback, this));
	this->addNeutralButton("Down", CC_CALLBACK_1(GMCommandWicket::buttonNextCallback, this));
	this->addPositiveButton("Enter", CC_CALLBACK_1(GMCommandWicket::buttonEnterCallback, this));

	Size frameSize = m_main->getBoundingBox().size;

	ui::Scale9Sprite* searchBoxBg = ui::Scale9Sprite::createWithSpriteFrameName(FRAMENAME_INPUTBOX_BG);
	searchBoxBg->setPosition(Vec2(INPUTBOX_BG_MARGIN_LEFTRIGHT, frameSize.height - INPUTBOX_MARGIN_TOP));
	searchBoxBg->setContentSize(Size(frameSize.width - INPUTBOX_BG_MARGIN_LEFTRIGHT * 2, INPUTBOX_BG_HEIGHT));
	searchBoxBg->setAnchorPoint(Point::ANCHOR_TOP_LEFT);
	m_main->addChild(searchBoxBg);

	m_editBox = EditBox::create(Size(searchBoxBg->getContentSize().width - INPUTBOX_MARGIN_LEFT - INPUTBOX_MARGIN_RIGHT, searchBoxBg->getContentSize().height), Scale9Sprite::create());
	m_editBox->setAnchorPoint(Point::ANCHOR_MIDDLE_LEFT);
	m_editBox->setPosition(Vec2(searchBoxBg->getPositionX() + INPUTBOX_MARGIN_LEFT, searchBoxBg->getPositionY() - searchBoxBg->getContentSize().height / 2));
	m_editBox->setTextHorizontalAlignment(TextHAlignment::LEFT);
	m_editBox->setFontSize(12);
	m_editBox->setFontColor(Color3B(150, 129, 56));
	m_editBox->setReturnType(ui::EditBox::KeyboardReturnType::DONE);
	m_editBox->setInputMode(ui::EditBox::InputMode::SINGLE_LINE);
	m_editBox->setDelegate(this);
	m_main->addChild(m_editBox);

	sLastSentCmdIndex = std::min(sLastSentCmdIndex, (int32)(sHistoryCmdList.size() - 1));
	if (sLastSentCmdIndex >= 0)
	{
		m_historyCmdIndex = sLastSentCmdIndex;
		m_editBox->setText(sHistoryCmdList.at(m_historyCmdIndex).c_str());
	}

	m_cleanInputBtn = Button::create(FRAMENAME_INPUTBOX_CLEAN, "", "", Widget::TextureResType::PLIST);
	m_cleanInputBtn->setAnchorPoint(Point::ANCHOR_MIDDLE_RIGHT);
	m_cleanInputBtn->setPosition(Vec2(searchBoxBg->getBoundingBox().getMaxX(), searchBoxBg->getPositionY() - searchBoxBg->getContentSize().height / 2));
	m_cleanInputBtn->addClickEventListener(CC_CALLBACK_1(GMCommandWicket::buttonCleanInputCallback, this));
	m_main->addChild(m_cleanInputBtn);
    this->updateCleanButtonVisibility();

	return true;
}


GMCommandWicket::GMCommandWicket() :
	m_main(nullptr),
	m_editBox(nullptr),
	m_cleanInputBtn(nullptr),
	m_historyCmdIndex(-1)
{
}

GMCommandWicket::~GMCommandWicket()
{
	m_main = nullptr;
	m_editBox = nullptr;
	m_cleanInputBtn = nullptr;
}

void GMCommandWicket::onEnter()
{
	Wicket::onEnter();

}

void GMCommandWicket::update(float delta)
{
}

void GMCommandWicket::sendCommand()
{
	std::string text = m_editBox->getText();
	text = StringUtility::trim(text);
	if (text.empty())
		return;

	std::string token;
	std::istringstream input(text);
	while (std::getline(input, token, ';')) {
		if (!token.empty())
		{
			std::string trimmed = StringUtility::trim(token);
			World::getInstance()->sendGMCommand(trimmed);
		}

	}

	m_historyCmdIndex = this->addCmdToHistoryList(text);
	sLastSentCmdIndex = m_historyCmdIndex;
}

int32 GMCommandWicket::addCmdToHistoryList(std::string const& cmd)
{
	int32 nCmds = (int32)sHistoryCmdList.size();
	for (int32 i = 0; i < nCmds; i++)
	{
		std::string c = sHistoryCmdList.at(i);
		if (c == cmd)
			return i;
	}

	if (nCmds >= MAX_NUMBER_OF_HISTORY_CMDS)
		sHistoryCmdList.erase(sHistoryCmdList.begin());
	sHistoryCmdList.push_back(cmd);

	int32 lastIndex = (int32)(sHistoryCmdList.size() - 1);
	return lastIndex;
}

void GMCommandWicket::nextCommand()
{
	if (sHistoryCmdList.empty())
		return;

	if (m_historyCmdIndex < 0)
		m_historyCmdIndex = 0;
	else
	{
		int32 nCmds = (int32)sHistoryCmdList.size();
		if (m_historyCmdIndex < nCmds - 1)
			m_historyCmdIndex++;
		else
			m_historyCmdIndex = 0;
	}

	std::string cmd = sHistoryCmdList.at(m_historyCmdIndex);
	m_editBox->setText(cmd.c_str());
	this->updateCleanButtonVisibility();
}

void GMCommandWicket::prevCommand()
{
	if (sHistoryCmdList.empty())
		return;

	if (m_historyCmdIndex < 0)
		m_historyCmdIndex = 0;
	else
	{
		if (m_historyCmdIndex > 0)
			m_historyCmdIndex--;
		else
		{
			int32 nCmds = (int32)sHistoryCmdList.size();
			m_historyCmdIndex = nCmds - 1;
		}
	}


	std::string cmd = sHistoryCmdList.at(m_historyCmdIndex);
	m_editBox->setText(cmd.c_str());
	this->updateCleanButtonVisibility();
}

void GMCommandWicket::delCommand()
{
	if (sHistoryCmdList.empty() || m_historyCmdIndex < 0)
		return;

	auto delIt = sHistoryCmdList.begin() + m_historyCmdIndex;
	sHistoryCmdList.erase(delIt);
	if (sHistoryCmdList.empty())
	{
		sLastSentCmdIndex = m_historyCmdIndex = -1;
		m_editBox->setText("");
	}
	else
	{
		int32 nCmds = (int32)sHistoryCmdList.size();
		sLastSentCmdIndex = m_historyCmdIndex = std::min(nCmds - 1, m_historyCmdIndex);
		std::string cmd = sHistoryCmdList.at(m_historyCmdIndex);
		m_editBox->setText(cmd.c_str());
	}
	this->updateCleanButtonVisibility();
}

void GMCommandWicket::buttonDelCallback(Ref* sender)
{
	this->delCommand();
}

void GMCommandWicket::buttonPrevCallback(Ref* sender)
{
	this->prevCommand();
}

void GMCommandWicket::buttonNextCallback(Ref* sender)
{
	this->nextCommand();
}

void GMCommandWicket::buttonCleanInputCallback(Ref* sender)
{
	m_editBox->setText("");
	this->updateCleanButtonVisibility();
}

void GMCommandWicket::buttonEnterCallback(Ref* sender)
{
	this->sendCommand();
	this->close();
}

void GMCommandWicket::updateCleanButtonVisibility()
{
	std::string text = m_editBox->getText();
	if (text.empty())
		m_cleanInputBtn->setVisible(false);
	else
		m_cleanInputBtn->setVisible(true);
}

void GMCommandWicket::editBoxTextChanged(ui::EditBox* editBox, std::string const& text)
{
	this->updateCleanButtonVisibility();
}

void GMCommandWicket::editBoxReturn(ui::EditBox* editBox)
{
}

void GMCommandWicket::onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event)
{
	switch (keyCode)
	{
	case EventKeyboard::KeyCode::KEY_ENTER:
		this->sendCommand();
		this->close();
		event->stopPropagation();
		break;
	case EventKeyboard::KeyCode::KEY_UP_ARROW:
		this->prevCommand();
		break;
	case EventKeyboard::KeyCode::KEY_DOWN_ARROW:
		this->nextCommand();
		break;
	case EventKeyboard::KeyCode::KEY_DELETE:
		this->delCommand();
		break;
	default:
		break;
	}
	Wicket::onKeyReleased(keyCode, event);
}

NS_END

#endif // USE_DEBUG_OPTION
