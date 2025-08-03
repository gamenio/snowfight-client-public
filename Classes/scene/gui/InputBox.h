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
    
    // 获取和设置内容的最大长度
    // 内容长度单位中的英文字符占1个单位，中文等字符占2个单位
    int32 getMaxContentLength() const { return m_maxContentLength; }
    void setMaxContentLength(int32 length) { m_maxContentLength = length; }
    // 计算UTF8字符串的内容长度
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
