#ifndef __SIL_CLIPPING_NODE_H__
#define __SIL_CLIPPING_NODE_H__

#include "common/Common.h"
#include "SilhouetteLayer.h"

USING_NS_CC;

NS_BEGIN

class SILClippingNode : public ClippingNode, public SilhouettableProtocol
{
public:
	static SILClippingNode* create();
	static SILClippingNode* create(Node *stencil);

	SILClippingNode();
	~SILClippingNode();

	bool init() override;
	bool init(Node* stencil) override;

	bool isSilhouetted() const override { return m_isSilhouetted; }
	void setSilhouetted(bool silhouetted) override;

	GLubyte getEffectOpacity() const override { return 0; }
	void setEffectOpacity(GLubyte opacity) override {}
	void setScreenTexture(Texture2D* texture) override {}

	void visit(Renderer* renderer, Mat4 const& parentTransform, uint32_t parentFlags) override;

private:
	bool m_isSilhouetted;

	GroupCommand m_maskGroupCommand;
	GroupCommand m_effectGroupCommand;
};

NS_END

#endif // __SIL_CLIPPING_NODE_H__
