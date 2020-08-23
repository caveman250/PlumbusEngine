#include "plumbus.h"
#include "ShaderSegment.h"

namespace plumbus::vk::shaders
{
    ShaderSegment::ShaderSegment()
     : m_Attributes()
     , m_SamplerUniforms()
     , m_BufferUniforms()
    {

    }

    ShaderSegment::~ShaderSegment()
    {
        for (Attribute* attribute : m_Attributes)
        {
            delete attribute;
        }

        m_Attributes.clear();

        for (BufferUniform* uniform : m_BufferUniforms)
        {
            delete uniform;
        }

        m_BufferUniforms.clear();

        for (Sampler2DUniform* uniform : m_SamplerUniforms)
        {
            delete uniform;
        }

        m_SamplerUniforms.clear();
    }

    void ShaderSegment::AddAttribute(Attribute* attribute)
    {
        m_Attributes.push_back(attribute);
    }

    void ShaderSegment::AddBufferUniform(BufferUniform* uniform)
    {
        m_BufferUniforms.push_back(uniform);
    }

    void ShaderSegment::AddSamplerUniform(Sampler2DUniform* uniform)
    {
        m_SamplerUniforms.push_back(uniform);
    }

    std::string ShaderSegment::ToGLSL()
    {
        std::string glsl;
        for (Attribute* attribute : m_Attributes)
        {
            glsl += attribute->ToGLSL();
            glsl += "\n";
        }

        for (Sampler2DUniform* sampler : m_SamplerUniforms)
        {
            glsl += sampler->ToGLSL();
            glsl += "\n";
        }

        for (BufferUniform* buffer : m_BufferUniforms)
        {
            glsl += buffer->ToGLSL();
            glsl += "\n";
        }

        return glsl;
    }
}
