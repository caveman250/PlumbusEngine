#pragma once
#include "renderer/base/Texture.h"

namespace mtl
{
    class Texture : public base::Texture
    {
    public:
        Texture();
        ~Texture();
        
        void LoadTexture(std::string fileName) override;
        void Cleanup() override;
        
        void CreateSampler();
        
        void* GetTexture();
        void* GetSamplerState();
        
    private:
        void* m_ObjCManager;
    };
}
