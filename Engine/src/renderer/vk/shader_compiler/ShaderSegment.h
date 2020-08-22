#pragma once
#include <vector>
#include <string>
#include "CommonTypes.h"

namespace plumbus::vk::shaders
{
    class ShaderSegment
    {
    public:
        ShaderSegment();
        ~ShaderSegment();

        void AddAttribute(Attribute* attribute);
        void AddUniform(Uniform* uniform);

    private:
        std::vector<Uniform*> m_Uniforms;
        std::vector<Attribute*> m_Attributes;
    };
}