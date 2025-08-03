#ifndef __FUNC_INPUT_BOX_H__
#define __FUNC_INPUT_BOX_H__

#include "common/Common.h"
#include "scene/gui/InputBox.h"

USING_NS_CC;

NS_BEGIN

class FuncNameInputBox : public Node
{
public:
	FuncNameInputBox();
	~FuncNameInputBox();

	static FuncNameInputBox* create();
	bool init();

	void setText(std::string const& text);
	std::string getText() const;
    
    InputBox* getInputBoxRender() const { return m_inputBox; }

	void setEnabled(bool enabled);
    void setEditEventListener(InputBoxEditCallback const& callback);

private:
	InputBox* m_inputBox;
};


NS_END

#endif // __FUNC_INPUT_BOX_H__
