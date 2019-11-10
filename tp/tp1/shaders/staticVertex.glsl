#version 430

layout(location= 0) in vec3 position;
out vec3 p;

layout(location= 1) in vec3 normal;
out vec3 n;

layout(location= 2) in int materialIndex;
flat out int matIndex;

uniform mat4 mvpMatrix;
uniform mat4 mMatrix;
uniform mat4 mvMatrix;
uniform mat4 mNormal;
uniform mat4 mvNormal;

uniform mat4 depthMVP;
out vec4 pDepth;
out vec3 nDepth;

void main( )
{

    matIndex = materialIndex;

    //Repère du monde
    p = vec3(mvMatrix * vec4(position,1));
    n = mat3(mvNormal) * normal; //No translations

    gl_Position= mvpMatrix * vec4(position, 1);

    pDepth = depthMVP * vec4(position, 1);
    nDepth = mat3(depthMVP) * normal;
}