#ifndef __CHECK_LABEL_BOX_H__
#define __CHECK_LABEL_BOX_H__

#include "ui/UICheckBox.h"

#include "common/Common.h"

USING_NS_CC;


NS_BEGIN

class CheckLabelBox: public Node
{
public:
	enum class EventType
	{
		SELECTED,
		UNSELECTED
	};

	typedef std::function<void(Ref*, CheckLabelBox::EventType)> CheckLabelBoxCallback;

	static CheckLabelBox* create(std::string const& title);

	static CheckLabelBox* create(std::string const& backGround,
		std::string const& cross,
		std::string const& title,
		ui::Widget::TextureResType texType = ui::Widget::TextureResType::LOCAL);

	static CheckLabelBox* create(std::string const& backGround,
		std::string const& backGroundSelected,
		std::string const& cross,
		std::string const& backGroundDisabled,
		std::string const& frontCrossDisabled,
		std::string const& title,
		ui::Widget::TextureResType texType = ui::Widget::TextureResType::LOCAL);

	bool init(std::string const& backGround,
		std::string const& backGroundSelected,
		std::string const& cross,
		std::string const& backGroundDisabled,
		std::string const& frontCrossDisabled,
		std::string const& title,
		ui::Widget::TextureResType texType);

	CheckLabelBox();
	~CheckLabelBox();

	Label* getLabel() const { return m_label; }
	ui::CheckBox* getCheckBox() const { return m_checkBox; }

	void setEnabled(bool enabled) { m_checkBox->setEnabled(enabled); }
	bool isEnabled() const { return m_checkBox->isEnabled(); }

	void setSelected(bool selected) { m_checkBox->setSelected(selected); }
	bool isSelected() const { return m_checkBox->isSelected(); }

	virtual bool onTouchBegan(Touch* touch, Event* unusedEvent);
	virtual void onTouchMoved(Touch* touch, Event* unusedEvent);
	virtual void onTouchEnded(Touch* touch, Event* unusedEvent);
	virtual void onTouchCancelled(Touch* touch, Event* unusedEvent);

	void addEventListener(CheckLabelBoxCallback const& callback);

	void layoutElements();
private:
	bool hitTest(Vec2 const& pt, Camera const* camera, Vec3* p) const;

	EventListenerTouchOneByOne* m_touchListener;
	bool m_hitted;
	Camera const* m_hittedByCamera;

	ui::CheckBox* m_checkBox;
	Label* m_label;

	CheckLabelBoxCallback m_checkLabelBoxCallback;
};


NS_END


#endif // __CHECK_LABEL_BOX_H__

