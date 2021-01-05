cmake_minimum_required(VERSION 3.18 FATAL_ERROR)
cmake_policy(VERSION 3.18)

#### OUTPUT DIR ####
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/bin/Debug")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin/Release")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DISTRIBUTION "${CMAKE_SOURCE_DIR}/bin/Distribution")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/bin/Debug")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin/Release")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DISTRIBUTION "${CMAKE_SOURCE_DIR}/bin/Distribution")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/bin/Debug")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin/Release")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DISTRIBUTION "${CMAKE_SOURCE_DIR}/bin/Distribution")

set(NAME PlumbusTester)
project(${NAME})

# get target platform
    if (WIN32)
        set(PLATFORM Windows)
    elseif (UNIX)
        if(APPLE)
            set(PLATFORM Mac)
        elseif(${CMAKE_SYSTEM_NAME} MATCHES Android)
            set(PLATFORM Android)
        else()
            set(PLATFORM Linux)   
        endif()
    endif()

#### COMPILER OPTIONS ####
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)

    if (${PLATFORM} MATCHES Mac OR ${PLATFORM} MATCHES Linux)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-format-truncation -Wno-unused-result -rdynamic")
    elseif(${PLATFORM} MATCHES Android)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-result")
    endif()

    if (DEFINED CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} cache string force)
    else()
        set(CMAKE_BUILD_TYPE Debug cache string)
    endif()

#### INCLUDES ####
	# find the system vulkan
	find_package(Vulkan REQUIRED)
	include_directories(${Vulkan_INCLUDE_DIRS})
	include_directories(../Engine/third_party)
	include_directories(../Engine/third_party/glm)
	include_directories(../Engine/third_party/gli)
	include_directories(../Engine/third_party/assimp/include/)
	include_directories(../Engine/third_party/imgui/)
	include_directories(../Engine/third_party/glfw/include/)
	include_directories(../Engine/Native/src)

	include_directories(src)

	if(${PLATFORM} MATCHES Android)
		set(native_app_glue ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)
		include_directories(${ANDROID_NDK}/sources/android/native_app_glue)
	endif()

	if (${PLATFORM} MATCHES Linux)
		find_package(PkgConfig REQUIRED)
		pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
		include_directories(${GTK3_INCLUDE_DIRS})
	endif()

#### SOURCE FILES ####
	file(GLOB_RECURSE SOURCE src/*.cpp src/*.h)
	source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE})

	if(${PLATFORM} MATCHES Android)
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

#### OUTPUT FILE ####
	if(${PLATFORM} MATCHES Android)
		add_library(${NAME} SHARED src/PlumbusTester.cpp ${SOURCE} ${native_app_glue})
	else()
		add_executable(${NAME} src/PlumbusTester.cpp ${SOURCE})
	endif()

#### PROCOMPILED HEADER ####
	if(NOT ${PLATFORM} MATCHES Android)
		target_precompile_headers(${NAME} PUBLIC "../Engine/Native/src/plumbus.h")
	endif()

#### LINKING ####
	target_link_libraries(${NAME} PlumbusEngine)
	link_libraries(PlumbusEngine)
	if (${PLATFORM} MATCHES Android)
		find_library( android-log-lib log )
	endif()

#### PREPROCESSOR DEFINES ####
	if (${PLATFORM} MATCHES Windows)
		add_definitions(-DPL_PLATFORM_WINDOWS=1)
	elseif (${PLATFORM} MATCHES Mac)
		add_definitions(-DPL_PLATFORM_OSX=1)
	elseif(${PLATFORM} MATCHES Android)
		add_definitions(-DPL_PLATFORM_ANDROID=1)
	else (${PLATFORM} MATCHES Linux)
		add_definitions(-DPL_PLATFORM_LINUX=1)
	endif()

	add_definitions(-DDLL_EXPORTS)
	add_definitions(-D_REENTRANT)

#### EXTRA MSVC OPTIONS ####
	if(${PLATFORM} MATCHES Windows)
		set_property(TARGET ${NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "..\\..\\bin\\$<CONFIG>")
		set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../ PROPERTY VS_STARTUP_PROJECT ${NAME})
	endif()



