mkdir vs_project
cd vs_project
cmake ^
-DASSIMP_BUILD_ASSIMP_TOOLS=false ^
-DASSIMP_BUILD_TESTS=false ^
-DASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT=false ^
-DASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT=false ^
-DASSIMP_BUILD_OBJ_IMPORTER=true ^
-DASSIMP_BUILD_COLLADA_IMPORTER=true ^
-DGLFW_BUILD_DOCS=false ^
-DGLFW_BUILD_EXAMPLES=false ^
-DGLFW_BUILD_TESTS=false ^
-DGLFW_INSTALL=false ^
-DSPIRV_CROSS_CLI=false ^
-DSPIRV_CROSS_ENABLE_CPP=false ^
-DSPIRV_CROSS_ENABLE_C_API=false ^
-DSPIRV_CROSS_ENABLE_GLSL=false ^
-DSPIRV_CROSS_ENABLE_HLSL=false ^
-DSPIRV_CROSS_ENABLE_MSL=false ^
-DSPIRV_CROSS_ENABLE_REFLECT=false ^
-DSPIRV_CROSS_ENABLE_TESTS=false ^
-DSPIRV_CROSS_ENABLE_UTIL=false ^
..