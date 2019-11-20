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
    unsigned vertex_count;
    unsigned keyframe_count;

    size_t vertexBufferSize, normalBufferSize;

    //On stocke les info des materials
    std::vector<Material_glsl> materials;

    Buffers() : init(false), vao(0), vertex_buffer(0), vertex_count(0) {};
    
    //Only one mesh = one keyframe
    void create(const Mesh& mesh);

    //Plusieurs keyframes (animation)
    void create(const std::vector<Mesh>& meshes);

    //Créer le buffer materials_buffer pour stocker les indices des matériaux
    //Et le vecteur materials contenant les informations des matériaux
    //Alignés pour être stockées dans un shader storage buffer.
    void createMaterials(const Mesh& mesh);

    //Décale les vertex attrib pointer pour faire pointer sur la frame nbFrame
    //Et nbFrame+1 (si l'objet est animé)
    void setPointer(size_t nbFrame);
    
    //Destruction des buffers
    void release();
};


#endif // BUFFERS_CLASS_H
