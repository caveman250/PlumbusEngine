cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
cmake_policy(VERSION 2.8)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

set(NAME VulkanRenderer)

project(${NAME})

if(UNIX)
add_subdirectory(include/glfw-3.2.1)
endif(UNIX)

include_directories(include)
include_directories(include/glm)
include_directories(include/gli)
include_directories(include/assimp)
include_directories(include/glfw-3.2.1/include/GLFW)
include_directories(include/vulkan)
include_directories(src)
include_directories(src/vk)
include_directories(src/components)

link_directories(${CMAKE_SOURCE_DIR}/lib)

file(GLOB SOURCE src/*.cpp src/*.h)
source_group("src" FILES ${SOURCE})
file(GLOB COMPONENTS src/components/*.cpp src/components/*.h)
source_group("src\\components" FILES ${COMPONENTS})
file(GLOB VK src/vk/*.cpp src/vk/*.h)
source_group("src\\vk" FILES ${VK})

add_executable(${NAME} src/main.cpp ${SOURCE} ${COMPONENTS} ${VK})


if (WIN32)
    target_link_libraries(${NAME} assimp.lib assimp-vc140-mt.lib glfw3.lib vulkan-1.lib)
endif (WIN32)
if (UNIX)
	find_library(Vulkan_LIBRARY NAMES vulkan HINTS "${CMAKE_SOURCE_DIR}/lib" "${CMAKE_SOURCE_DIR}/libs/vulkan" REQUIRED)
	IF (Vulkan_LIBRARY)
		set(Vulkan_FOUND ON)
		MESSAGE("Using bundled Vulkan library version")
	ENDIF()
	find_package(ASSIMP REQUIRED)
    target_link_libraries(${NAME} glfw ${GLFW_LIBRARIES})
    target_link_libraries(${NAME} ${Vulkan_LIBRARY} ${ASSIMP_LIBRARIES})
    link_libraries(${Vulkan_LIBRARY} ${ASSIMP_LIBRARIES})
endif (UNIX)

