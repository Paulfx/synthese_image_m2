#define GL_SILENCE_DEPRECATION
#ifndef SKY_BOX_H
#define SKY_BOX_H

#include <vector>
#include <string>
#include <iostream>

#include "glcore.h"
#include <image_io.h> //Read image
#include <program.h>
#include <mat.h>
#include <uniforms.h>
#include "orbiter.h"

#include <mesh.h>

using std::vector;
using std::string;

const float skyboxVertices[] = {
    // positions          
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};

class SkyBox
{
public:
	SkyBox();
	~SkyBox();

	void loadProgram(string file);

	// Order : +x, -x, +y, -y, +z, -z
	int loadCubeMap(vector<string> faces, const GLenum texel_type= GL_RGBA);

	void render(const Transform& view, const Transform& projection);

    void renderGkit(const Transform& view, const Transform& projection);

    int loadCubeMap(string allFaces);
	
private:
	GLuint m_texture;

	GLuint m_program;

	unsigned int m_vao, m_vbo;

    Mesh m_mesh;

    void loadTextureCubeMap(const string &file, unsigned int i, const GLenum texel_type);

	
};

#endif // SKY_BOX_H
