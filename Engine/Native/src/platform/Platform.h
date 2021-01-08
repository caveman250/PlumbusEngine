#pragma once
#include <string>

namespace plumbus
{
    class Platform
    {
    public:
        static std::string GetTextureDirPath();
        static std::string GetAssetsPath();
        static std::string GetTextureExtension();
    };
}
