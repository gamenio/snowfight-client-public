//
// Created by Luthier on 2019/9/24.
//

#ifndef __GLPROGRAM_MANAGER_H__
#define __GLPROGRAM_MANAGER_H__

#include "common/Common.h"


USING_NS_CC;

NS_BEGIN

extern const char* SHADER_NAME_LIGHTNESS;
extern const char* SHADER_NAME_SILHOUETTABLE_NODE_MASK;
extern const char* SHADER_NAME_SILHOUETTABLE_NODE_MASK_NO_MVP;
extern const char* SHADER_NAME_SILHOUETTABLE_NODE_EFFECT;
extern const char* SHADER_NAME_SILHOUETTABLE_NODE_EFFECT_NO_MVP;
extern const char* SHADER_NAME_DISCARD_ALPHA;

extern const char* UNIFORM_NAME_MASK_COLOR ;
extern const char* UNIFORM_NAME_SOURCE_MASK_COLOR;
extern const char* UNIFORM_NAME_ALPHA_VALUE;
extern const char* UNIFORM_NAME_PREMULTIPLIED_ALPHA;
extern const char* UNIFORM_NAME_SCREEN_PIXEL_SIZE;
extern const char* UNIFORM_NAME_SCREEN_TEXTURE;

class GLProgramManager : public Ref
{
public:
    GLProgramManager();
    ~GLProgramManager();

    static GLProgramManager* getInstance();
    static void destroyInstance();

    void loadGLPrograms();
    GLProgram* getGLProgram(std::string const& key);

private:
    bool init();
    void reloadGLPrograms();
    void loadGLProgram(GLProgram *program, int type);

    std::unordered_map<std::string, GLProgram*> m_programs;
    EventListenerCustom* m_backToForegroundListener;
};

NS_END

#endif // __GLPROGRAM_MANAGER_H__
