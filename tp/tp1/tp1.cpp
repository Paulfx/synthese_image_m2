
//! \file tuto7.cpp reprise de tuto6.cpp mais en derivant App::init(), App::quit() et bien sur App::render().

#include "wavefront.h"
#include "texture.h"

#include "orbiter.h"
#include "draw.h"        
#include "app_time.h"        // classe Application a deriver
#include "program.h"
#include "uniforms.h"
#include "mesh.h"

#include <vector>


struct Buffers
{
    GLuint vao;
    GLuint vertex_buffer;

    size_t vertexBufferSize;

    GLuint materials_buffer; //Contient les indices des matérials
    int vertex_count;
    int keyframe_count;

    
    //On stocke les info des materials
    int NB_MATERIALS;
    std::vector<Color> diffuse;
    std::vector<Color> specular;
    std::vector<Color> emission;
    std::vector<float> ns;

    Buffers( ) : vao(0), vertex_buffer(0), vertex_count(0) {}

    void create(const std::vector<Mesh>& meshes) {
        //Les mesh doivent être identiques...
        //Pré condition
        keyframe_count = meshes.size();
        vertexBufferSize = meshes[0].vertex_buffer_size();

        //Pour ne pas perdre la dernière frame, on ajoute la kf 0 à la fin
        size_t bufferSize = ( keyframe_count + 1) * vertexBufferSize;

        glGenBuffers(1, &vertex_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        glBufferData(GL_ARRAY_BUFFER, bufferSize, nullptr, GL_STATIC_DRAW);

        //On load les keyframes
        size_t offset = 0;
        for (unsigned i=0; i<meshes.size(); ++i) { 
            glBufferSubData(GL_ARRAY_BUFFER, offset, meshes[i].vertex_buffer_size(), meshes[i].vertex_buffer());
            offset += meshes[i].vertex_buffer_size();
        }
        //On rajoute la dernière keyframe
        glBufferSubData(GL_ARRAY_BUFFER, offset, meshes[0].vertex_buffer_size(), meshes[0].vertex_buffer());

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        // attribut 0, position des sommets, declare dans le vertex shader : in vec3 position;
        glVertexAttribPointer(0, 
            3, GL_FLOAT,            // size et type, position est un vec3 dans le vertex shader
            GL_FALSE,               // pas de normalisation des valeurs
            0,                      // stride 0, les valeurs sont les unes a la suite des autres
            0                      // offset
        );
        glEnableVertexAttribArray(0);

        // attribut 1, position des sommets de la keyframe 2
        glVertexAttribPointer(1, 
            3, GL_FLOAT,            // size et type, position est un vec3 dans le vertex shader
            GL_FALSE,               // pas de normalisation des valeurs
            0,                      // stride 0, les valeurs sont les unes a la suite des autres
            (const void *) vertexBufferSize   // Après la première keyframe
        );
        glEnableVertexAttribArray(1);

        vertex_count = meshes[0].vertex_count();

        createMaterials(meshes);

        //On initialise les vertex attrib (au cas ou..)
        setPointer(0);
    }

    void createMaterials(const std::vector<Mesh>& meshes) {
        //On stocke info materials
        const std::vector<Material> materials = meshes[0].mesh_materials();
        printf("Nombre de materiaux : %ld\n", materials.size());

        NB_MATERIALS = materials.size();

        for (int i = 0; i < NB_MATERIALS; ++i) {
            diffuse.push_back(materials[i].diffuse);
            specular.push_back(materials[i].specular);
            emission.push_back(materials[i].emission);
            ns.push_back(materials[i].ns);
        }


        //Materials, on créé un buffer
        const std::vector<unsigned int> matIndex = meshes[0].materials();

        //On multiplie les indices pour chaque vertex
        std::vector<unsigned char> matIndexForVertex;
        for (std::vector<unsigned int>::const_iterator i = matIndex.begin(); i != matIndex.end(); ++i) {
            matIndexForVertex.insert(matIndexForVertex.end(), 3, *i); //3 fois le même indice
        }


        //Unsigned char => 256 materials max
        size_t bufferSize = matIndexForVertex.size() * sizeof(unsigned char);
        glGenBuffers(1, &materials_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, materials_buffer);

        glBufferData(GL_ARRAY_BUFFER, bufferSize, matIndexForVertex.data(), GL_STATIC_DRAW);
    }
    
    void setPointer(size_t nbFrame) {

        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

        //Frame 0
        glVertexAttribPointer(0, 
            3, GL_FLOAT,            // size et type, position est un vec3 dans le vertex shader
            GL_FALSE,               // pas de normalisation des valeurs
            0,                      // stride 0, les valeurs sont les unes a la suite des autres
            (const void *) (nbFrame * vertexBufferSize)              // offset
        );
        glEnableVertexAttribArray(0);

        //Frame 1
        glVertexAttribPointer(1, 
            3, GL_FLOAT,            // size et type, position est un vec3 dans le vertex shader
            GL_FALSE,               // pas de normalisation des valeurs
            0,                      // stride 0, les valeurs sont les unes a la suite des autres
            (const void *) ( (nbFrame+1) * vertexBufferSize)   // Après la première keyframe
        );
        glEnableVertexAttribArray(1);

        //Pas de changement pour les matérials
        glBindBuffer(GL_ARRAY_BUFFER, materials_buffer);
        glVertexAttribIPointer(2, 1, GL_UNSIGNED_BYTE, 0, 0);
        glEnableVertexAttribArray(2);

    }

    
    void release( )
    {
        glDeleteBuffers(1, &vertex_buffer);
        glDeleteBuffers(1, &materials_buffer);
        glDeleteVertexArrays(1, &vao);
    }
};

class TP : public App
{
public:
    // constructeur : donner les dimensions de l'image, et eventuellement la version d'openGL.
    TP( ) : App(1024, 640) {}
    
    // creation des objets de l'application
    int init( )
    {
        std::vector<Mesh> meshes;
        const std::string beginFile = "tp/tp1/Robot/run/Robot_0000";
        std::string meshFile, numStr;
        for (int i = 1; i <= 23; ++i)
        {
            if (i < 10) numStr = "0";
            else numStr = "";
            numStr += std::to_string(i);
            meshFile = beginFile + numStr + ".obj";
            meshes.push_back(read_mesh(meshFile.c_str()));
        }

        //Create all keyframes
        m_objet.create(meshes);

        Point pmin, pmax;
        // meshes[0].bounds(pmin, pmax);

        pmin = Point(-10,-10,0);
        pmax = Point(10,10,0);

        m_camera.lookat(pmin, pmax);

        m_program= read_program("tp/tp1/robot.glsl");
        program_print_errors(m_program);

        // etat openGL par defaut
        glClearColor(0.2f, 0.2f, 0.2f, 1.f);        // couleur par defaut de la fenetre
        
        glClearDepth(1.f);                          // profondeur par defaut
        glDepthFunc(GL_LESS);                       // ztest, conserver l'intersection la plus proche de la camera
        glEnable(GL_DEPTH_TEST);                    // activer le ztest


        return 0;   // ras, pas d'erreur
    }
    
    // destruction des objets de l'application
    int quit( )
    {
        m_objet.release();
        glDeleteTextures(1, &m_texture);

        release_program(m_program);
        
        return 0;
    }

    void printTextConsole() {
        //printf(m_console, 0, 3, "nb cube %d", nrow * ncol);
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


        Transform view = m_camera.view();
        Transform projection = m_camera.projection(window_width(), window_height(), 45);
        Transform model;
        Transform mvp;

        int location;
        glUseProgram(m_program);

        location = glGetUniformLocation(m_program, "mvpMatrix");

        glBindVertexArray(m_objet.vao);

        model = Identity();
        mvp = projection * view * model;

        //on envoie la mvp au shader
        glUniformMatrix4fv(location, 1, GL_TRUE, mvp.buffer());

        const int nbFramePerSec = 24;
        const float frameTime = 1000.f / nbFramePerSec; //En ms
        float time = global_time() / frameTime; //Ex : 1.5 entre frame 1 et 2
        int nbFrame = (int) time % m_objet.keyframe_count;
        float interpolationFactor = time - (int) time; //partie fractionnaire

        location = glGetUniformLocation(m_program, "time");
        glUniform1f(location, interpolationFactor);


        //Les materials
        location = glGetUniformLocation(m_program, "diffuse");
        glUniform4fv(location, m_objet.NB_MATERIALS, &m_objet.diffuse[0].r);

        location = glGetUniformLocation(m_program, "specular");
        glUniform4fv(location, m_objet.NB_MATERIALS, &m_objet.specular[0].r);

        location = glGetUniformLocation(m_program, "emission");
        glUniform4fv(location, m_objet.NB_MATERIALS, &m_objet.emission[0].r);

        location = glGetUniformLocation(m_program, "ns");
        glUniform1fv(location, m_objet.NB_MATERIALS, m_objet.ns.data());


        //On commence à un offset correspondant à la frame en question
        //glDrawArrays(GL_TRIANGLES, m_objet.vertex_count * nbFrame, m_objet.vertex_count);
            
        //Version avec décalage de pointeur (MIEUX)
        m_objet.setPointer(nbFrame);
        glDrawArrays(GL_TRIANGLES, 0, m_objet.vertex_count);
        
        return 1;
    }

protected:
    
    Buffers m_objet;

    GLuint m_texture;
    GLuint m_program;
    Orbiter m_camera;

};


int main( int argc, char **argv )
{
    // il ne reste plus qu'a creer un objet application et la lancer 
    TP tp;
    tp.run();
    
    return 0;
}
