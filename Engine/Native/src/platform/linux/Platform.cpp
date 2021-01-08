#include "platform/Platform.h"

namespace plumbus
{
    std::string plumbus::Platform::GetTextureDirPath()
    {
        return GetAssetsPath() + "textures/desktop/";
    }

    std::string Platform::GetAssetsPath()
    {
        return "../../PlumbusTester/assets/";
    }

    std::string Platform::GetTextureExtension()
    {
        return ".ktx";
    }
};