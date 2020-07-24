cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
cmake_policy(VERSION 2.8)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_SUPPRESS_REGENERATION true)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-format-truncation -Wno-unused-result")
endif (UNIX)

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
include_directories(../third_party/imgui/)
include_directories(../third_party/glfw/include/)
include_directories(../Engine/src)
include_directories(src)
include_directories(../third_party/vulkan)
include_directories(../third_party/SPIRV-Cross)

if (WIN32)
	add_compile_definitions(PL_PLATFORM_WINDOWS=1)
else (WIN32)
	add_compile_definitions(PL_PLATFORM_LINUX=1)
endif(WIN32)

file(GLOB_RECURSE SOURCE src/*.cpp src/*.h)
source_group("src" FILES ${SOURCE})

add_executable(${NAME} src/PlumbusTester.cpp ${SOURCE})

target_link_libraries(${NAME} PlumbusEngine)
link_libraries(PlumbusEngine)

find_package(Vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIRS})
target_link_libraries(${NAME} ${Vulkan_LIBRARIES})

target_link_libraries(${NAME} glfw ${GLFW_LIBRARIES})
target_link_libraries(${NAME} assimp)
link_libraries(${ASSIMP_LIBRARIES})

target_link_libraries(${NAME} spirv-cross-core)

if (UNIX)
	FIND_PACKAGE(PkgConfig REQUIRED)
	PKG_CHECK_MODULES(GTK3 REQUIRED gtk+-3.0)
	INCLUDE_DIRECTORIES(${GTK3_INCLUDE_DIRS})
	LINK_DIRECTORIES(${GTK3_LIBRARY_DIRS})
	ADD_DEFINITIONS(${GTK3_CFLAGS_OTHER})
	TARGET_LINK_LIBRARIES(${NAME} ${GTK3_LIBRARIES})
endif(UNIX)

if(WIN32)
	set_property(TARGET ${NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "..\\..\\PlumbusTester")
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../ PROPERTY VS_STARTUP_PROJECT ${NAME})
endif(WIN32)


