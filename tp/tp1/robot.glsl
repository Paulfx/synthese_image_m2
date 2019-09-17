#version 330

#ifdef VERTEX_SHADER

layout(location= 0) in vec3 position0;
layout(location= 1) in vec3 normal0;

layout(location= 2) in vec3 position1;
layout(location= 3) in vec3 normal1;

layout(location= 4) in int materialIndex;


uniform mat4 mMatrix;
uniform mat4 mvpMatrix;
uniform mat4 mvMatrix;
uniform mat4 mvNormal;

uniform float t; //Entre 0 et 1

//flat => pas d'interpolation
flat out int matIndex;
out vec3 vertexPosition;
out vec3 normal;

void main( )
{
    vec3 position = mix(position0, position1, t);
    matIndex = materialIndex;
   	
   	//Repère caméra
   	normal = mix(normal0, normal1, t);
   	normal = mat3(mvNormal) * normal;
   	vertexPosition = vec3(mvMatrix * vec4(position, 1));

   	gl_Position= mvpMatrix * vec4(position, 1);
}

#endif

#ifdef FRAGMENT_SHADER

#define NB_MAT 3

out vec4 fragment_color;

in vec3 vertexPosition;
in vec3 normal;
flat in int matIndex;

uniform vec3 viewPosition;

uniform vec4 diffuse[NB_MAT];
uniform vec4 specular[NB_MAT];
uniform vec4 emission[NB_MAT];
uniform float ns[NB_MAT];


void main( )
{	
	

    //fragment_color= diffuse[matIndex];
    vec3 lightPos = vec3(0,0,0); //Origine caméra
    vec4 lightCol = vec4(1,1,1,1);

    vec3 normalN = normalize(normal);
  	vec3 lightDir = normalize(lightPos - vertexPosition);

    vec4 diffuseCol = diffuse[matIndex] * lightCol * max(dot(lightDir, normalN), 0.0);

    fragment_color = diffuseCol;

    //fragment_color = vec4(abs(normalN),1);

 //    vec4 emissionCol = emission[matIndex] * lightCol;

 //    //Vecteur lumière
 //    vec3 lightDir = normalize(lightPos - vertexPosition);

 //    vec4 diffuseCol = diffuse[matIndex] * lightCol * dot(lightDir, normalN);

 //    vec3 viewDir = normalize(viewPosition - vertexPosition);
    
 //    vec3 reflectDir = reflect(-lightDir, normalN);


	// float spec = pow(max(dot(viewDir, reflectDir), 0.0), ns[matIndex]);
    
 //    vec4 specularCol = specular[matIndex] * spec * lightCol;


 //    fragment_color = (emissionCol + diffuseCol + specularCol);


}

#endif
