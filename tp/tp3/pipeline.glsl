#version 430

#ifdef VERTEX_SHADER

out vec2 position;

void main( )
{
    vec2 positions[3]= vec2[3]( vec2(-1,-3), vec2(3, 1), vec2(-1, 1) );
    
    position= positions[gl_VertexID];
    gl_Position= vec4(positions[gl_VertexID], 0, 1);
}
#endif

#ifdef FRAGMENT_SHADER

uniform sampler2D tex;

in vec2 position;
out vec4 fragment_color;

void main( )
{
	vec4 color = texture(tex, position);
    fragment_color= color;
}
#endif

