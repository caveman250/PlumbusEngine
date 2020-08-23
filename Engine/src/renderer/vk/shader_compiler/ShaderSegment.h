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
        void AddSamplerUniform(Sampler2DUniform* uniform);
        void AddBufferUniform(BufferUniform* uniform);

        std::string ToGLSL();

    private:
        std::vector<Sampler2DUniform*> m_SamplerUniforms;
        std::vector<BufferUniform*> m_BufferUniforms;
        std::vector<Attribute*> m_Attributes;
    };
}