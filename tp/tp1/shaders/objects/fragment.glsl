#version 430

#define NUMBER_OF_LIGHTS 2

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

layout(std430, binding=0) readonly buffer lightsAndMaterials {
    Light lights[NUMBER_OF_LIGHTS];

    Material materials[];
};

out vec4 fragment_color;

uniform sampler2D shadowMap;
in vec4 pDepth;

//Repère du monde
in vec3 p;
in vec3 n;
flat in int matIndex;

uniform vec3 camera;


uniform float F0;       //indice de réfraction
uniform float alpha;    //Rugosité

const float PI= 3.14159265359;

//https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping
float shadowCalculations(vec4 positionLightSpace, vec3 normal, vec3 lightDir) {
    vec3 projCoords = positionLightSpace.xyz / positionLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;

    
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    //bias = 0.005;

    //if (projCoords.z > 1.0) return 1.0;
    return currentDepth - bias > closestDepth ? 0.0 : 1.0;
}

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
    // if(cos_theta <= 0 || cos_theta_h <= 0 || cos_theta_o <= 0) {
    //     fragment_color = vec4(0,0,0,1);
    //     return;
    // }
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

    //Projection du point sur sa normale
    float shadow = shadowCalculations(pDepth,nn,l);

    //TODO ALL LIGHTS
    vec3 color= shadow * materials[matIndex].diffuse.rgb * lights[0].intensity * fr * cos_theta;


    fragment_color = vec4(color,1);
}
