cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
cmake_policy(VERSION 2.8)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_SUPPRESS_REGENERATION true)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (NOT MSVC AND NOT ${CMAKE_SYSTEM_NAME} MATCHES Android)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-format-truncation -Wno-unused-result")
endif ()

if(${CMAKE_SYSTEM_NAME} MATCHES Android)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-result")
endif()

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
include_directories(../third_party/SPIRV-Cross)

find_package(Vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIRS})

if(${CMAKE_SYSTEM_NAME} MATCHES "Android")
	set(native_app_glue ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)
	include_directories(${ANDROID_NDK}/sources/android/native_app_glue)
	find_library( android-log-lib log )
endif()

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

file(GLOB_RECURSE SOURCE src/*.cpp src/*.h)
source_group("src" FILES ${SOURCE})


if(${CMAKE_SYSTEM_NAME} MATCHES "Android")
    add_library(${NAME} SHARED src/PlumbusTester.cpp ${SOURCE} ${native_app_glue})
else()
    add_executable(${NAME} src/PlumbusTester.cpp ${SOURCE})
endif()

target_link_libraries(${NAME} PlumbusEngine)
link_libraries(PlumbusEngine)

message("CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}")
if (UNIX AND NOT APPLE AND NOT ${CMAKE_SYSTEM_NAME} MATCHES Android)
	FIND_PACKAGE(PkgConfig REQUIRED)
	PKG_CHECK_MODULES(GTK3 REQUIRED gtk+-3.0)
	INCLUDE_DIRECTORIES(${GTK3_INCLUDE_DIRS})
endif(UNIX AND NOT APPLE AND NOT ${CMAKE_SYSTEM_NAME} MATCHES Android)

if(MSVC)
	set_property(TARGET ${NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "..\\..\\PlumbusTester")
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../ PROPERTY VS_STARTUP_PROJECT ${NAME})
endif(MSVC)

if(${CMAKE_SYSTEM_NAME} MATCHES "Android")
	set(native_app_glue ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)
	include_directories(${ANDROID_NDK}/sources/android/native_app_glue)
	find_library( android-log-lib log )

	# From platform/android/CMakeLists.txt
	# Include Validation layers in the APK
	# platform-android can be replaced with any target, for example your native library
	set(layer-files ${ANDROID_NDK}/sources/third_party/vulkan/src/build-android/jniLibs/${ANDROID_ABI})
	file(GLOB layers ${layer-files}/*.so)
	foreach(layer ${layers})
		file(MAKE_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
		add_custom_command(TARGET ${NAME} POST_BUILD
				COMMAND ${CMAKE_COMMAND} -E copy ${layer} ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
	endforeach()
endif()

add_definitions(-DDLL_EXPORTS)
add_definitions(-D_REENTRANT)


