#include "platform/Platform.h"

namespace plumbus
{
    std::string Platform::GetTextureExtension()
    {
        return ".astc";
    }

    std::string Platform::GetTextureDirPath()
    {
        return "textures/";
    }

    std::string Platform::GetAssetsPath()
    {
        return "";
    }
}
