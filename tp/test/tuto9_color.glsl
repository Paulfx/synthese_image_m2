//! \file tuto9_color.glsl

#version 330

#ifdef VERTEX_SHADER
layout(location= 0) in vec3 position0;
layout(location= 1) in vec3 position1;

uniform mat4 mvpMatrix;
uniform float interpolationFactor;

void main( )
{
    vec3 position = mix(position0, position1, interpolationFactor);
    gl_Position= mvpMatrix * vec4(position, 1);
}

#endif


#ifdef FRAGMENT_SHADER
out vec4 fragment_color;

uniform vec4 color;

void main( )
{
    fragment_color= color;
}

#endif
