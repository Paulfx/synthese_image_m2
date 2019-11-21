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
uniform mat4 mvpNormal;

uniform mat4 depthMVP_Sun;
out vec4 pDepth_Sun;
uniform mat4 depthMVP_Lamp1;
out vec4 pDepth_Lamp1;
uniform mat4 depthMVP_Lamp2;
out vec4 pDepth_Lamp2;
uniform mat4 depthMVP_Lamp3;
out vec4 pDepth_Lamp3;

void main( )
{
    matIndex = materialIndex;

    //Repère du monde
    p = vec3(mMatrix * vec4(position,1));
    n = mat3(mNormal) * normal; //No translations

    gl_Position= mvpMatrix * vec4(position, 1);

    pDepth_Sun = depthMVP_Sun * vec4(position, 1);
    pDepth_Lamp1 = depthMVP_Lamp1 * vec4(position, 1);
    pDepth_Lamp2 = depthMVP_Lamp2 * vec4(position, 1);
    pDepth_Lamp3 = depthMVP_Lamp3 * vec4(position, 1);
}