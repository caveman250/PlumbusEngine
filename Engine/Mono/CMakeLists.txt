project(PlumbusEngineMono)

add_custom_target(PlumbusEngineMono WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} COMMAND dotnet build PlumbusMono.sln)