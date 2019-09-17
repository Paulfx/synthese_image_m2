#version 330

#ifdef VERTEX_SHADER

layout(location= 0) in vec3 position0;
layout(location= 1) in vec3 position1;

layout(location= 2) in int materialIndex;

uniform mat4 mvpMatrix;

uniform float time; //Entre 0 et 1

//flat => pas d'interpolation
flat out int matIndex;

void main( )
{

    vec3 position = mix(position0, position1, time);

    matIndex = materialIndex;
   	
   	gl_Position= mvpMatrix * vec4(position, 1);
}

#endif

#ifdef FRAGMENT_SHADER

#define NB_MAT 3

out vec4 fragment_color;

flat in int matIndex;

uniform vec4 diffuse[NB_MAT];
uniform vec4 specular[NB_MAT];
uniform vec4 emission[NB_MAT];
uniform float ns[NB_MAT];

void main( )
{	
	vec4 color;


	//fragment_color= vec4(matIndex/2.0, matIndex/2.0, matIndex/2.0, 1);
    //fragment_color= color;
    fragment_color= diffuse[matIndex];
}

#endif
