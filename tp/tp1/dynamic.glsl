#version 430

#ifdef VERTEX_SHADER

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

uniform float t; //Entre 0 et 1

//flat => pas d'interpolation
flat out int matIndex;
out vec3 p;
out vec3 n;

void main( )
{
    vec3 position = mix(position0, position1, t);
    matIndex = materialIndex;
   	
   	//Repère du monde
   	n = mat3(mvNormal) * mix(normal0, normal1, t);
   	p = vec3(mvMatrix * vec4(position, 1));

   	gl_Position= mvpMatrix * vec4(position, 1);
}

#endif







#ifdef FRAGMENT_SHADER

//TODO same for static!!

struct Light {
    vec3 position;
    float intensity;
    vec4 color;
};

struct Material {
    vec4 emission;
    vec4 diffuse;
    vec4 specular;
    float ns;
};

#define NUMBER_OF_LIGHTS 2

layout(std430, binding=0) readonly buffer lightsAndMaterials {
    Light lights[NUMBER_OF_LIGHTS];

    Material materials[];
};

out vec4 fragment_color;

in vec3 p;
in vec3 n;
flat in int matIndex;

uniform vec3 camera;

//TODO use storage buffers to store light + materials..
//No predefined size

// #define NB_LIGHTS 2
// uniform vec3 lightPosition[NB_LIGHTS];
// uniform vec4 lightColor[NB_LIGHTS];

// uniform vec4 diffuse[NB_MAT];
// uniform vec4 specular[NB_MAT];
// uniform vec4 emission[NB_MAT];
// uniform float ns[NB_MAT];

uniform float F0;       //indice de réfraction
uniform float alpha;    //Rugosité

const float PI= 3.14159265359;

void main( )
{	
    // directions
// vec3 o= normalize(camera - p);
// vec3 l= normalize(lightPosition[0] - p);
// vec3 h= normalize(o + l);
// float cos_theta_h= dot(normalize(n), h);
// // cos
// float cos_theta= max(0, dot(normalize(n), l));
// // brdf
// float fr= (alpha+8) / (8*PI) * pow(cos_theta_h, alpha);
// vec3 color= diffuse[matIndex].rgb * fr * cos_theta;    

// fragment_color = vec4(color,1);

    // directions
    vec3 o= normalize(camera - p);
    vec3 l= normalize(lights[0].position - p);
    vec3 h= normalize(o + l);
    // cos
    vec3 nn= normalize(n);
    float cos_theta= max(0, dot(nn, l));
    float cos_theta_h= max(0, dot(nn, h));
    float cos_theta_o= max(0, dot(nn, o));
    // meme hemisphere
    if(cos_theta <= 0 || cos_theta_h <= 0 || cos_theta_o <= 0) {
        fragment_color = vec4(0,0,0,1);
        return;
    }
    // D
    float D= alpha / (1 + cos_theta_h*cos_theta_h * (alpha*alpha -1));
    D= D*D / PI;
    // G2
    float tan2_theta_o= 1 / (cos_theta_o*cos_theta_o) - 1;
    float lambda_o= 1 + alpha*alpha * tan2_theta_o;
    float tan2_theta= 1 / (cos_theta*cos_theta) - 1;
    float lambda= 1 + alpha*alpha * tan2_theta;
    float G2= 2 / (sqrt(lambda_o) + sqrt(lambda));
    // F
    float F= F0 + (1 - F0) * pow(1 - dot(o, h), 5);
    // brdf
    float fr= (F * D * G2) / (4 * cos_theta_o * cos_theta);

    //Couleur du soleil

    //vec3 sunColor = mix(vec3(1,0,0), vec3(1,1,1) , abs(lights[0].position.y) / 175);


    vec3 color=  materials[matIndex].diffuse.rgb * lights[0].intensity * fr * cos_theta;


    fragment_color = vec4(color,1);    
    //fragment_color = vec4(nn,1);

    //fragment_color = lights[0].color;
}

#endif
