#version 330 core

#ifdef VERTEX_SHADER

layout(location = 0) in vec3 position;

// Values that stay constant for the whole mesh.
uniform mat4 depthMVP;

void main(){
 gl_Position =  depthMVP * vec4(position,1);
}

#endif

#ifdef FRAGMENT_SHADER

//uniform sampler2D c_texture;

// Ouput data
layout(location = 0) out float fragmentdepth;

//out vec4 fragment_color;

void main(){
    // Not really needed, OpenGL does it anyway
    fragmentdepth = gl_FragCoord.z;

    //fragment_color = vec4(gl_FragCoord.z,0,0,1);

}


#endif