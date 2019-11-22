
//! \file tuto_vertex_compute.cpp  exemple direct.

#include "app.h"

#include "vec.h"
#include "mat.h"

#include "program.h"
#include "uniforms.h"
#include "texture.h"

#include "mesh.h"
#include "wavefront.h"

#include "orbiter.h"


// cf tuto_storage
namespace glsl 
{
    template < typename T >
    struct alignas(8) gvec2
    {
        alignas(4) T x, y;
        
        gvec2( ) {}
        gvec2( const ::vec2& v ) : x(v.x), y(v.y) {}
    };
    
    typedef gvec2<float> vec2;
    typedef gvec2<int> ivec2;
    typedef gvec2<unsigned int> uvec2;
    typedef gvec2<int> bvec2;
    
    template < typename T >
    struct alignas(16) gvec3
    {
        alignas(4) T x, y, z;
        
        gvec3( ) {}
        gvec3( const ::vec3& v ) : x(v.x), y(v.y), z(v.z) {}
        gvec3( const Point& v ) : x(v.x), y(v.y), z(v.z) {}
        gvec3( const Vector& v ) : x(v.x), y(v.y), z(v.z) {}
    };
    
    typedef gvec3<float> vec3;
    typedef gvec3<int> ivec3;
    typedef gvec3<unsigned int> uvec3;
    typedef gvec3<int> bvec3;
    
    template < typename T >
    struct alignas(16) gvec4
    {
        alignas(4) T x, y, z, w;
        
        gvec4( ) {}
        gvec4( const ::vec4& v ) : x(v.x), y(v.y), z(v.z), w(v.w) {}
    };
    
    typedef gvec4<float> vec4;
    typedef gvec4<int> ivec4;
    typedef gvec4<unsigned int> uvec4;
    typedef gvec4<int> bvec4;
}


class VertexCompute : public App
{
public:
    // application openGL 4.3
    VertexCompute( ) : App(1024, 640,  4,3) {}
    
    int init( )
    {
        m_mesh= read_mesh("data/bigguy.obj");
        printf("  positions %d\n", m_mesh.vertex_count());
        printf("  triangles %d\n", m_mesh.triangle_count());
        
        // construit le storage buffer contenant les positions, les normales et les texcoords, en utilisant les types alignes
        struct triangle_glsl
        {
            glsl::vec3 a,b,c;
            glsl::vec3 na,nb,nc;
            glsl::vec2 ta,tb,tc;

            triangle_glsl(TriangleData td) : a(td.a), b(td.b), c(td.c), na(td.na), nb(td.nb), nc(td.nc), ta(td.ta), tb(td.tb), tc(td.tc) {}; 
        };
        
        // recupere les attributs du mesh
        std::vector<triangle_glsl> data;//(m_mesh.triangle_count());
        for(int i= 0; i < m_mesh.triangle_count(); i++)
            data.push_back( triangle_glsl(m_mesh.triangle(i)));
        
        // vao par defaut, pas d'attribut. les positions des sommets sont dans le storage buffer...
        glGenVertexArrays(1, &m_vao);
        glBindVertexArray(m_vao);
        
        // storage buffers
        glGenBuffers(1, &m_buffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_buffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(triangle_glsl) * data.size(), data.data(), GL_STREAM_READ);
        
        //
        m_compute_program= read_program("tp/tp3/compute.glsl");
        program_print_errors(m_compute_program); 
        m_program= read_program("tp/tp3/pipeline.glsl");
        program_print_errors(m_program);
        
        //
        Point pmin, pmax;
        m_mesh.bounds(pmin, pmax);
        m_camera.lookat(pmin, pmax);
        
        // etat openGL par defaut
        glClearColor(0.2f, 0.2f, 0.2f, 1.f);        // couleur par defaut de la fenetre
        
        glClearDepth(1.f);                          // profondeur par defaut
        glDepthFunc(GL_LESS);                       // ztest, conserver l'intersection la plus proche de la camera
        glEnable(GL_DEPTH_TEST);                    // activer le ztest
        
        // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);      // ne dessine que les aretes des triangles
        // glLineWidth(2);
        
        return 0;   // ras, pas d'erreur
    }

    void createTextureOutput() {
        glGenTextures(1, &m_texture_output);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_texture_output);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, window_width(), window_height(), 0, GL_RGBA, GL_FLOAT,
         NULL);
        glBindImageTexture(0, m_texture_output, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    }
    
    void printInfoCompute() {
        int work_grp_cnt[3];

        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &work_grp_cnt[0]);
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &work_grp_cnt[1]);
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &work_grp_cnt[2]);

        printf("max global (total) work group size x:%i y:%i z:%i\n",
          work_grp_cnt[0], work_grp_cnt[1], work_grp_cnt[2]);

        int work_grp_size[3];

        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &work_grp_size[0]);
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &work_grp_size[1]);
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &work_grp_size[2]);

        printf("max local (in one shader) work group sizes x:%i y:%i z:%i\n",
          work_grp_size[0], work_grp_size[1], work_grp_size[2]);

        int work_grp_inv;

        glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &work_grp_inv);
        printf("max local work group invocations %i\n", work_grp_inv);
    }

    // destruction des objets de l'application
    int quit( )
    {
        m_mesh.release();
        release_program(m_program);
        release_program(m_compute_program);
        glDeleteBuffers(1, &m_buffer);
        glDeleteTextures(1, &m_texture_output);
        glDeleteVertexArrays(1, &m_vao);
        return 0;
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

        // selectionne le buffer
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_buffer);        
        // etape 1 : utilise le compute shader pour tray tracer les primitives de la scène
        glUseProgram(m_compute_program);
        
        // uniforms du compute shader
        Transform model= RotationY(global_time() / 60);
        Transform view= m_camera.view();
        Transform projection= m_camera.projection(window_width(), window_height(), 45);
        Transform mvp= projection * view * model;
        
        // program_uniform(m_compute_program, "mvpMatrix", mvp);
        // program_uniform(m_compute_program, "mvpInvMatrix", mvp.inverse());
        
        // Nombre de groupes : 

        //Version 1: 1 groupe par pixel de l'image
        glDispatchCompute(window_width(), window_height(), 1);
        
        // etape 2 : synchronisation
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);     
        
        // Dessiner la texture_output
        glBindVertexArray(m_vao);
        glUseProgram(m_program);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_texture_output);

        //3 vertices pour toute la scène
        glDrawArrays(GL_TRIANGLES, 0, 3);
        return 1;
    }

protected:
    Mesh m_mesh;
    Orbiter m_camera;

    GLuint m_vao;
    GLuint m_buffer;
    GLuint m_texture_output;
    GLuint m_program;
    GLuint m_compute_program;
};


int main( int argc, char **argv )
{
    VertexCompute app;
    app.run();
    
    return 0;
}
