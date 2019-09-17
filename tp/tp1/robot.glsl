#version 330

#ifdef VERTEX_SHADER

layout(location= 0) in vec3 position0;
layout(location= 1) in vec3 position1;
//layout(location= 2) in int materialIndex;

uniform mat4 mvpMatrix;

uniform float time; //Entre 0 et 1

//flat => pas d'interpolation
//flat out int matIndex;

void main( )
{

    vec3 position = mix(position0, position1, time);

    //matIndex = materialIndex;
   	
   	gl_Position= mvpMatrix * vec4(position, 1);
}

#endif

#ifdef FRAGMENT_SHADER

#define NB_MAT 3

out vec4 fragment_color;

//flat in int matIndex;

// uniform vec4 diffuse[NB_MAT];
// uniform vec4 specular[NB_MAT];
// uniform vec4 emission[NB_MAT];
// uniform float ns[NB_MAT];

void main( )
{	
	vec4 color;
	// if (matIndex == 0) color = vec4(1,0,0,1);
	// if (matIndex == 1) color = vec4(0,1,0,1);
	// if (matIndex == 2) color = vec4(0,0,1,1);
	// if (matIndex == 3) color = vec4(1,0,1,1);
    
    //fragment_color= color;
    fragment_color= vec4(1,0,1,1);
}

#endif
