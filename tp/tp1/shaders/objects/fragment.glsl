#version 430

#define NUMBER_OF_LIGHTS 4

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

//TODO factoriser tout ça!!
uniform sampler2D shadowMapSun;
uniform sampler2D shadowMapLamp1;
uniform sampler2D shadowMapLamp2;
uniform sampler2D shadowMapLamp3;

in vec4 pDepth_Sun;
in vec4 pDepth_Lamp1;
in vec4 pDepth_Lamp2;
in vec4 pDepth_Lamp3;

//Repère du monde
in vec3 p;
in vec3 n;
flat in int matIndex;

uniform vec3 camera;

uniform float F0;       //indice de réfraction
uniform float alpha;    //Rugosité

const float PI= 3.14159265359;

//https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping
float shadowCalculations(vec4 positionLightSpace, vec3 normal, vec3 lightDir, sampler2D shadowMap) {
    vec3 projCoords = positionLightSpace.xyz / positionLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;
    
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);

    if (projCoords.z > 1.0 || projCoords.z < 0.0 ) return 0.0;
    return currentDepth - bias > closestDepth ? 0.0 : 1.0;
}

void main( )
{	
    // directions
    vec3 o= normalize(camera - p);
    vec3 nn= normalize(n);

    float cos_theta_o= max(0, dot(nn, o));

    vec3 color = vec3(0,0,0);

    //pour toutes les lumières
    for (int i=0; i<NUMBER_OF_LIGHTS; ++i) {

        vec3 l= normalize(lights[i].position - p);
        vec3 h= normalize(o + l);
        // cos
        float cos_theta= max(0, dot(nn, l));
        float cos_theta_h= max(0, dot(nn, h));
        
        // meme hemisphere
        if(cos_theta <= 0 || cos_theta_h <= 0 || cos_theta_o <= 0) {
            //fragment_color = vec4(0,0,0,1);
            continue;
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
        
        //TODO ajouter un tableau pour stocker les sampler, les pDepth associés à chaque lumière...
        //Pour l'instant si i==0, soleil, sinon lamp i, c'est pas très générique tout ça...
        //Et c'est triste d'avoir un if dans un shader
        float shadow;
        if (i == 0)
            shadow = shadowCalculations(pDepth_Sun,nn,l, shadowMapSun);
        else if (i==1)
            shadow = shadowCalculations(pDepth_Lamp1,nn,l, shadowMapLamp1);
        else if (i==2)
            shadow = shadowCalculations(pDepth_Lamp2,nn,l, shadowMapLamp2);
        else
            shadow = shadowCalculations(pDepth_Lamp3,nn,l, shadowMapLamp3);

        color=      color + 
                    shadow                          * 
                    materials[matIndex].diffuse.rgb * 
                    vec3(lights[i].color)           * 
                    lights[i].intensity             * 
                    fr                              * 
                    cos_theta;

    }

    fragment_color = vec4(color,1);
}
