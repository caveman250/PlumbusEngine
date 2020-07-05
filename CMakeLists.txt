cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
cmake_policy(VERSION 2.8)

project(PlumbusEngine)

add_subdirectory(third_party/glfw)
add_subdirectory(third_party/assimp)
add_subdirectory(PlumbusTester)
add_subdirectory(Engine)
