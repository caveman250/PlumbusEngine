cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
cmake_policy(VERSION 2.8)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (NOT MSVC AND NOT ${CMAKE_SYSTEM_NAME} MATCHES Android)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-format-truncation -Wno-unused-result -fPIC")
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
include_directories(../third_party/glslang/)
include_directories(${CMAKE_BINARY_DIR}/third_party/assimp/include/)
include_directories(src)
include_directories(src/components)

if(${CMAKE_SYSTEM_NAME} MATCHES Android)
    include_directories(${ANDROID_NDK}/sources/android/native_app_glue)
endif()

find_package(Vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIRS})

link_directories(${CMAKE_SOURCE_DIR}/lib)

file(GLOB_RECURSE SOURCE src/*.cpp src/*.h)
source_group("src" FILES ${SOURCE})

file(GLOB IMGUIIMGUI ../third_party/imgui/*.cpp ../third_party/imgui/*.h)
source_group("third_party\\imgui" FILES ${IMGUIIMGUI})

file(GLOB PLATFORM src/platform/windows/*.cpp src/platform/windows/*.h)
source_group("src\\platform\\windows" FILES ${PLATFORM})

add_library(${NAME} STATIC ${SOURCE} ${IMGUIIMGUI})

find_package(Vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIRS})
target_link_libraries(${NAME} ${Vulkan_LIBRARIES})

if(NOT ${CMAKE_SYSTEM_NAME} MATCHES Android)
    target_link_libraries(${NAME} glfw ${GLFW_LIBRARIES})
endif()

target_link_libraries(${NAME} assimp)
link_libraries(${ASSIMP_LIBRARIES})

target_link_libraries(${NAME} SPIRV)
target_link_libraries(${NAME} spirv-cross-core)
target_link_libraries(${NAME} glslang)

if(${CMAKE_SYSTEM_NAME} MATCHES Android)
    target_link_libraries(${NAME} ${android-log-lib} android)
endif()

message("CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}")
if (UNIX AND NOT APPLE AND NOT ${CMAKE_SYSTEM_NAME} MATCHES Android)
    FIND_PACKAGE(PkgConfig REQUIRED)
    PKG_CHECK_MODULES(GTK3 REQUIRED gtk+-3.0)
    INCLUDE_DIRECTORIES(${GTK3_INCLUDE_DIRS})
    LINK_DIRECTORIES(${GTK3_LIBRARY_DIRS})
    ADD_DEFINITIONS(${GTK3_CFLAGS_OTHER})
    TARGET_LINK_LIBRARIES(${NAME} ${GTK3_LIBRARIES})
endif(UNIX AND NOT APPLE AND NOT ${CMAKE_SYSTEM_NAME} MATCHES Android)

add_compile_definitions(DLL_EXPORTS)
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

