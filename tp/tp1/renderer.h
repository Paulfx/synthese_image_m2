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
    Renderer( ) : App(1024, 640) {}

    // creation des objets de l'application
    int init();

    void createPrograms();

    GLuint compileShader(const char* filename, const GLenum shader_type);
    void createProgramAndLinkShaders(GLuint &program, GLuint v_shader, GLuint f_shader);
    
    void createObjects();

    void createDepthFrameBuffer();

    void createSSBO();

    void putLightsInSSBO();

    void putMaterialsInSSBO(const Buffers& obj);
    
    // destruction des objets de l'application
    int quit();

    void addAnimatedObj(const std::string& beginFilePath, int nb);

    int addStaticObj(const std::string& filePath);

    int update(const float time, const float delta);
    

    void updateModels();

    // dessiner une nouvelle image
    int render();

    Transform Ortho(const float left, const float right, const float bottom, const float top, const float znear, const float zfar);

    void shadowMapRender();

    void normalRender();

    void renderAnimatedObj();

    void renderStaticObj();

    void setUniform(GLuint program, const Transform& model);

    Transform getViewFromLight(vec3 pos);

protected:
    
    //AnimationBuffers m_robot;

    GLuint m_ssbo;
    const int numberOfLights = 2;
    const int max_materials = 10;

    std::vector<Buffers> m_animatedObj;
    std::vector<Transform> m_animatedModels;

    std::vector<Buffers> m_staticObj;
    std::vector<Transform> m_staticModels;


    GLuint m_programDynamic;
    GLuint m_programStatic;
    Orbiter m_camera;

    //Lumière

    std::vector<Light> m_lights;


    int m_framebuffer_height = 1024;
    int m_framebuffer_width = 1024;

    Transform orthoProjShadowMap;

    GLuint m_staticShadowMap_program;
    GLuint m_dynamicShadowMap_program;
    GLuint m_framebuffer;
    GLuint m_depth_buffer;
    GLuint m_color_buffer;
    GLuint m_color_sampler;

    //Point lightPos[2];
    //Color lightColor[2];

    //Les modeles
    //Transform lampModel, robotModel;
};


#endif // RENDERER_H
