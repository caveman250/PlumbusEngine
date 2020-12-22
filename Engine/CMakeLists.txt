cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
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

if (NOT MSVC AND NOT ${CMAKE_SYSTEM_NAME} MATCHES Android)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-format-truncation -Wno-unused-result")
endif ()

if(${CMAKE_SYSTEM_NAME} MATCHES Android)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-result")
endif()

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

project(${NAME})

include_directories(../third_party)
include_directories(../third_party/glm)
include_directories(../third_party/gli)
include_directories(../third_party/assimp/include/)
include_directories(../third_party/glfw/include/)
include_directories(${CMAKE_BINARY_DIR}/third_party/assimp/include/)
include_directories(src)
include_directories(src/components)

if(${CMAKE_SYSTEM_NAME} MATCHES Android)
    include_directories(${ANDROID_NDK}/sources/android/native_app_glue)
endif()

find_package(Vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIRS})

link_directories(${CMAKE_SOURCE_DIR}/lib)

file(GLOB SOURCE src/*.cpp src/*.h)
source_group("src" FILES ${SOURCE})
file(GLOB COMPONENTS src/components/*.cpp src/components/*.h)
source_group("src\\components" FILES ${COMPONENTS})

file(GLOB VK src/renderer/vk/*.cpp src/renderer/vk/*.h)
source_group("src\\renderer\\vk" FILES ${VK})

file(GLOB SHADER_COMPILER src/renderer/vk/shader_compiler/*.cpp src/renderer/vk/shader_compiler/*.h)
source_group("src\\renderer\\vk\\shader_compiler" FILES ${SHADER_COMPILER})

file(GLOB SHADER_COMPILER_PARSERS src/renderer/vk/shader_compiler/parsers/*.cpp src/renderer/vk/shader_compiler/parsers/*.h)
source_group("src\\renderer\\vk\\shader_compiler\\builders" FILES ${SHADER_COMPILER_PARSERS})

file(GLOB IMGUI src/imgui_impl/*.cpp src/imgui_impl/*.h)
source_group("src\\imgui_impl" FILES ${IMGUI})

file(GLOB IMGUIIMGUI ../third_party/imgui/*.cpp ../third_party/imgui/*.h)
source_group("third_party\\imgui" FILES ${IMGUIIMGUI})

file(GLOB PLATFORM src/platform/windows/*.cpp src/platform/windows/*.h)
source_group("src\\platform\\windows" FILES ${PLATFORM})

add_library(${NAME} STATIC ${SOURCE} ${COMPONENTS} ${VK} ${SHADER_COMPILER} ${SHADER_COMPILER_PARSERS} ${IMGUI} ${IMGUIIMGUI} ${BASE} ${PLATFORM})

if (UNIX AND NOT APPLE AND NOT ${CMAKE_SYSTEM_NAME} MATCHES Android )
    FIND_PACKAGE(PkgConfig REQUIRED)
    PKG_CHECK_MODULES(GTK3 REQUIRED gtk+-3.0)
    INCLUDE_DIRECTORIES(${GTK3_INCLUDE_DIRS})
endif(UNIX AND NOT APPLE AND NOT ${CMAKE_SYSTEM_NAME} MATCHES Android )

if (WIN32)
    add_definitions(-DPL_PLATFORM_WINDOWS=1)
endif(WIN32)

if (UNIX)
	if(APPLE)
        add_definitions(-DPL_PLATFORM_OSX=1)
    elseif(${CMAKE_SYSTEM_NAME} MATCHES "Android")
		add_definitions(-DPL_PLATFORM_ANDROID=1)
    else ()
        add_definitions(-DPL_PLATFORM_LINUX=1)
	endif()
endif(UNIX)

message(${CMAKE_SYSTEM_NAME} )
if (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Android")
    target_precompile_headers(${NAME} PUBLIC "src/plumbus.h")
endif (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Android")

