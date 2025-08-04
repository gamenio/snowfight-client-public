#ifndef __INPUT_BOX_H__
#define __INPUT_BOX_H__

#include "ui/UIEditBox/UIEditBox.h"
#include "ui/UIScale9Sprite.h"

#include "common/Common.h"

USING_NS_CC;

NS_BEGIN

enum InputBoxEditEvent
{
	INPUTBOX_EVENT_EDITING_DID_BEGIN,
	INPUTBOX_EVENT_EDITING_DID_END,
	INPUTBOX_EVENT_RETURN,
	INPUTBOX_EVENT_TEXT_CHANGED
};
typedef std::function<void(Ref*, InputBoxEditEvent)> InputBoxEditCallback;

class InputBox: public ui::EditBox, public ui::EditBoxDelegate
{
public:
    InputBox();
    ~InputBox();
    
    static InputBox* create(cocos2d::Size const& size, std::string const& normal9SpriteBg, TextureResType texType = Widget::TextureResType::LOCAL);
	static InputBox* create(cocos2d::Size const& size);
    bool init(cocos2d::Size const& size, std::string const& normal9SpriteBg, TextureResType texType);
	bool init(cocos2d::Size const& size);

	void onEnter() override;

    void editBoxEditingDidBegin(EditBox* editBox) override;
    void editBoxEditingDidEndWithAction(EditBox* editBox, EditBoxEndAction action) override;
    void editBoxReturn(EditBox* editBox) override;
    void editBoxTextChanged(EditBox* editBox, std::string const& text) override;

	void setMiddlePlaceholderFontColor(Color4B const& color);
	void setMiddlePlaceHolder(std::string const& text);
	void setMiddlePlaceholderFont(std::string const& systemFont, float fontSize);
    void setText(const char* pText);
    
	// Get and set the maximum length of content.
	// In content length units, English characters count as 1 unit, while Chinese characters and other characters count as 2 units.
    int32 getMaxContentLength() const { return m_maxContentLength; }
    void setMaxContentLength(int32 length) { m_maxContentLength = length; }
	// Calculate the content length of a UTF8 string
    static int32 calcContentLength(std::string const& utf8str);

    void setEditEventListener(InputBoxEditCallback const& callback) { m_editCallback = callback; }
    
private:
	void enableMiddlePlaceholder();
    
    int32 m_maxContentLength;
	bool m_enableMiddlePlaceholder;
	Label* m_middlePlaceholderLabel;
    
    InputBoxEditCallback m_editCallback;
};

NS_END

#endif // __INPUT_BOX_H__
