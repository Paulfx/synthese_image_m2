
//! \file vertex_compute.glsl

#version 430

#ifdef COMPUTE_SHADER

layout(rgba32f, binding = 0) uniform image2D img_output;

layout(std430, binding= 1) readonly buffer primitiveData
{
    vec3 triangles[];
};


#define inf 999999.0

//Intersection rayon/triangle
float triangle(const in vec3 o, const in vec3 d, const in vec3 a, const in vec3 b, const in vec3 c) {

    vec3 ab= b-a;
    vec3 ac= c-a;

    vec3 pvec= cross(d,ac);
    float inv_det= 1/dot(ab,pvec);

    vec3 tvec = o-a;

    float u = dot(tvec,pvec) * inv_det;
    if (u<0 || u>1) return inf;

    vec3 qvec = cross(tvec,ab);
    float v= dot(d, qvec) * inv_det;
    if (v<0 || u+v > 1) return inf;

    float t= dot(ac, qvec) * inv_det;
    if (t>inf || t<0) return inf;

    return t;
}

// uniform mat4 mvpMatrix;
// uniform mat4 mvpInvMatrix;

//Taille des groupes locaux
layout(local_size_x= 1, local_size_y=1) in;
void main( )
{
	//position du pixel dans l'image
	ivec2 position = ivec2(gl_GlobalInvocationID.xy);

	// construction du rayon pour le pixel, passage depuis le repere projectif
    // vec4 oh= mvpInvMatrix * vec4(position, 0, 1);       // origine sur near
    // vec4 eh= mvpInvMatrix * vec4(position, 1, 1);       // extremite sur far

    // // origine et direction
    // vec3 o= oh.xyz / oh.w;                              // origine
    // vec3 d= eh.xyz / eh.w - oh.xyz / oh.w;              // direction
    // d= normalize(d);

    
    //Version 1 :
    //1 groupe par rayon == par pixel de l'image
    //Qui teste toutes les primitives (local_size devrait être le nombre de primitives..)

    vec4 color = vec4(1,0,0,1);

    //Output
    imageStore(img_output, position, color);

    //Version 2: 1 groupe par primitives, qui teste tous les rayons de l'image
    //Et stocke le min en atomicMin, ou min dans une passe après

    //Version 3: 1 thread par paire (rayon,primitive)

}
#endif

