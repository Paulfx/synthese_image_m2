#version 330 core

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