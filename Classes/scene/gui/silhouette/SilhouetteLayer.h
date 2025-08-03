#ifndef __SILHOUETTE_LAYER_H__
#define __SILHOUETTE_LAYER_H__

#include "common/Common.h"

USING_NS_CC;

NS_BEGIN

class SilhouettableProtocol
{
public:
	virtual ~SilhouettableProtocol() {}

	virtual bool isSilhouetted() const = 0;
	virtual void setSilhouetted(bool silhouetted) = 0;

	virtual GLubyte getEffectOpacity() const = 0;
	virtual void setEffectOpacity(GLubyte opacity) = 0;

	virtual void setScreenTexture(Texture2D* texture) = 0;
};

class SilhouetteLayer : public Layer
{
public:
	static SilhouetteLayer* create();

	SilhouetteLayer();
	~SilhouetteLayer();

	bool init() override;

	Vector<Node*> const& getCovers() const { return m_covers; }
	void setCovers(Vector<Node*> const& covers);

	void setGlobalZOrderRange(float min, float max);

	void onEnter() override;
	void onEnterTransitionDidFinish() override;

	void onExit() override;
	void onExitTransitionDidStart() override;

	void addChild(Node* child) override;

	void visit(Renderer* renderer, Mat4 const& parentTransform, uint32_t parentFlags) override;

private:
	void setScreenTextureRecursively(Node* node, Texture2D* texture);
	void visitInternal(Renderer* renderer, uint32_t flags);

	Vector<Node*> m_covers;
	float m_minGlobalZOrder;
	float m_maxGlobalZOrder;

	RenderTexture* m_offscreenRender;
	GroupCommand m_groupCommand;
};

NS_END

#endif // __SILHOUETTE_LAYER_H__
