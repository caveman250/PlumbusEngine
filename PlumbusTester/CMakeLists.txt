cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
cmake_policy(VERSION 2.8)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_SUPPRESS_REGENERATION true)

# SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
# SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")
# SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -pg")

set(NAME PlumbusTester)

IF(DEFINED CMAKE_BUILD_TYPE)
   SET(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "Was Allready Defined Choose the type of
build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug
Release RelWithDebInfo MinSizeRel.")
ELSE()
   SET(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build,
options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release
RelWithDebInfo MinSizeRel.")
ENDIF()

project(${NAME})

include_directories(../third_party)
include_directories(../third_party/glm)
include_directories(../third_party/gli)
include_directories(../third_party/assimp/include/)
include_directories(../third_party/glfw-3.2.1/include/)
include_directories(../src)
include_directories(src)
include_directories(../third_party/vulkan)

file(GLOB_RECURSE SOURCE src/*.cpp src/*.h)
source_group("src" FILES ${SOURCE})

add_executable(${NAME} src/PlumbusTester.cpp ${SOURCE})
target_compile_definitions(${NAME} PUBLIC METAL_RENDERER=0 VULKAN_RENDERER=1)

target_link_libraries(${NAME} VulkanRenderer)
link_libraries(VulkanRenderer)

# find_library(Vulkan_LIBRARY NAMES vulkan HINTS "${CMAKE_SOURCE_DIR}/lib" "${CMAKE_SOURCE_DIR}/libs/vulkan" REQUIRED)
# IF (Vulkan_LIBRARY)
# 	set(Vulkan_FOUND ON)
# 	MESSAGE("Using bundled Vulkan library version")
# ENDIF()
# target_link_libraries(${NAME} ${Vulkan_LIBRARY})
# link_libraries(${Vulkan_LIBRARY})

find_package(Vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIRS})
target_link_libraries(${NAME} ${Vulkan_LIBRARIES})

target_link_libraries(${NAME} glfw ${GLFW_LIBRARIES})
target_link_libraries(${NAME} assimp)
link_libraries(${ASSIMP_LIBRARIES})

# if (WIN32)
#     target_link_libraries(${NAME} glfw ${GLFW_LIBRARIES})
#     target_link_libraries(${NAME} assimp ${ASSIMP_LIBRARIES})
#     target_link_libraries(${NAME} vulkan-1.lib)
# endif (WIN32)
# if (UNIX AND NOT APPLE)
# 	find_library(Vulkan_LIBRARY NAMES vulkan HINTS "${CMAKE_SOURCE_DIR}/lib" "${CMAKE_SOURCE_DIR}/libs/vulkan" REQUIRED)
# 	IF (Vulkan_LIBRARY)
# 		set(Vulkan_FOUND ON)
# 		MESSAGE("Using bundled Vulkan library version")
# 	ENDIF()
# 	target_link_libraries(${NAME} ${Vulkan_LIBRARY})
# 	link_libraries(${Vulkan_LIBRARY})
	
# endif (UNIX AND NOT APPLE)

# if(UNIX)
#     target_link_libraries(${NAME} glfw ${GLFW_LIBRARIES})
#     target_link_libraries(${NAME} assimp)
# 	link_libraries(${ASSIMP_LIBRARIES})
# endif(UNIX)

# if(APPLE)
# 	ADD_OSX_FRAMEWORK(Metal ${NAME})
# 	ADD_OSX_FRAMEWORK(MetalKit ${NAME})
# 	ADD_OSX_FRAMEWORK(QuartzCore ${NAME})
# endif(APPLE)

