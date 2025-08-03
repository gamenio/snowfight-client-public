#ifndef __MAGIC_BEAN_H__
#define __MAGIC_BEAN_H__

#include "common/Common.h"
#include "game/entities/ObjectGuid.h"
#include "scene/gui/silhouette/SILSprite.h"
#include "scene/gui/silhouette/SILLabel.h"

USING_NS_CC;

NS_BEGIN

class MagicBean : public Node
{
public:
	static MagicBean* create();

	MagicBean();
	~MagicBean();

	bool init() override;

	void setCount(int32 count);

	void setGlobalZOrder(float globalZOrder) override;
	void visit(Renderer* renderer, Mat4 const& parentTransform, uint32_t parentFlags) override;

private:
	void layoutElements();

	bool m_isLayoutDirty;
	SILSprite* m_iconSp;
	SILLabel* m_countLabel;

};

NS_END


#endif // __MAGIC_BEAN_H__
