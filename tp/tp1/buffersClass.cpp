#include "buffersClass.h"

//Only one mesh = one keyframe
void Buffers::create( const Mesh& mesh ) {
    if(!mesh.vertex_buffer_size() || init) return;
    printf("Begin of static buffer creation\n");
    init = true;

    vertexBufferSize = mesh.vertex_buffer_size();
    normalBufferSize = mesh.normal_buffer_size();

    // conserve le nombre de sommets
    vertex_count= mesh.vertex_count();
    //Nombre de keyframe = 1
    keyframe_count = 1;

    // cree et initialise le buffer: conserve la positions des sommets
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, vertexBufferSize + normalBufferSize, nullptr, GL_STATIC_DRAW);

    glBufferSubData(GL_ARRAY_BUFFER, 0, vertexBufferSize, mesh.vertex_buffer());

    //On ajoute les normales
    glBufferSubData(GL_ARRAY_BUFFER, vertexBufferSize, normalBufferSize, mesh.normal_buffer());

    //Creation des materials
    createMaterials(mesh);

    glGenVertexArrays(1, &vao);
    setPointer(0);

    printf("End of static buffer creation\n");
}

//Plusieurs keyframes (animation)
void Buffers::create(const std::vector<Mesh>& meshes) {
    //Les mesh doivent être identiques...
    //Pré condition
    if (init || meshes.size() == 0) return;
    vertexBufferSize = meshes[0].vertex_buffer_size();
    normalBufferSize = meshes[0].normal_buffer_size();
    //Check if same size
    for (unsigned i=1; i<meshes.size(); ++i) {
        if (vertexBufferSize != meshes[i].vertex_buffer_size() ||
            normalBufferSize != meshes[i].normal_buffer_size())
            return;
    }
    printf("Begin of animated buffer creation\n");
    init = true;
    keyframe_count = meshes.size();
    vertex_count = meshes[0].vertex_count();
    
    //Pour ne pas perdre la dernière frame, on ajoute la kf 0 à la fin
    size_t bufferSize = ( keyframe_count + 1) * ( vertexBufferSize + normalBufferSize);

    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, bufferSize, nullptr, GL_STATIC_DRAW);

    //On load les keyframes
    size_t offset = 0;
    for (unsigned i=0; i<meshes.size(); ++i) { 
        glBufferSubData(GL_ARRAY_BUFFER, offset, meshes[i].vertex_buffer_size(), meshes[i].vertex_buffer());
        offset += vertexBufferSize;
        glBufferSubData(GL_ARRAY_BUFFER, offset, meshes[i].normal_buffer_size(), meshes[i].normal_buffer());
        offset += normalBufferSize;
    }
    //On rajoute la dernière keyframe
    glBufferSubData(GL_ARRAY_BUFFER, offset, meshes[0].vertex_buffer_size(), meshes[0].vertex_buffer());
    glBufferSubData(GL_ARRAY_BUFFER, offset + normalBufferSize, meshes[0].normal_buffer_size(), meshes[0].normal_buffer());

    //Materials
    createMaterials(meshes[0]);
    
    glGenVertexArrays(1, &vao);
    //On initialise les vertex attrib (au cas ou..)
    setPointer(0);

    printf("End of animated buffer creation\n");
}

void Buffers::createMaterials(const Mesh& mesh) {

    //On stocke info materials
    const std::vector<Material> mats = mesh.mesh_materials();
    printf("Nombre de materiaux : %ld\n", mats.size());

    //Création du vecteur de mat_glsl
    for (unsigned i = 0; i < mats.size(); ++i) {
        materials.push_back(Material_glsl(mats[i]));
    }

    //Materials, on créé un buffer
    const std::vector<unsigned int> matIndex = mesh.materials();

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

void Buffers::setPointer(size_t nbFrame) {
    if (!init || nbFrame >= keyframe_count) return;

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

    const size_t bufferSize = vertexBufferSize + normalBufferSize;

    unsigned int vertexAttribNb = 0;

//Frame 0
    //Position
    glVertexAttribPointer(vertexAttribNb, 
        3, GL_FLOAT,            // size et type, position est un vec3 dans le vertex shader
        GL_FALSE,               // pas de normalisation des valeurs
        0,                      // stride 0, les valeurs sont les unes a la suite des autres
        (const void *) (nbFrame * bufferSize)              // offset
    );
    glEnableVertexAttribArray(vertexAttribNb++);

    //Normales
    glVertexAttribPointer(vertexAttribNb, 
        3, GL_FLOAT,            // size et type, position est un vec3 dans le vertex shader
        GL_FALSE,               // pas de normalisation des valeurs
        0,                      // stride 0, les valeurs sont les unes a la suite des autres
        (const void *) (nbFrame * bufferSize + vertexBufferSize)              // offset
    );
    glEnableVertexAttribArray(vertexAttribNb++);

    //Si animé, deux layouts en plus..
    if (keyframe_count > 1) {
        //Frame 1
        //Position
        glVertexAttribPointer(vertexAttribNb, 
            3, GL_FLOAT,            // size et type, position est un vec3 dans le vertex shader
            GL_FALSE,               // pas de normalisation des valeurs
            0,                      // stride 0, les valeurs sont les unes a la suite des autres
            (const void *) ( (nbFrame+1) * bufferSize)   // Après la première keyframe
        );
        glEnableVertexAttribArray(vertexAttribNb++);

        //Normales
        glVertexAttribPointer(vertexAttribNb, 
            3, GL_FLOAT,            // size et type, position est un vec3 dans le vertex shader
            GL_FALSE,               // pas de normalisation des valeurs
            0,                      // stride 0, les valeurs sont les unes a la suite des autres
            (const void *) ((nbFrame+1) * bufferSize + vertexBufferSize)              // offset
        );
        glEnableVertexAttribArray(vertexAttribNb++);
    }

    //Pas de changement pour les matérials
    glBindBuffer(GL_ARRAY_BUFFER, materials_buffer);
    glVertexAttribIPointer(vertexAttribNb, 1, GL_UNSIGNED_BYTE, 0, 0);
    glEnableVertexAttribArray(vertexAttribNb);
}

void Buffers::release( )
{
    glDeleteBuffers(1, &vertex_buffer);
    glDeleteBuffers(1, &materials_buffer);
    glDeleteVertexArrays(1, &vao);
}