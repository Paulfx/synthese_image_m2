#version 330 core

#ifdef VERTEX_SHADER

layout(location= 0) in vec3 position0;
layout(location= 1) in vec3 normal0;

layout(location= 2) in vec3 position1;
layout(location= 3) in vec3 normal1;

uniform mat4 depthMVP;

uniform float t; //Entre 0 et 1

void main( )
{
    vec3 position = mix(position0, position1, t);
    
   	gl_Position= depthMVP * vec4(position, 1);
}

#endif

#ifdef FRAGMENT_SHADER

//uniform sampler2D c_texture;

// Ouput data
layout(location = 0) out float fragmentdepth;

out vec4 fragment_color;

void main(){
    // Not really needed, OpenGL does it anyway
    fragmentdepth = gl_FragCoord.z;

//    fragment_color = vec4(gl_FragCoord.z,0,0,1);

}


#endif