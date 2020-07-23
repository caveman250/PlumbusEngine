cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
cmake_policy(VERSION 2.8)
#set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
#set(CMAKE_SUPPRESS_REGENERATION true)

# SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
# SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")
# SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -pg")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-format-truncation -Wno-unused-result")
endif (UNIX)

set(NAME PlumbusEngine)

IF(DEFINED CMAKE_BUILD_TYPE)
   SET(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "Was Allready Defined Choose the type of
build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug
Release RelWithDebInfo MinSizeRel." FORCE)
ELSE()
   SET(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build,
options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release
RelWithDebInfo MinSizeRel.")
ENDIF()

IF(CMAKE_BUILD_TYPE MATCHES DEBUG)
	add_compile_definitions(PL_DEBUG=1)
ENDIF(CMAKE_BUILD_TYPE MATCHES DEBUG)

IF(CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)
	add_compile_definitions(PL_RELEASE=1)
ENDIF(CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)

IF(CMAKE_BUILD_TYPE MATCHES Release)
	add_compile_definitions(PL_DIST=1)
ENDIF(CMAKE_BUILD_TYPE MATCHES Release)

project(${NAME})

include_directories(../third_party)
include_directories(../third_party/glm)
include_directories(../third_party/gli)
include_directories(../third_party/assimp/include/)
include_directories(${CMAKE_BINARY_DIR}/third_party/assimp/include/)
include_directories(../third_party/glfw/include/)
include_directories(src)
if(!APPLE)
	include_directories(src/vk)
endif(!APPLE)

include_directories(src/components)

find_package(Vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIRS})

link_directories(${CMAKE_SOURCE_DIR}/lib)

file(GLOB SOURCE src/*.cpp src/*.h)
source_group("src" FILES ${SOURCE})
file(GLOB COMPONENTS src/components/*.cpp src/components/*.h)
source_group("src\\components" FILES ${COMPONENTS})
file(GLOB BASE src/renderer/base/*.cpp src/renderer/base/*.h)
source_group("src\\renderer\\base" FILES ${BASE})
if(APPLE)
	file(GLOB MTL src/renderer/mtl/*.cpp src/renderer/mtl/*.m src/renderer/mtl/*.h src/renderer/mtl/*.hpp src/renderer/mtl/*.mm)
	source_group("src\\renderer\\mtl" FILES ${MTL})
	file(GLOB SHADERS src/renderer/mtl/shaders/*.metal)
	source_group("src\\renderer\\mtl\\shaders" FILES ${SHADERS})
	set_source_files_properties(${SHADERS} PROPERTIES LANGUAGE
METAL)
	file(GLOB PLATFORM src/platform/osx/*.mm src/platform/osx/*.h)
	source_group("src\\platform\\osx" FILES ${PLATFORM})
else(APPLE)
	file(GLOB VK src/renderer/vk/*.cpp src/renderer/vk/*.h)
	source_group("src\\renderer\\vk" FILES ${VK})
	file(GLOB IMGUI src/imgui_impl/*.cpp src/imgui_impl/*.h)
	source_group("src\\imgui_impl" FILES ${IMGUI})
	file(GLOB IMGUIIMGUI ../third_party/imgui/*.cpp ../third_party/imgui/*.h)
	source_group("third_party\\imgui" FILES ${IMGUIIMGUI})
	file(GLOB PLATFORM src/platform/windows/*.cpp src/platform/windows/*.h)
	source_group("src\\platform\\windows" FILES ${PLATFORM})
endif(APPLE)

if(APPLE)
	add_executable(${NAME} src/main.cpp ${SOURCE} ${COMPONENTS} ${MTL} ${BASE} ${PLATFORM} ${SHADERS})
	target_compile_definitions(${NAME} PUBLIC METAL_RENDERER=1 VULKAN_RENDERER=0)
else(APPLE)
	#add_executable(${NAME} src/main.cpp ${SOURCE} ${COMPONENTS} ${VK} ${IMGUI} ${IMGUIIMGUI} ${BASE} ${PLATFORM})
	#target_compile_definitions(${NAME} PUBLIC METAL_RENDERER=0 VULKAN_RENDERER=1)
	add_library(${NAME} STATIC ${SOURCE} ${COMPONENTS} ${VK} ${IMGUI} ${IMGUIIMGUI} ${BASE} ${PLATFORM})
	target_compile_definitions(${NAME} PUBLIC METAL_RENDERER=0 VULKAN_RENDERER=1)
endif(APPLE)

if (WIN32)
	add_compile_definitions(PL_PLATFORM_WINDOWS=1)
else (WIN32)
	add_compile_definitions(PL_PLATFORM_LINUX=1)
	FIND_PACKAGE(PkgConfig REQUIRED)
	PKG_CHECK_MODULES(GTK3 REQUIRED gtk+-3.0)
	INCLUDE_DIRECTORIES(${GTK3_INCLUDE_DIRS})
endif(WIN32)

target_precompile_headers(${NAME} PUBLIC "src/plumbus.h")

