
//! \file vertex_compute.glsl

#version 430

#ifdef COMPUTE_SHADER

layout(std430, binding= 1) readonly buffer vertexData
{
    vec3 data[];
};

layout(std430, binding= 0) writeonly buffer transformedData
{
    vec4 transformed[];
};

uniform mat4 mvpMatrix;
uniform mat4 mvpInvMatrix;

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

layout(local_size_x= 256) in;
void main( )
{

	// construction du rayon pour le pixel, passage depuis le repere projectif
    vec4 oh= mvpInvMatrix * vec4(position, 0, 1);       // origine sur near
    vec4 eh= mvpInvMatrix * vec4(position, 1, 1);       // extremite sur far


    

    

    // chaque thread transforme un sommet.
    if(gl_LocalInvocationIndex < data.length())
        transformed[gl_GlobalInvocationID.x]= mvpMatrix * vec4(data[gl_GlobalInvocationID.x], 1);
}
#endif

