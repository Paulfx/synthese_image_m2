#ifndef RENDERER_H
#define RENDERER_H

#include "wavefront.h"
#include "texture.h"

#include "orbiter.h"
#include "draw.h"        
#include "app_time.h"        // classe Application a deriver
#include "program.h"
#include "uniforms.h"
#include "mesh.h"

#include "buffersClass.h"
#include "infos.h"

#include <vector>
#include <sstream>
#include <fstream>

class Renderer : public App {

public:
    // constructeur : donner les dimensions de l'image, et eventuellement la version d'openGL.
    Renderer( ) : App(1400, 900) {}

// creation des objets de l'application
    int init();

    void createPrograms();
    GLuint compileShader(const char* filename, const GLenum shader_type);
    void createProgramAndLinkShaders(GLuint &program, GLuint v_shader, GLuint f_shader);
    
    void createObjects();
    void addAnimatedObj(const std::string& beginFilePath, int nb);
    int addStaticObj(const std::string& filePath);

    void createDepthFrameBuffer();
    void createDepthBuffer(GLuint &buf);

    void createSSBO();
    void putLightsInSSBO();
    void putMaterialsInSSBO(const Buffers& obj);

//update de la scène
    int update(const float time, const float delta);
    void updateModels();
    
// dessiner une nouvelle image
    int render();

    Transform Ortho(const float left, const float right, const float bottom, const float top, const float znear, const float zfar);
    Transform getViewFromSun(vec3 pos);
    Transform getViewFromLight(vec3 pos);
    void shadowMapRender(const Transform& orthoProjShadowMap, const Transform &view);
    void normalRender();
    void renderAnimatedObj();
    void renderStaticObj();

    void setUniform(GLuint program, const Transform& model);

// destruction des objets de l'application
    int quit();

protected:
    
    const unsigned numberOfLights = 4;
    const unsigned max_materials = 100;
//Camera
    Orbiter m_camera;
//Lumière
    std::vector<Light> m_lights;
//Objets animés
    std::vector<Buffers> m_animatedObj;
    std::vector<Transform> m_animatedModels;
//Onjets statiques
    std::vector<Buffers> m_staticObj;
    std::vector<Transform> m_staticModels;

//Storage buffer light + material
    GLuint m_ssbo;
//Shaders d'affichage d'objets
    GLuint m_programDynamic;
    GLuint m_programStatic;
//Shadow mapping
    int m_framebuffer_width;
    int m_framebuffer_height;
    //2 projections orthographiques différentes en fonction de la lumière
    Transform orthoProjShadowMapSun;
    Transform orthoProjShadowMapLamp;
    GLuint m_staticShadowMap_program;
    GLuint m_dynamicShadowMap_program;
    GLuint m_framebuffer;
    GLuint m_depth_buffer1; //Soleil
    GLuint m_depth_buffer2; //Lamp1
    GLuint m_depth_buffer3; //Lamp2
    GLuint m_depth_buffer4; //Lamp3
    GLuint m_color_buffer;
    GLuint m_color_sampler;
};


#endif // RENDERER_H
