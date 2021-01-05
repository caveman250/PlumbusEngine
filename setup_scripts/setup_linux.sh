cp linuxEnvFiles/c_cpp_properties.json ../.vscode/c_cpp_properties.json

mkdir ../bin
cd ../bin
mkdir Debug
cd Debug
cmake ../../

cd ../
mkdir Release
cd Release
cmake ../../

cd ../
mkdir Dist
cd Dist
cmake ../../
