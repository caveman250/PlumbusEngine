#pragma once

#if PL_PLATFORM_WINDOWS
    #define MONO_EXPORT(RetType) extern "C" __declspec(dllexport) RetType  __cdecl
#else
    #define MONO_EXPORT(RetType) extern "C" RetType __attribute__((visibility("default")))
#endif


#include <memory>

namespace plumbus::mono
{
	class Class;
	typedef std::shared_ptr<Class> ClassRef;

	class ObjectInstance;
	typedef std::shared_ptr<ObjectInstance> ObjectInstanceRef;
}

#include <glm/vec3.hpp>
struct mono_vec2
{
    mono_vec2(const glm::vec2& vec)
            : x(std::move(vec.x))
            , y(std::move(vec.y))
    {}
    float x; float y;
};

struct mono_vec3
{
	mono_vec3(const glm::vec3& vec)
		: x(std::move(vec.x))
		, y(std::move(vec.y))
		, z(std::move(vec.z))
	{}
	float x; float y; float z;
};

struct mono_mat4
{
	mono_mat4(const glm::mat4& mat)
		: x0(mat[0].x),  y0(mat[0].y),  z0(mat[0].z),  w0(mat[0].w)
		, x1(mat[1].x),  y1(mat[1].y),  z1(mat[1].z),  w1(mat[1].w)
		, x2(mat[2].x),  y2(mat[2].y),  z2(mat[2].z),  w2(mat[2].w)
		, x3(mat[3].x),  y3(mat[3].y),  z3(mat[3].z),  w3(mat[3].w)
	{}
	float x0, x1, x2, x3;
	float y0, y1, y2, y3;
	float z0, z1, z2, z3;
	float w0, w1, w2, w3;
};
