#include "renderer.h"

// creation des objets de l'application
int Renderer::init() {
    SDL_SetWindowFullscreen(m_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
    m_framebuffer_width = window_width();
    m_framebuffer_height = window_height();

    createPrograms();
    createObjects();
    createSSBO();
    createDepthFrameBuffer();

    // etat openGL par defaut
    glClearColor(0.2f, 0.2f, 0.2f, 1.f);        // couleur par defaut de la fenetre
    glClearDepth(1.f);                          // profondeur par defaut
    glDepthFunc(GL_LESS);                       // ztest, conserver l'intersection la plus proche de la camera
    glEnable(GL_DEPTH_TEST);                    // activer le ztest

    //Paramètres de la projection orthographique shadow map
    orthoProjShadowMapSun = Ortho(-150,150,-150,150,0.1f,200.f);
    orthoProjShadowMapLamp = Ortho(-20,20,-20,20,0.1f,30.f);

    //Load orbiter
    m_camera.read_orbiter("tp/tp1/objects/orbiter.txt");

    return 0;   // ras, pas d'erreur
}

void Renderer::createPrograms() {
	GLuint staticVertex_shadowMap = compileShader("tp/tp1/shaders/shadowMap/staticVertex.glsl", GL_VERTEX_SHADER);
	GLuint dynamicVertex_shadowMap = compileShader("tp/tp1/shaders/shadowMap/dynamicVertex.glsl", GL_VERTEX_SHADER);
	GLuint fragment_shadowMap = compileShader("tp/tp1/shaders/shadowMap/fragment.glsl", GL_FRAGMENT_SHADER);

	createProgramAndLinkShaders(m_staticShadowMap_program, staticVertex_shadowMap, fragment_shadowMap);
	createProgramAndLinkShaders(m_dynamicShadowMap_program, dynamicVertex_shadowMap, fragment_shadowMap);

    //Different vertex shader but same fragment shader
    GLuint fragmentShader = compileShader("tp/tp1/shaders/objects/fragment.glsl", GL_FRAGMENT_SHADER);
    GLuint staticVertex = compileShader("tp/tp1/shaders/objects/staticVertex.glsl", GL_VERTEX_SHADER);
    GLuint dynamicVertex = compileShader("tp/tp1/shaders/objects/dynamicVertex.glsl", GL_VERTEX_SHADER);

    createProgramAndLinkShaders(m_programStatic, staticVertex, fragmentShader);
    createProgramAndLinkShaders(m_programDynamic, dynamicVertex, fragmentShader);
}

GLuint Renderer::compileShader(const char* filename, const GLenum shader_type) {
	std::stringbuf source;
    std::ifstream in(filename);
    if(in.good() == false)
        printf("[error] loading program '%s'...\n", filename);
    else
        printf("loading program '%s'...\n", filename);

    in.get(source, 0);        // lire tout le fichier, le caractere '\0' ne peut pas se trouver dans le source de shader
    std::string sourceStr = source.str();

    GLuint shader = glCreateShader(shader_type);

    const char *sources = sourceStr.c_str();
    glShaderSource(shader, 1, &sources, NULL);
    glCompileShader(shader);

    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

    if (status == GL_FALSE) {
    	printf("[error] compiling shader %s\n", filename);

    GLint maxLength = 0;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

	// The maxLength includes the NULL character
	std::vector<GLchar> errorLog(maxLength);
	glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);

		for (int i=0; i < maxLength; ++i) {
			printf("%c", errorLog[i]);
		}
		glDeleteShader(shader); // Don't leak the shader.
    }

    return (status == GL_TRUE) ? shader : 0;
}

void Renderer::createProgramAndLinkShaders(GLuint &program, GLuint v_shader, GLuint f_shader) {
	program = glCreateProgram();

    glAttachShader(program, v_shader);
    glAttachShader(program, f_shader);
    glLinkProgram(program);

    // verifie les erreurs
    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if(status == GL_FALSE)
    {
        printf("[error] linking program %u...\n", program);
    }

    program_print_errors(program);
}

void Renderer::createObjects() {
//Objects
    addAnimatedObj("tp/tp1/objects/Robot/run/Robot_0000", 23);
    m_animatedModels.push_back(Translation(10,1,30));

    addAnimatedObj("tp/tp1/objects/Dog/Dog_0000",39);
    m_animatedModels.push_back(Translation(6,1,-2) * Scale(0.5,0.5,0.5));

    addStaticObj("tp/tp1/objects/city.obj");
    m_staticModels.push_back(Translation(0,-1.5,0) * Scale(10,10,10));

    addStaticObj("tp/tp1/objects/Nature_pack/OBJ/Wheat.obj");
    m_staticModels.push_back(Translation(-22,25,43));

//Lights    
    //Soleil
    m_lights.push_back(Light(Point(0,0,0), 12, White()));
    
    //Position des street lamp
    m_lights.push_back(Light(Point(15,25,32), 10, Color(1,0.5,0.5)));
    m_lights.push_back(Light(Point(-21,25,42.5), 10, Color(1,0.5,0.5)));
    m_lights.push_back(Light(Point(50,25,60), 10, Color(1,0.5,0.5)));
}

void Renderer::addAnimatedObj(const std::string& beginFilePath, int nb) {

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

int Renderer::addStaticObj(const std::string& filePath) {
    int index = m_staticObj.size();
    Buffers b;
    b.create(read_mesh(filePath.c_str()));
    m_staticObj.push_back(b);
    return index;
}

void Renderer::createDepthFrameBuffer() {

    // etape 1 : creer une texture couleur...
    glGenTextures(1, &m_color_buffer);
    glBindTexture(GL_TEXTURE_2D, m_color_buffer);
    
    glTexImage2D(GL_TEXTURE_2D, 0,
        GL_RGBA, m_framebuffer_width, m_framebuffer_height, 0,
        GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    // ... et tous ses mipmaps
    glGenerateMipmap(GL_TEXTURE_2D);
    
    // et son sampler
    glGenSamplers(1, &m_color_sampler);
    
    glSamplerParameteri(m_color_sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glSamplerParameteri(m_color_sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(m_color_sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glSamplerParameteri(m_color_sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    createDepthBuffer(m_depth_buffer1);
    createDepthBuffer(m_depth_buffer2);
    createDepthBuffer(m_depth_buffer3);
    createDepthBuffer(m_depth_buffer4);

    // etape 2 : creer et configurer un framebuffer object
    glGenFramebuffers(1, &m_framebuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_framebuffer);
    glFramebufferTexture(GL_DRAW_FRAMEBUFFER,  /* attachment */ GL_COLOR_ATTACHMENT0, /* texture */ m_color_buffer, /* mipmap level */ 0);
    glFramebufferTexture(GL_DRAW_FRAMEBUFFER,  /* attachment */ GL_DEPTH_ATTACHMENT, /* texture */ m_depth_buffer1, /* mipmap level */ 0);
    
    // // le fragment shader ne declare qu'une seule sortie, indice 0
    GLenum buffers[]= { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, buffers);

    // nettoyage
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void Renderer::createDepthBuffer(GLuint &buf) {
    glGenTextures(1, &buf);
    glBindTexture(GL_TEXTURE_2D, buf);
    
    glTexImage2D(GL_TEXTURE_2D, 0,
        GL_DEPTH_COMPONENT, m_framebuffer_width, m_framebuffer_height, 0,
        GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

}

void Renderer::createSSBO() {
    glGenBuffers(1, &m_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssbo);

    //On alloue une taille de 100 materiaux pour être sur d'avoir la place de les stocker
    glBufferData(   GL_SHADER_STORAGE_BUFFER,  
                    numberOfLights * sizeof(Light) + max_materials * sizeof(Material_glsl), 
                    nullptr, 
                    GL_DYNAMIC_READ);
}

void Renderer::putLightsInSSBO() {
    const int lightSize = numberOfLights * sizeof(Light);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssbo);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, lightSize, m_lights.data());
}

void Renderer::putMaterialsInSSBO(const Buffers& obj) {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssbo);
    assert(obj.materials.size() <= max_materials);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 
                    numberOfLights * sizeof(Light), 
                    obj.materials.size() * sizeof(Material_glsl), 
                    obj.materials.data());
}

int Renderer::update(const float time, const float delta) {
    // deplace la camera
    int mx, my;
    unsigned int mb= SDL_GetRelativeMouseState(&mx, &my);
    if(mb & SDL_BUTTON(1))              // le bouton gauche est enfonce
        m_camera.rotation(mx, my);
    else if(mb & SDL_BUTTON(3))         // le bouton droit est enfonce
        m_camera.move(mx);
    else if(mb & SDL_BUTTON(2))         // le bouton du milieu est enfonce
        m_camera.translation((float) mx / (float) window_width(), (float) my / (float) window_height());

    updateModels();

    return 0;
}

void Renderer::updateModels() {

    //TODO connaitre indice du chien, etc...
    float t = global_time();
    float x = sin(t / 1000) * 10;
    float z = cos(t / 1000) * 10;
    

    //static int signDog = 1;
    //static Transform Rdog = RotationY(0);

    //todo move the dog

    // if (m_animatedModels[DOG][3].z > 6 || m_animatedModels[DOG][3].z < -2) {
    //     signDog = -signDog;
    //     Rdog = RotationY(90) * Rdog;
    // }
    // Transform Tdog = Translation(0,0,signDog * t/60.f) * Rdog;
    // m_animatedModels[DOG] = Tdog;

    //Déplacer le robot, ZQSD
    const float coefTurn = 2.f;
    m_animatedModels[ROBOT] =   m_animatedModels[ROBOT] 
                                * Translation(0, 0, key_state('z') ? 0.5 : 0) 
                                * RotationY(key_state('d') ? -coefTurn : key_state('q') ? coefTurn: 0);

    //Le soleil : distance à midi de 100 pixels en Y, puis rotate autour du monde
    Transform Tsun = RotationZ(t/50.f) * Translation(0,100,0);
    Point posSun = Tsun(Point(0,0,0));
    m_lights[0].pos = vec3(posSun);

    float factorY = posSun.y/100.f;
    m_lights[0].color = vec4(1,factorY, factorY,1);
    const float intensityMaxSun = 12;
    m_lights[0].intensity = std::max(0.f, factorY * intensityMaxSun);

    const float intensityMaxLamp = 7;
    //Les street lamp s'allument quand le soleil décroit
    float intensityLamp = intensityMaxLamp * (1 - m_lights[0].intensity / intensityMaxSun);
    for (int i=1; i<numberOfLights; ++i) {
        m_lights[i].intensity = intensityLamp;        
    }
    printf("%f\n",m_lights[1].intensity/7.f);
}

// dessiner une nouvelle image
int Renderer::render() {
    //Passe 1
    
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_framebuffer);

    //Le soleil
    glFramebufferTexture(GL_DRAW_FRAMEBUFFER,   /* attachment */ GL_DEPTH_ATTACHMENT, 
                                                /* texture */ m_depth_buffer1, 
                                                /* mipmap level */ 0);
    shadowMapRender(orthoProjShadowMapSun, getViewFromSun(m_lights[0].pos));

    //Les street lamp TODO factoriser
    glFramebufferTexture(GL_DRAW_FRAMEBUFFER,   /* attachment */ GL_DEPTH_ATTACHMENT, 
                                                /* texture */ m_depth_buffer2, 
                                                /* mipmap level */ 0);
    shadowMapRender(orthoProjShadowMapLamp, getViewFromLight(m_lights[1].pos));
    glFramebufferTexture(GL_DRAW_FRAMEBUFFER,   /* attachment */ GL_DEPTH_ATTACHMENT, 
                                                /* texture */ m_depth_buffer3, 
                                                /* mipmap level */ 0);
    shadowMapRender(orthoProjShadowMapLamp, getViewFromLight(m_lights[2].pos));
    glFramebufferTexture(GL_DRAW_FRAMEBUFFER,   /* attachment */ GL_DEPTH_ATTACHMENT, 
                                                /* texture */ m_depth_buffer4, 
                                                /* mipmap level */ 0);
    shadowMapRender(orthoProjShadowMapLamp, getViewFromLight(m_lights[3].pos));
    
    if (key_state(' ')) {
        //Afficher le depth buffer
        glBindFramebuffer(GL_READ_FRAMEBUFFER, m_framebuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glViewport(0, 0, window_width(), window_height());
        glClearColor(0.2, 0.2, 0.2, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glBlitFramebuffer(
            0, 0, m_framebuffer_width, m_framebuffer_height,        // rectangle origine dans READ_FRAMEBUFFER
            0, 0, m_framebuffer_width, m_framebuffer_height,        // rectangle destination dans DRAW_FRAMEBUFFER
            GL_COLOR_BUFFER_BIT, GL_LINEAR);                        // ne copier que la couleur (+ interpoler)

    }
    else {
        normalRender();
    }

    return 1;
}

Transform Renderer::Ortho(const float left, const float right, const float bottom, const float top, const float znear, const float zfar) {
    float tx= - (right + left) / (right - left);
    float ty= - (top + bottom) / (top - bottom);
    float tz= - (zfar + znear) / (zfar - znear);

    return Transform(
        2.f / (right - left),                    0,                     0, tx,
                           0, 2.f / (top - bottom),                     0, ty,
        0,                                       0, -2.f / (zfar - znear), tz,
        0,                                       0,                     0, 1);
}

Transform Renderer::getViewFromSun(vec3 pos) {
    return Lookat(pos, Point(0,0,0), Vector(0,1,0));
}

Transform Renderer::getViewFromLight(vec3 pos) {
    return Lookat(pos, Point(pos.x,0,pos.z+0.1), Vector(0,1,0));
}

void Renderer::shadowMapRender(const Transform& orthoProjShadowMap, const Transform &view) {    
    glViewport(0, 0, m_framebuffer_width, m_framebuffer_height);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    Transform vp = orthoProjShadowMap*view;
    Transform depthMVP;

    int location;

    //Animated objects
    glUseProgram(m_dynamicShadowMap_program);
    for (unsigned i=0; i<m_animatedObj.size(); ++i) {

        depthMVP = vp * m_animatedModels[i];

        location = glGetUniformLocation(m_dynamicShadowMap_program, "depthMVP");
        glUniformMatrix4fv(location, 1, GL_TRUE, depthMVP.buffer());

        const int nbFramePerSec = 24;
        const float frameTime = 1000.f / nbFramePerSec; //En ms
        float time = global_time() / frameTime; //Ex : 1.5 entre frame 1 et 2
        int nbFrame = (int) time % m_animatedObj[i].keyframe_count;
        float interpolationFactor = time - (int) time; //partie fractionnaire

        //Décalage de l'offset en fct du numéro de la frame
        m_animatedObj[i].setPointer(nbFrame);

        glUniform1f(glGetUniformLocation(m_dynamicShadowMap_program, "t"), interpolationFactor);

        glBindVertexArray(m_animatedObj[i].vao);
        glDrawArrays(GL_TRIANGLES, 0, m_animatedObj[i].vertex_count);
    }

    //Static objects
    glUseProgram(m_staticShadowMap_program);
    for (unsigned i=0; i<m_staticObj.size(); ++i) {

        depthMVP = vp * m_staticModels[i];

        location = glGetUniformLocation(m_staticShadowMap_program, "depthMVP");
        glUniformMatrix4fv(location, 1, GL_TRUE, depthMVP.buffer());

        glBindVertexArray(m_staticObj[i].vao);
        glDrawArrays(GL_TRIANGLES, 0, m_staticObj[i].vertex_count);
    }

}

void Renderer::normalRender() {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glViewport(0, 0, window_width(), window_height());
    glClearColor(0.2f, 0.2f, 0.2f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    //On a une texture depth par lumière..
    //Le soleil:
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_depth_buffer1);
    //Les street lamp:
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, m_depth_buffer2);
    glActiveTexture(GL_TEXTURE0 + 2);
    glBindTexture(GL_TEXTURE_2D, m_depth_buffer3);
    glActiveTexture(GL_TEXTURE0 + 3);
    glBindTexture(GL_TEXTURE_2D, m_depth_buffer4);

    putLightsInSSBO();
    renderAnimatedObj();
    renderStaticObj();
}

void Renderer::renderAnimatedObj() {

    glUseProgram(m_programDynamic);

    glUniform1i(glGetUniformLocation(m_programDynamic, "shadowMapSun"), 0);
    glUniform1i(glGetUniformLocation(m_programDynamic, "shadowMapLamp1"), 1);
    glUniform1i(glGetUniformLocation(m_programDynamic, "shadowMapLamp2"), 2);
    glUniform1i(glGetUniformLocation(m_programDynamic, "shadowMapLamp3"), 3);

    for (unsigned i=0; i<m_animatedObj.size(); ++i) {

        //ssbo materials
        putMaterialsInSSBO(m_animatedObj[i]);
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

void Renderer::renderStaticObj() {
    
    glUseProgram(m_programStatic);
    glUniform1i(glGetUniformLocation(m_programStatic, "shadowMapSun"), 0);
    glUniform1i(glGetUniformLocation(m_programStatic, "shadowMapLamp1"), 1);
    glUniform1i(glGetUniformLocation(m_programStatic, "shadowMapLamp2"), 2);
    glUniform1i(glGetUniformLocation(m_programStatic, "shadowMapLamp3"), 3);

    for (unsigned i=0; i<m_staticObj.size(); ++i) {

        //ssbo materials
        putMaterialsInSSBO(m_staticObj[i]);
        setUniform(m_programStatic, m_staticModels[i]);

        //TODO stocker ça dans info objets! Buffers == obj?
        const float F0 = 1.f;
        glUniform1f(glGetUniformLocation(m_programStatic, "F0"), F0);

        const float alpha = 0.5f;
        glUniform1f(glGetUniformLocation(m_programStatic, "alpha"), alpha);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_ssbo);

        glBindVertexArray(m_staticObj[i].vao);
        glDrawArrays(GL_TRIANGLES, 0, m_staticObj[i].vertex_count);
    }
}

void Renderer::setUniform(GLuint program, const Transform& model) {
    Transform view = m_camera.view();
    Transform projection = m_camera.projection(window_width(), window_height(), 45);
    
    Transform mvp = projection * view * model;
    Transform mv = view * model;

    //Shadow map
    Transform mvpDepthSun = orthoProjShadowMapSun*getViewFromSun(m_lights[0].pos)*model;
    Transform mvpDepthLamp1 = orthoProjShadowMapLamp*getViewFromLight(m_lights[1].pos)*model;
    Transform mvpDepthLamp2 = orthoProjShadowMapLamp*getViewFromLight(m_lights[2].pos)*model;
    Transform mvpDepthLamp3 = orthoProjShadowMapLamp*getViewFromLight(m_lights[3].pos)*model;

    int location;

    glUseProgram(program);
    //TODO factoriser lumières
    location = glGetUniformLocation(program, "depthMVP_Sun");
    glUniformMatrix4fv(location, 1, GL_TRUE, mvpDepthSun.buffer());
    location = glGetUniformLocation(program, "depthMVP_Lamp1");
    glUniformMatrix4fv(location, 1, GL_TRUE, mvpDepthLamp1.buffer());
    location = glGetUniformLocation(program, "depthMVP_Lamp2");
    glUniformMatrix4fv(location, 1, GL_TRUE, mvpDepthLamp2.buffer());
    location = glGetUniformLocation(program, "depthMVP_Lamp3");
    glUniformMatrix4fv(location, 1, GL_TRUE, mvpDepthLamp3.buffer());

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
    location = glGetUniformLocation(program, "mvpNormal");
    glUniformMatrix4fv(location, 1, GL_TRUE, mvp.normal().buffer());
    //Camera pos
    location = glGetUniformLocation(program, "camera");
    Point p = m_camera.position(); //Repère du monde
    glUniform3fv(location, 1, &p.x);
}

// destruction des objets de l'application
int Renderer::quit() {
    for (unsigned i=0; i<m_animatedObj.size(); ++i)
        m_animatedObj[i].release();
    for (unsigned i=0; i<m_staticObj.size(); ++i)
        m_staticObj[i].release();
    
    glDeleteBuffers(1, &m_ssbo);
    glDeleteBuffers(1, &m_framebuffer);

    glDeleteTextures(1, &m_color_buffer);
    glDeleteTextures(1, &m_depth_buffer1);
    glDeleteTextures(1, &m_depth_buffer2);
    glDeleteTextures(1, &m_depth_buffer3);
    glDeleteTextures(1, &m_depth_buffer4);
    glDeleteSamplers(1, &m_color_sampler);

    //TODO glDeleteShader une seule fois pour le fragment, qui est lié aux deux à chaque fois
    release_program(m_staticShadowMap_program);
    release_program(m_dynamicShadowMap_program);
    release_program(m_programDynamic);
    release_program(m_programStatic);

    return 0;
}