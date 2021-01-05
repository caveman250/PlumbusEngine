#pragma once
#include <string>

namespace plumbus
{
    class Platform
    {
    public:
        static std::string GetTextureDirPath() { return "textures/"; }
        static std::string GetAssetsPath() { return ""; }
        static std::string GetTextureExtension() { return ".astc"; }
    };
}
