#pragma once

#if PL_PLATFORM_WINDOWS
    #define MONO_EXPORT(RetType) extern "C" __declspec(dllexport) RetType
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
struct mono_vec3
{
	mono_vec3(const glm::vec3& vec)
		: x(std::move(vec.x))
		, y(std::move(vec.y))
		, z(std::move(vec.z))
	{}
	float x; float y; float z;
};
