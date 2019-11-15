#include "SkyBox.h"

SkyBox::SkyBox() {
	//Create vertex buffer

    // skybox VAO
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindVertexArray(0);
}

SkyBox::~SkyBox() {
    glDeleteVertexArrays(1, &m_vao);
    glDeleteBuffers(1, &m_vao);
}

void SkyBox::loadProgram(string file) {
	m_program= read_program(file.c_str());
    program_print_errors(m_program);
}

int SkyBox::loadCubeMap(string allFaces) {
    return 1;
}

int SkyBox::loadCubeMap(vector<string> faces, const GLenum texel_type) {
	glGenTextures(1, &m_texture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_texture);

    ImageData im;
	for (unsigned int i = 0; i < faces.size(); ++i) {	
        im = read_image_data(faces[i].c_str());
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, im.width, im.height, 0, GL_RGB, GL_UNSIGNED_BYTE, im.buffer());
    }

    // fixe les parametres de filtrage par defaut
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return 1;
}

void SkyBox::render(const Transform& view, const Transform& projection) {
    glDepthMask(GL_FALSE);
    glUseProgram(m_program);
    glActiveTexture(GL_TEXTURE0);

    Transform viewNoTranslation = Transform(view[0], view[1], view[2], Vector(0,0,0));

    Transform viewRotated = viewNoTranslation * RotationZ(180);

    // . parametrer le shader program :
    glUniformMatrix4fv( glGetUniformLocation(m_program, "view"), 1, GL_FALSE, viewRotated.buffer() );
    glUniformMatrix4fv( glGetUniformLocation(m_program, "projection"), 1, GL_FALSE, projection.buffer() );


    // Draw skybox cube
    glBindVertexArray(m_vao);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_texture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    
    glDepthMask(GL_TRUE);
    glUseProgram(0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}