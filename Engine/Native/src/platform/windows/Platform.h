#pragma once
#include <string>

namespace plumbus
{
    class Platform
    {
    public:
        static std::string GetTextureDirPath() { return GetAssetsPath() + "textures/desktop/"; }
        static std::string GetAssetsPath() { return "../../PlumbusTester/assets/"; } //TODO
        static std::string GetTextureExtension() { return ".ktx"; }
    };
}
