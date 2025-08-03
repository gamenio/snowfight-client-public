#ifndef __LOADING_INDICATOR_H__
#define __LOADING_INDICATOR_H__

#include "common/Common.h"


USING_NS_CC;

NS_BEGIN

class LoadingIndicator : public Node
{
public:
	static LoadingIndicator* create(cocos2d::Size const& size, Color3B const& tintColor = Color3B::WHITE);
	bool init(cocos2d::Size const& size, Color3B const& tintColor);

	LoadingIndicator();
	~LoadingIndicator();

	void setVisible(bool visible) override;
	void setTintColor(Color3B const& tintColor);

	void stopAnimation();
	void runAnimation();
	bool isAnimating() const { return m_isAnimating; }

private:
	bool m_isAnimating;
};

NS_END


#endif // __LOADING_INDICATOR_H__
