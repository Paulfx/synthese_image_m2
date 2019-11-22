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


sampler2D getSampler(int i) {
    switch(i) {
        case 0: return shadowMapSun;
        case 1: return shadowMapLamp1;
        case 2: return shadowMapLamp2;
        case 3: return shadowMapLamp3;
    }
}

vec3 computeColorFromLight(vec3 o, vec3 nn, float cos_theta_o, Light light) {

    vec3 l= normalize(light.position - p);
    vec3 h= normalize(o + l);
    // cos
    float cos_theta= max(0, dot(nn, l));
    float cos_theta_h= max(0, dot(nn, h));
    
    // meme hemisphere
    if(cos_theta <= 0 || cos_theta_h <= 0 || cos_theta_o <= 0) {
        return vec3(0,0,0);
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
    

    return  materials[matIndex].diffuse.rgb * 
            vec3(light.color)               * 
            light.intensity                 * 
            fr                              * 
            cos_theta;
}

bool isInSpot(vec3 vertexPos, vec3 lightPos, out float intensity) {
    vec3 lightDirection = vertexPos - lightPos;
    vec3 down = vec3(0,-1,0);

    float angle = acos(dot(down,normalize(lightDirection)));

    const float maxAngle = 0.7f;
    if (angle > maxAngle) 
        return false;

    //Compute intensity
    //distance au sol
    float l = tan(angle) * lightPos.y;
    const float lMax = tan(maxAngle) * lightPos.y; 
    //Plus il est loin, moins il éclaire
    intensity = 1 - l/lMax;

    return true;
}

void main( )
{	
    // directions
    vec3 o= normalize(camera - p);
    vec3 nn= normalize(n);

    float cos_theta_o= max(0, dot(nn, o));

    
    vec4 pDepths[NUMBER_OF_LIGHTS] = vec4[](pDepth_Sun, 
                                            pDepth_Lamp1,
                                            pDepth_Lamp2,
                                            pDepth_Lamp3);

    vec3 color = vec3(0,0,0);

    vec3 tempColor;
    float shadow;
    float spotIntensity;
    bool spot;
    //pour toutes les lumières
    for (int i=0; i<NUMBER_OF_LIGHTS; ++i) {
        spotIntensity = 1;
        spot = true;
        //Le soleil n'est pas un spot
        if (i != 0)
             spot = isInSpot(p,lights[i].position, spotIntensity);
        
        if (spot) {
            //Si l'objet est éclairé, on calcule sa brdf + shadow
            tempColor = computeColorFromLight(o, nn, cos_theta_o, lights[i]);
            shadow = shadowCalculations(pDepths[i], nn, normalize(lights[i].position - p), getSampler(i));
            color += shadow * tempColor * spotIntensity;
        }

        //color += materials[matIndex].emission.rgb * lights[i].intensity / 7.f;

    }

    fragment_color = vec4(color,1);
}
