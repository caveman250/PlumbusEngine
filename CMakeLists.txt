cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
cmake_policy(VERSION 3.16)

project(PlumbusNative)

#### Get Platform ####
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

#### Third Party Options ####
option(ASSIMP_BUILD_ASSIMP_TOOLS "" FALSE)
option(ASSIMP_BUILD_TESTS "" FALSE)
option(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT "" FALSE)
option(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT "" FALSE)
option(ASSIMP_BUILD_OBJ_IMPORTER "" TRUE)
option(ASSIMP_BUILD_COLLADA_IMPORTER "" TRUE)
option(GLFW_BUILD_DOCS "" FALSE)
option(GLFW_BUILD_EXAMPLES "" FALSE)
option(GLFW_BUILD_TESTS "" FALSE)
option(GLFW_INSTALL "" FALSE)
option(SPIRV_CROSS_CLI "" FALSE)
option(SPIRV_CROSS_ENABLE_CPP "" FALSE)
option(SPIRV_CROSS_ENABLE_C_API "" FALSE)
option(SPIRV_CROSS_ENABLE_GLSL "" FALSE)
option(SPIRV_CROSS_ENABLE_HLSL "" FALSE)
option(SPIRV_CROSS_ENABLE_MSL "" FALSE)
option(SPIRV_CROSS_ENABLE_REFLECT "" FALSE)
option(SPIRV_CROSS_ENABLE_TESTS "" FALSE)
option(SPIRV_CROSS_STATIC "" TRUE)
option(ENABLE_HLSL "" FALSE)
option(ENABLE_OPT "" FALSE)
option(ENABLE_SPVREMAPPER "" FALSE)
option(ENABLE_CTEST "" FALSE)
option(ASSIMP_INSTALL "" FALSE)
option(ASSIMP_INSTALL_PDB "" FALSE)
option(ASSIMP_INJECT_DEBUG_POSTFIX "" FALSE)
option(BUILD_TESTING "" FALSE)
option(ENABLE_GLSLANG_BINARIES "" FALSE)
option(BUILD_SHARED_LIBS "" FALSE)
set(CMAKE_DEBUG_POSTFIX "")
#### allow subdirectories to be placed inside visual studio filters ####
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4100 /wd4190")
endif (MSVC)

if(WIN32 AND NOT MSVC)
    #fix for assimp under mingw
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unknown-pragmas")
endif(WIN32 AND NOT MSVC)

### Setup Build Variants Debug/Release/Distribution ####
if(CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_CONFIGURATION_TYPES Debug Release Distribution)
    set(CMAKE_CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES}" CACHE STRING "Reset the configurations to what we need" FORCE)

    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELWITHDEBINFO}")
    set(CMAKE_LINKER_FLAGS_RELEASE "${CMAKE_LINKER_FLAGS_RELWITHDEBINFO}")
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO}")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO}")

    set(CMAKE_CXX_FLAGS_DISTRIBUTION "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
    set(CMAKE_C_FLAGS_DISTRIBUTION "${CMAKE_C_FLAGS_RELWITHDEBINFO}")
    set(CMAKE_LINKER_FLAGS_DISTRIBUTION "${CMAKE_LINKER_FLAGS_RELWITHDEBINFO}")
    set(CMAKE_SHARED_LINKER_FLAGS_DISTRIBUTION "${CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO}")
    set(CMAKE_EXE_LINKER_FLAGS_DISTRIBUTION "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO}")

    set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS
        $<$<CONFIG:Debug>:PL_DEBUG=1>
        $<$<CONFIG:Release>:PL_RELEASE=1>
        $<$<CONFIG:Distribution>:PL_DIST=1>)
else()
    if(CMAKE_BUILD_TYPE MATCHES Debug)
        add_definitions(-DPL_DEBUG=1)
    endif()

    if(CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)
        add_definitions(-DPL_RELEASE=1)
    endif()

    if(CMAKE_BUILD_TYPE MATCHES Release)
        add_definitions(-DPL_DIST=1)
    endif()

    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELWITHDEBINFO}")
    set(CMAKE_LINKER_FLAGS_RELEASE "${CMAKE_LINKER_FLAGS_RELWITHDEBINFO}")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO}")
endif()

#### Projects to build ####
# plumbus projects
add_subdirectory(PlumbusTester)
add_subdirectory(Engine)

# third party
define_property(
    TARGET
    PROPERTY FOLDER
    INHERITED
    BRIEF_DOCS "Set the folder name."
    FULL_DOCS  "Use to organize targets in an IDE."
)

function(add_subdirectory_with_folder _folder_name _folder)
    add_subdirectory(${_folder} ${ARGN})
    set_property(DIRECTORY "${_folder}" PROPERTY FOLDER "${_folder_name}")
endfunction()

# assimp
add_subdirectory_with_folder("third_party/assimp" Engine/third_party/assimp)

#SPIRV-Cross
add_subdirectory_with_folder("third_party/SPIRV" Engine/third_party/SPIRV-Cross)

# glfw
if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Android" )
    add_subdirectory("Engine/third_party/glfw")
    set_target_properties(glfw PROPERTIES FOLDER "third_party/glfw")
endif(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Android" )

# glslang
add_subdirectory("Engine/third_party/glslang")
set_target_properties(GenericCodeGen PROPERTIES FOLDER "third_party/glslang")
set_target_properties(glslang PROPERTIES FOLDER "third_party/glslang")
set_target_properties(glslang-build-info PROPERTIES FOLDER "third_party/glslang")
set_target_properties(MachineIndependent PROPERTIES FOLDER "third_party/glslang")
set_target_properties(OGLCompiler PROPERTIES FOLDER "third_party/glslang")
set_target_properties(OSDependent PROPERTIES FOLDER "third_party/glslang")
set_target_properties(SPIRV PROPERTIES FOLDER "third_party/glslang")
