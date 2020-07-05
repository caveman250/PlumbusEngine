cp linuxEnvFiles/c_cpp_properties.json .vscode/c_cpp_properties.json

mkdir bin
cd bin
mkdir Debug
cd Debug
cmake -DCMAKE_BUILD_TYPE=Debug -DASSIMP_BUILD_ASSIMP_TOOLS=false -DASSIMP_BUILD_TESTS=false -DASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT=true -DGLFW_BUILD_DOCS=false -DGLFW_BUILD_EXAMPLES=false -DGLFW_BUILD_TESTS=false -DGLFW_INSTALL=false ../../
cd ../
mkdir Release
cd Release
cmake -DCMAKE_BUILD_TYPE=Release -DASSIMP_BUILD_ASSIMP_TOOLS=false -DASSIMP_BUILD_TESTS=false -DASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT=true -DGLFW_BUILD_DOCS=false -DGLFW_BUILD_EXAMPLES=false -DGLFW_BUILD_TESTS=false -DGLFW_INSTALL=false ../../