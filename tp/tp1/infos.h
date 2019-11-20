#ifndef INFOS_H
#define INFOS_H

#include "mesh.h" //Material

namespace glsl 
{
    template < typename T >
    struct alignas(8) gvec2
    {
        alignas(4) T x, y;
        
        gvec2( ) {}
        gvec2( const ::vec2& v ) : x(v.x), y(v.y) {}
    };
    
    typedef gvec2<float> vec2;
    typedef gvec2<int> ivec2;
    typedef gvec2<unsigned int> uvec2;
    typedef gvec2<int> bvec2;
    
    template < typename T >
    struct alignas(16) gvec3
    {
        alignas(4) T x, y, z;
        
        gvec3( ) {}
        gvec3( const ::vec3& v ) : x(v.x), y(v.y), z(v.z) {}
        gvec3( const Point& v ) : x(v.x), y(v.y), z(v.z) {}
        gvec3( const Vector& v ) : x(v.x), y(v.y), z(v.z) {}
    };
    
    typedef gvec3<float> vec3;
    typedef gvec3<int> ivec3;
    typedef gvec3<unsigned int> uvec3;
    typedef gvec3<int> bvec3;
    
    template < typename T >
    struct alignas(16) gvec4
    {
        alignas(4) T x, y, z, w;
        
        gvec4( ) {}
        gvec4( const ::vec4& v ) : x(v.x), y(v.y), z(v.z), w(v.w) {}
    };
    
    typedef gvec4<float> vec4;
    typedef gvec4<int> ivec4;
    typedef gvec4<unsigned int> uvec4;
    typedef gvec4<int> bvec4;
}

enum ObjectType {
	ANIMATED,
	STATIC
};

enum ObjectIndex {
	ROBOT=0,
	DOG=1,
};

typedef struct Light {

	vec3 pos;
	float intensity;
	glsl::vec4 color;

	Light(Point _pos, float _int, Color _c) : pos(_pos), intensity(_int), color(_c) 
	{};


} Light;

typedef struct Material_glsl {

	glsl::vec4 emission;
	glsl::vec4 diffuse;
	glsl::vec4 specular;
	float ns;

    Material_glsl(const Material& mat) : emission(mat.emission), diffuse(mat.diffuse), specular(mat.specular), ns(mat.ns) {};

} Material_glsl;


#endif // INFOS_H
