#ifndef BUFFERS_CLASS_H
#define BUFFERS_CLASS_H

#include "glcore.h"
#include "mesh.h"
#include <vector>

#include "infos.h"

//Buffers pour objet statique (pas d'animation) ou animé
class Buffers {

public:

    bool init; //On ne peut faire qu'un seul create

    GLuint vao;
    GLuint vertex_buffer;
    GLuint materials_buffer;
    int vertex_count;
    int keyframe_count;

    size_t vertexBufferSize, normalBufferSize;

    //On stocke les info des materials

    //TODO struct uniform array
    // int NB_MATERIALS;
    // std::vector<Color> diffuse;
    // std::vector<Color> specular;
    // std::vector<Color> emission;
    // std::vector<float> ns;

    std::vector<Material_glsl> materials;

    Buffers();
    
    //Only one mesh = one keyframe
    void create(const Mesh& mesh);

    //Plusieurs keyframes (animation)
    void create(const std::vector<Mesh>& meshes);

    void createMaterials(const Mesh& mesh);

    void setPointer(size_t nbFrame);
    
    void release();
};


#endif // BUFFERS_CLASS_H
