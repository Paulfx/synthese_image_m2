#version 430

layout(location= 0) in vec3 position0;
layout(location= 1) in vec3 normal0;

layout(location= 2) in vec3 position1;
layout(location= 3) in vec3 normal1;

layout(location= 4) in int materialIndex;


uniform mat4 mvpMatrix;
uniform mat4 mMatrix;
uniform mat4 mvMatrix;
uniform mat4 mNormal;
uniform mat4 mvNormal;

uniform mat4 depthMVP;
out vec4 pDepth;
out vec3 nDepth;

uniform float t; //Entre 0 et 1

//flat => pas d'interpolation
flat out int matIndex;
out vec3 p;
out vec3 n;

void main( )
{
    vec3 position = mix(position0, position1, t);
    matIndex = materialIndex;
   	
   	vec3 normal = mix(normal0, normal1, t);
   	//Repère du monde
   	n = mat3(mvNormal) * normal;
   	p = vec3(mvMatrix * vec4(position, 1));

   	gl_Position= mvpMatrix * vec4(position, 1);

    pDepth = depthMVP * vec4(position, 1);
    nDepth = mat3(depthMVP) * normal;

}
