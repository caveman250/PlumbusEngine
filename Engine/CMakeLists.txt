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
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-format-truncation -Wno-unused-result -rdynamic")
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

#MONO TODO this should be smarter
if (UNIX AND NOT APPLE AND NOT ${CMAKE_SYSTEM_NAME} MATCHES Android )
    include_directories("/usr/include/mono-2.0")
endif()
if(WIN32)
    include_directories("C:\\Program Files\\Mono\\include\\mono-2.0")
endif()


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

link_directories(${CMAKE_SOURCE_DIR}/lib)

file(GLOB_RECURSE SOURCE src/*.cpp src/*.h)
source_group("src" FILES ${SOURCE})

file(GLOB IMGUIIMGUI ../third_party/imgui/*.cpp ../third_party/imgui/*.h)
source_group("third_party\\imgui" FILES ${IMGUIIMGUI})

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

if (UNIX AND NOT APPLE AND NOT ${CMAKE_SYSTEM_NAME} MATCHES Android )
    target_link_libraries(${NAME} "/usr/lib/libmono-2.0.a")
    target_link_libraries(${NAME} "rt")
    target_link_libraries(${NAME} "pthread")
    target_link_libraries(${NAME} "m")
    target_link_libraries(${NAME} "dl")
endif()
if(WIN32)
    target_link_libraries(${NAME} "C:\\Program Files\\Mono\\lib\\mono-2.0-sgen.lib")
endif()

if (UNIX AND NOT APPLE AND NOT ${CMAKE_SYSTEM_NAME} MATCHES Android )
    FIND_PACKAGE(PkgConfig REQUIRED)
    PKG_CHECK_MODULES(GTK3 REQUIRED gtk+-3.0)
    INCLUDE_DIRECTORIES(${GTK3_INCLUDE_DIRS})
    LINK_DIRECTORIES(${GTK3_LIBRARY_DIRS})
    ADD_DEFINITIONS(${GTK3_CFLAGS_OTHER})
    TARGET_LINK_LIBRARIES(${NAME} ${GTK3_LIBRARIES})
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

add_definitions(-DDLL_EXPORTS)
add_definitions(-D_REENTRANT)

message(${CMAKE_SYSTEM_NAME} )
if (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Android")
    target_precompile_headers(${NAME} PUBLIC "src/plumbus.h")
endif (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Android")

