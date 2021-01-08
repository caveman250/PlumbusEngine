cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
cmake_policy(VERSION 3.16)

add_subdirectory("Mono")

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

#### OUTPUT DIR ####
    if(${PLATFORM} MATCHES Windows OR ${PLATFORM} MATCHES Mac)
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/bin/Debug")
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin/Release")
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DISTRIBUTION "${CMAKE_SOURCE_DIR}/bin/Distribution")
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/bin/Debug")
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin/Release")
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DISTRIBUTION "${CMAKE_SOURCE_DIR}/bin/Distribution")
    else()
        if(CMAKE_BUILD_TYPE MATCHES Debug)
            set(OUTDIR ${CMAKE_SOURCE_DIR}/bin/Debug)
        elseif(CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)
            set(OUTDIR ${CMAKE_SOURCE_DIR}/bin/Release)
        elseif(CMAKE_BUILD_TYPE MATCHES Release)
            set(OUTDIR ${CMAKE_SOURCE_DIR}/bin/Distribution)
        endif()
        set(LIBRARY_OUTPUT_PATH ${OUTDIR})
    endif()

set(NAME PlumbusEngine)
project(${NAME})

#### COMPILER OPTIONS ####
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)

    if (${PLATFORM} MATCHES Mac OR ${PLATFORM} MATCHES Linux)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-format-truncation -Wno-unused-result -rdynamic")
    elseif(${PLATFORM} MATCHES Android)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-result")
    endif()

    if(DEFINED CMAKE_BUILD_TYPE)
        SET(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "" FORCE)
    else()
        SET(CMAKE_BUILD_TYPE Debug CACHE STRING "")
    endif()

#### INCLUDES ####
    # find the system vulkan
    find_package(Vulkan REQUIRED)
    include_directories(${Vulkan_INCLUDE_DIRS})

    # find the system mono. TODO: this should be smarter.
    if (${PLATFORM} MATCHES Linux)
        include_directories("/usr/include/mono-2.0")
    elseif(${PLATFORM} MATCHES Windows)
        include_directories("C:\\Program Files\\Mono\\include\\mono-2.0")
    elseif(${PLATFORM} MATCHES Mac)
        include_directories("/Library/Frameworks/Mono.framework/Versions/Current/include/mono-2.0")
    else()
        message(FATAL_ERROR "Mono path not defined for " ${PLATFORM})
    endif()

    # find gtk-3 (used for assert windows on linux)
    if (${PLATFORM} MATCHES Linux)
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
        include_directories(${GTK3_INCLUDE_DIRS})
    endif()

    # include third party folder
    include_directories(third_party)
    include_directories(third_party/glm)
    include_directories(third_party/gli)
    include_directories(third_party/assimp/include/)
    include_directories(third_party/glfw/include/)
    include_directories(third_party/glslang/)
    include_directories(third_party/assimp/include/)

    # include Engine headers
    include_directories(Native/src)
    include_directories(Native/src/components)

    # android native app glue
    if(${PLATFORM} MATCHES Android)
        include_directories(${ANDROID_NDK}/sources/android/native_app_glue)
    endif()

#### SOURCE FILES ####
    file(GLOB_RECURSE SOURCE Native/src/*.cpp Native/src/*.h)
    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE})

    file(GLOB IMGUI third_party/imgui/*.cpp third_party/imgui/*.h)
    source_group("src/imgui" FILES ${IMGUI})

#### OUTPUT FILES ####
    add_library(${NAME} STATIC ${SOURCE} ${IMGUI})

#### DEPENDENCIES ####
add_dependencies(${NAME} PlumbusEngineMono)

#### PROCOMPILED HEADER ####
    if(NOT ${PLATFORM} MATCHES Android)
        target_precompile_headers(${NAME} PUBLIC "Native/src/plumbus.h")
    endif()

#### LINKING ####
    target_link_libraries(${NAME} ${Vulkan_LIBRARIES})

    if(NOT ${PLATFORM} MATCHES Android)
        target_link_libraries(${NAME} glfw ${GLFW_LIBRARIES})
    endif()

    target_link_libraries(${NAME} assimp)
    link_libraries(${ASSIMP_LIBRARIES})

    target_link_libraries(${NAME} SPIRV)
    target_link_libraries(${NAME} spirv-cross-core)
    target_link_libraries(${NAME} glslang)

    if(${PLATFORM} MATCHES Android)
        target_link_libraries(${NAME} ${android-log-lib} android)
    endif()

    if (${PLATFORM} MATCHES Linux)
        target_link_libraries(${NAME} "/usr/lib/libmono-2.0.a")
        target_link_libraries(${NAME} "rt")
        target_link_libraries(${NAME} "pthread")
        target_link_libraries(${NAME} "m")
        target_link_libraries(${NAME} "dl")
    elseif(${PLATFORM} MATCHES Windows)
        target_link_libraries(${NAME} "C:\\Program Files\\Mono\\lib\\mono-2.0-sgen.lib")
    elseif(${PLATFORM} MATCHES Mac)
        target_link_libraries(${NAME} "/Library/Frameworks/Mono.framework/Versions/Current/lib/libmono-2.0.a")
        target_link_libraries(${NAME} "iconv")
    else()
        message(FATAL_ERROR "mono library paths not set for " ${PLATFORM})
    endif()

    if (${PLATFORM} MATCHES Linux)
        link_directories(${GTK3_LIBRARY_DIRS})
        target_link_libraries(${NAME} ${GTK3_LIBRARIES})
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

    if (${PLATFORM} MATCHES Linux)
        add_definitions(${GTK3_CFLAGS_OTHER})
    endif()