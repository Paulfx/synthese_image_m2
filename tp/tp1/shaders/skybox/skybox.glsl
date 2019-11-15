#version 330

#ifdef VERTEX_SHADER

layout(location= 0) in vec3 position;

out vec3 tex_coords;

uniform mat4 view;
uniform mat4 projection;

void main( )
{
	tex_coords = normalize(position);
    gl_Position= projection * view * vec4(position, 1.0);
}

#endif

#ifdef FRAGMENT_SHADER

out vec4 fragment_color;

in vec3 tex_coords;

uniform samplerCube skybox;

void main( )
{
    fragment_color= texture(skybox, tex_coords);
}

#endif
