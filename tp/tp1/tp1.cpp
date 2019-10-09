//! \file tuto7.cpp reprise de tuto6.cpp mais en derivant App::init(), App::quit() et bien sur App::render().

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

class TP : public App {

public:
    // constructeur : donner les dimensions de l'image, et eventuellement la version d'openGL.
    TP( ) : App(1024, 640) {}
    
    // utilitaire : renvoie la chaine de caracteres pour un type glsl.
const char *glsl_string( const GLenum type )
{
    switch(type)
    {
        case GL_BOOL:
            return "bool";
        case GL_UNSIGNED_INT:
            return "uint";
        case GL_INT:
            return "int";
        case GL_FLOAT:
            return "float";
        case GL_FLOAT_VEC2:
            return "vec2";
        case GL_FLOAT_VEC3:
            return "vec3";
        case GL_FLOAT_VEC4:
            return "vec4";
        case GL_FLOAT_MAT4:
            return "mat4";

        default:
            return "";
    }
}

    int print_storage( const GLuint program )
{
    if(program == 0)
    {
        printf("[error] program 0, no storage buffers...\n");
        return -1;
    }
    
    // recupere le nombre de storage buffers
    GLint buffer_count= 0;
    glGetProgramInterfaceiv(program, GL_SHADER_STORAGE_BLOCK, GL_ACTIVE_RESOURCES, &buffer_count);
    if(buffer_count == 0)
        return 0;
    
    for(int i= 0; i < buffer_count; i++)
    {
        // recupere le nom du storage buffer
        char bname[1024]= { 0 };
        glGetProgramResourceName(program, GL_SHADER_STORAGE_BLOCK, i, sizeof(bname), NULL, bname);
        
        // et le binding
        GLint binding= 0;
        {
            GLenum prop[]= { GL_BUFFER_BINDING };
            glGetProgramResourceiv(program, GL_SHADER_STORAGE_BLOCK, i, 1, prop, 1, NULL, &binding);
        }

        printf("  buffer '%s' binding %d\n", bname, binding);
        
        // nombre de variables declarees
        GLint variable_count= 0;
        {
            GLenum prop[]= { GL_NUM_ACTIVE_VARIABLES };
            glGetProgramResourceiv(program, GL_SHADER_STORAGE_BLOCK, i, 1, prop, 1, NULL, &variable_count);
        }
        
        // identifidants des variables 
        std::vector<GLint> variables(variable_count);
        {
            GLenum prop[]= { GL_ACTIVE_VARIABLES };
            glGetProgramResourceiv(program, GL_SHADER_STORAGE_BLOCK, i, 1, prop, variable_count, NULL, variables.data());
        }
        
        for(int k= 0; k < variable_count; k++)
        {
            // organisation des variables dans le buffer
            GLenum props[]= { GL_OFFSET, GL_TYPE, GL_ARRAY_SIZE, GL_ARRAY_STRIDE, GL_MATRIX_STRIDE, GL_IS_ROW_MAJOR, GL_TOP_LEVEL_ARRAY_STRIDE };
            const int size= sizeof(props) / sizeof(GLenum);
            
            GLint params[size];
            glGetProgramResourceiv(program, GL_BUFFER_VARIABLE, variables[k], size, props, size, NULL, params);
            
            // nom de la variable
            char vname[1024]= { 0 };
            glGetProgramResourceName(program, GL_BUFFER_VARIABLE, variables[k], sizeof(vname), NULL, vname);
            
            printf("    '%s %s': offset %d", glsl_string(params[1]), vname, params[0]);
            if(params[2] > 1)
                printf(", array size %d", params[2]);
            
            printf(", stride %d", params[3]);
            
            // organisation des matrices
            if(params[1] == GL_FLOAT_MAT4 || params[1] == GL_FLOAT_MAT3) 
                printf(", %s, matrix stride %d", params[5] ? "row major" : "column major", params[4]);
            
            printf(", top level stride %d\n", params[6]);
        }
    }
    
    return 0;
}

    // creation des objets de l'application
    int init( )
    {
        // std::vector<Mesh> meshes;
        // const std::string beginFile = "tp/tp1/Robot/run/Robot_0000";
        // std::string meshFile, numStr;
        // for (int i = 1; i <= 23; ++i)
        // {
        //     if (i < 10) numStr = "0";
        //     else numStr = "";
        //     numStr += std::to_string(i);
        //     meshFile = beginFile + numStr + ".obj";
        //     meshes.push_back(read_mesh(meshFile.c_str()));
        // }

        // //Create all keyframes
        // m_robot.create(meshes);

        m_shadowMap_program = read_program("tp/tp1/shadowMap.glsl");
        program_print_errors(m_shadowMap_program);


        addAnimatedObj("tp/tp1/Robot/run/Robot_0000", 23);
        m_animatedModels.push_back(Translation(2,0,2));

        addAnimatedObj("tp/tp1/Dog/Dog_0000",39);
        m_animatedModels.push_back(Translation(6,0,0) * Scale(0.5,0.5,0.5));

        addAnimatedObj("tp/tp1/Wolf/Wolf_0000", 40);
        m_animatedModels.push_back(Translation(-6,0,0));

        m_programDynamic= read_program("tp/tp1/dynamic.glsl");
        program_print_errors(m_programDynamic);
        
        print_storage(m_programDynamic);

        //robotModel = Translation(2,0,2);
        //TODO in buffers
        //m_animatedModels[ROBOT] = Translation(2,0,2); 


        //Create the lamp
        // Mesh lamp = read_mesh("tp/tp1/parkLamp/streetLight.obj");
        // m_lamp.create(lamp);
        // m_programStatic = read_program("tp/tp1/staticglsl");
        // program_print_errors(m_programStatic);

        // lampModel = Translation(10,0,0) * Scale(5,5,5);
        // Point pmin, pmax;
        // lamp.bounds(pmin, pmax);
        // m_camera.lookat(pmin, pmax);
        
        addStaticObj("tp/tp1/parkLamp/streetLight.obj");
        m_staticModels.push_back(Scale(5,5,5) * Translation(-2,0,0));

        // addStaticObj("tp/tp1/road/road.obj");
        // m_staticModels.push_back(Scale(5,5,5) * Translation(-2,0,0) * RotationZ(90));


        m_programStatic = read_program("tp/tp1/static.glsl");
        program_print_errors(m_programStatic);

        //2 lumières
        //Le soleil => todo move it with time
        // lightPos[0] = Point(10,40,0); 
        // lightColor[0] = White();

        // //lightPos[1] = lampModel(Point(0,3.3,0)); 
        // lightPos[1] = Point(10,10,0);
        // lightColor[1] = White();

        m_lights.push_back(Light(Point(30,10,0), 5, White()));
        m_lights.push_back(Light(Point(10,10,0), 5, White()));


        //Create ssbo
        createSSBO();


        // etat openGL par defaut
        glClearColor(0.2f, 0.2f, 0.2f, 1.f);        // couleur par defaut de la fenetre
        
        glClearDepth(1.f);                          // profondeur par defaut
        glDepthFunc(GL_LESS);                       // ztest, conserver l'intersection la plus proche de la camera
        glEnable(GL_DEPTH_TEST);                    // activer le ztest

        return 0;   // ras, pas d'erreur
    }

    void createDepthFrameBuffer() {

        glGenTextures(1, &m_depth_buffer);
        glBindTexture(GL_TEXTURE_2D, m_depth_buffer);
        
        glTexImage2D(GL_TEXTURE_2D, 0,
            GL_DEPTH_COMPONENT, window_width(), window_height(), 0,
            GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);
        
        // etape 2 : creer et configurer un framebuffer object
        glGenFramebuffers(1, &m_framebuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_framebuffer);
        glFramebufferTexture(GL_DRAW_FRAMEBUFFER,  /* attachment */ GL_DEPTH_ATTACHMENT, /* texture */ m_depth_buffer, /* mipmap level */ 0);
        
        // le fragment shader ne declare qu'une seule sortie, indice 0
        GLenum buffers[]= { GL_DEPTH_ATTACHMENT };
        glDrawBuffers(1, buffers);
        
        // nettoyage
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    }

    void updateDepthBuffer() {

        //Dessiner dans le framebuffer

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_framebuffer);
        glViewport(0, 0, window_width(), window_height());
        glClearColor(1, 1, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        


        glUseProgram(m_shadowMap_program);





        // // draw "classique"
        // glBindVertexArray(m_vao);
        // glUseProgram(m_texture_program);
        
        // Transform m= m_model;
        // Transform v= m_framebuffer_camera.view();
        // Transform p= m_framebuffer_camera.projection(window_width(), window_height(), 45);
        // Transform mvp= p * v * m;
        // Transform mv= v * m;
        
        // program_uniform(m_texture_program, "mvpMatrix", mvp);
        // program_uniform(m_texture_program, "mvMatrix", mv);
        // program_uniform(m_texture_program, "normalMatrix", mv.normal());
        // program_use_texture(m_texture_program, "color_texture", 0, m_color_texture, 0);
        
        // glDrawArrays(GL_TRIANGLES, 0, m_vertex_count);


    }
    
    // destruction des objets de l'application
    int quit( )
    {
        //m_robot.release();
        
        for (int i=0; i<m_animatedObj.size(); ++i)
            m_animatedObj[i].release();
        for (int i=0; i<m_staticObj.size(); ++i)
            m_staticObj[i].release();
        

        //m_lamp.release();
        //m_dog.release();
        glDeleteBuffers(1, &m_ssbo);

        release_program(m_programDynamic);
        release_program(m_programStatic);
        return 0;
    }

    void addAnimatedObj(const std::string& beginFilePath, int nb) {

        std::vector<Mesh> meshes;
        std::string meshFile, numStr;
        for (int i = 1; i <= nb; ++i)
        {
            if (i < 10) numStr = "0";
            else numStr = "";
            numStr += std::to_string(i);
            meshFile = beginFilePath + numStr + ".obj";
            meshes.push_back(read_mesh(meshFile.c_str()));
        }

        //Create all keyframes
        Buffers b;
        b.create(meshes);
        m_animatedObj.push_back(b);
    }

    int addStaticObj(const std::string& filePath) {
        int index = m_staticObj.size();
        Buffers b;
        b.create(read_mesh(filePath.c_str()));
        m_staticObj.push_back(b);
        return index;
    }

    void createSSBO() {
        glGenBuffers(1, &m_ssbo);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssbo);

        //On alloue une taille de 10 materiaux pour être sur d'avoir la place de les stocker
        glBufferData(GL_SHADER_STORAGE_BUFFER,  numberOfLights * sizeof(Light) + max_materials * sizeof(Material_glsl), nullptr, GL_DYNAMIC_READ);
    }

    void putLightsInSSBO() {
        const int lightSize = numberOfLights * sizeof(Light);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssbo);
        //printf("Lights in ssbo offset %d, size %d\n", 0, lightSize);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, lightSize, m_lights.data());
        //number of lights
        //glBufferSubData(GL_SHADER_STORAGE_BUFFER, lightSize, sizeof(float), &numberOfLights);
    }

    void putMaterialsInSSBO(const Buffers& obj) {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssbo);
        assert(obj.materials.size() <= max_materials);
        //printf("Mats in ssbo offset %ld, size %ld\n", numberOfLights * sizeof(Light), obj.materials.size() * sizeof(Material_glsl));
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, numberOfLights * sizeof(Light), obj.materials.size() * sizeof(Material_glsl), obj.materials.data());
    }

    void printTextConsole() {
        //printf(m_console, 0, 3, "nb cube %d", nrow * ncol);
    }

    int update(const float time, const float delta) {

        //Le soleil
        // const int rSun = 30;

        // Transform model = Translation(sin(time/1000) * rSun, 0, cos(time/1000) * rSun);
        // lightPos[0] = model(lightPos[0]); 

        updateModels();

        return 0;
    }
    

    void updateModels() {

        //TODO connaitre indice du chien, etc...
        float t = global_time();
        float x = sin(t / 1000) * 10;
        float z = cos(t / 1000) * 10;
        Transform T = Translation(x,0,z) * RotationY(t/100.f) * Scale(0.5,0.5,0.5);

        m_animatedModels[DOG] = T;

        const int rSun = 1;

        //const float angle = (t - (int) t) * 360.f;
        //const float angle = (int) t % 360;

        //T = RotationX(angle);
        //m_lights[0].pos = vec3(T(Point(10,0,-10))); 

          

        // T = Translation(sin(t/4000) * rSun, -cos(t/5000) * rSun, 0);
        // m_lights[0].pos = vec3(T(Point(m_lights[0].pos)));

        // printf("h:%f %f %f\n", m_lights[0].pos.x, m_lights[0].pos.y, m_lights[0].pos.z);

    }

    // dessiner une nouvelle image
    int render( )
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // deplace la camera
        int mx, my;
        unsigned int mb= SDL_GetRelativeMouseState(&mx, &my);
        if(mb & SDL_BUTTON(1))              // le bouton gauche est enfonce
            m_camera.rotation(mx, my);
        else if(mb & SDL_BUTTON(3))         // le bouton droit est enfonce
            m_camera.move(mx);
        else if(mb & SDL_BUTTON(2))         // le bouton du milieu est enfonce
            m_camera.translation((float) mx / (float) window_width(), (float) my / (float) window_height());

        putLightsInSSBO();

        //glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_ssbo);

        renderAnimatedObj();
        renderStaticObj();

        //Draw the lamp:
        //renderLamp();
        
        return 1;
    }

    // void renderRobot() {
    //     setUniform(m_programStatic, robotModel, m_robot);

    //     const float F0 = 1.f;
    //     glUniform1f(glGetUniformLocation(m_programStatic, "F0"), F0);

    //     const float alpha = 0.5f;
    //     glUniform1f(glGetUniformLocation(m_programStatic, "alpha"), alpha);

    //     const int nbFramePerSec = 24;
    //     const float frameTime = 1000.f / nbFramePerSec; //En ms
    //     float time = global_time() / frameTime; //Ex : 1.5 entre frame 1 et 2
    //     int nbFrame = (int) time % m_robot.keyframe_count;
    //     float interpolationFactor = time - (int) time; //partie fractionnaire

    //     glUniform1f(glGetUniformLocation(m_programStatic, "t"), interpolationFactor);

    //     //Décalage de l'offset en fct du numéro de la frame
    //     m_robot.setPointer(nbFrame);

    //     glBindVertexArray(m_robot.vao);
    //     glDrawArrays(GL_TRIANGLES, 0, m_robot.vertex_count);
    // }

    void renderAnimatedObj() {

        for (int i=0; i<m_animatedObj.size(); ++i) {

            //ssbo materials
            putMaterialsInSSBO(m_animatedObj[i]);

            //TODO different models
            //Same shader
            setUniform(m_programDynamic, m_animatedModels[i]);

            //TODO stocker ça dans info objets! Buffers == obj?
            const float F0 = 1.f;
            glUniform1f(glGetUniformLocation(m_programDynamic, "F0"), F0);

            const float alpha = 0.5f;
            glUniform1f(glGetUniformLocation(m_programDynamic, "alpha"), alpha);

            const int nbFramePerSec = 24;
            const float frameTime = 1000.f / nbFramePerSec; //En ms
            float time = global_time() / frameTime; //Ex : 1.5 entre frame 1 et 2
            int nbFrame = (int) time % m_animatedObj[i].keyframe_count;
            float interpolationFactor = time - (int) time; //partie fractionnaire

            glUniform1f(glGetUniformLocation(m_programDynamic, "t"), interpolationFactor);

            //Décalage de l'offset en fct du numéro de la frame
            m_animatedObj[i].setPointer(nbFrame);

            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_ssbo);

            glBindVertexArray(m_animatedObj[i].vao);
            glDrawArrays(GL_TRIANGLES, 0, m_animatedObj[i].vertex_count);
        }
    }

    void renderStaticObj() {
        
        for (int i=0; i<m_staticObj.size(); ++i) {

            //ssbo materials
            putMaterialsInSSBO(m_staticObj[i]);

            //TODO different models
            //Same shader
            setUniform(m_programStatic, m_staticModels[i]);

            //TODO stocker ça dans info objets! Buffers == obj?
            const float F0 = 1.f;
            glUniform1f(glGetUniformLocation(m_programStatic, "F0"), F0);

            const float alpha = 0.5f;
            glUniform1f(glGetUniformLocation(m_programStatic, "alpha"), alpha);

            m_staticObj[i].setPointer(0);

            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_ssbo);


            glBindVertexArray(m_staticObj[i].vao);
            glDrawArrays(GL_TRIANGLES, 0, m_staticObj[i].vertex_count);
        }
    }
/*
    void renderLamp() {        
        setUniform(m_programStatic, lampModel, m_lamp);

        const float F0 = 1.f;
        glUniform1f(glGetUniformLocation(m_programStatic, "F0"), F0);

        const float alpha = 0.5f;
        glUniform1f(glGetUniformLocation(m_programStatic, "alpha"), alpha);

        glBindVertexArray(m_lamp.vao);
        // dessiner les triangles de l'objet
        glDrawArrays(GL_TRIANGLES, 0, m_lamp.vertex_count);
    }
*/

    //void updateAndBindSSBOForObject(ObjectIndex oIndex, ObjectType oType) {

    //    //On met les bons materials après les lumières

    //    std::vector<Buffers> &obj = (oType == ANIMATED) ? m_animatedObj : m_staticObj;

    //    glBufferSubData(GL_SHADER_STORAGE_BUFFER, numberOfLights * sizeof(Light), max_materials * sizeof(Material_glsl), obj.data());
    //}


    void setUniform(GLuint program, const Transform& model) {
        Transform view = m_camera.view();
        Transform projection = m_camera.projection(window_width(), window_height(), 45);
        Transform mvp = projection * view * model;
        Transform mv = view * model;

        int location;

        glUseProgram(program);
        location = glGetUniformLocation(program, "mvpMatrix");
        glUniformMatrix4fv(location, 1, GL_TRUE, mvp.buffer());
        location = glGetUniformLocation(program, "mMatrix");
        glUniformMatrix4fv(location, 1, GL_TRUE, model.buffer());
        location = glGetUniformLocation(program, "mNormal");
        glUniformMatrix4fv(location, 1, GL_TRUE, model.normal().buffer());
        location = glGetUniformLocation(program, "mvMatrix");
        glUniformMatrix4fv(location, 1, GL_TRUE, mv.buffer());
        location = glGetUniformLocation(program, "mvNormal");
        glUniformMatrix4fv(location, 1, GL_TRUE, mv.normal().buffer());
        //Camera pos
        location = glGetUniformLocation(program, "camera");
        Point p = m_camera.position(); //Repère du monde
        glUniform3fv(location, 1, &p.x);

        //const int NB_LIGHTS = 2;
        //Light pos
        //location = glGetUniformLocation(program, "lightPosition");
        //glUniform3fv(location, NB_LIGHTS, &lightPos[0].x);
        //glUniform3fv(location, 1, &p.x);

        //Light col
        //location = glGetUniformLocation(program, "lightColor");
        //glUniform4fv(location, NB_LIGHTS, &lightColor[0].r);

        //TODO storage buffer


        //Les materials (uniform arrays)
        // location = glGetUniformLocation(program, "diffuse");
        // glUniform4fv(location, buffer.NB_MATERIALS, &buffer.diffuse[0].r);

        // location = glGetUniformLocation(program, "specular");
        // glUniform4fv(location, buffer.NB_MATERIALS, &buffer.specular[0].r);

        // location = glGetUniformLocation(program, "emission");
        // glUniform4fv(location, buffer.NB_MATERIALS, &buffer.emission[0].r);

        // location = glGetUniformLocation(program, "ns");
        // glUniform1fv(location, buffer.NB_MATERIALS, buffer.ns.data());
    }

protected:
    
    //AnimationBuffers m_robot;

    GLuint m_ssbo;
    const int numberOfLights = 2;
    const int max_materials = 10;


    std::vector<Buffers> m_animatedObj;
    std::vector<Transform> m_animatedModels;

    std::vector<Buffers> m_staticObj;
    std::vector<Transform> m_staticModels;

    //Buffers m_robot, m_lamp, m_dog;

    GLuint m_programDynamic;
    GLuint m_programStatic;
    Orbiter m_camera;

    //Lumière

    std::vector<Light> m_lights;

    GLuint m_shadowMap_program;
    GLuint m_framebuffer;
    GLuint m_depth_buffer;

    //Point lightPos[2];
    //Color lightColor[2];

    //Les modeles
    //Transform lampModel, robotModel;
};


int main( int argc, char **argv )
{
    // il ne reste plus qu'a creer un objet application et la lancer 
    TP tp;
    tp.run();
    
    return 0;
}
