#ifndef __TOASTER_H__
#define __TOASTER_H__

#include "common/Common.h"
#include "Toast.h"

USING_NS_CC;

NS_BEGIN

class Toaster : public Node
{
public:
	Toaster();
	~Toaster();

	static Toaster* create();
	bool init() override;

	void addToast(std::string const& text, MessageToast::Severity severity = MessageToast::INFO, float duration = TOAST_DURATION_SHORT, int32 tag = 0, bool flickering = false);
	void addToast(Toast* toast);
	void removeToastByTag(int32 tag);

private:
	void layoutElements();
	
	Node* m_mainContainer;
};


NS_END

#endif // __TOASTER_H__
