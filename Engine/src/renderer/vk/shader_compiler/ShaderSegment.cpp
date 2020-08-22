#include "plumbus.h"
#include "ShaderSegment.h"

namespace plumbus::vk::shaders
{
    ShaderSegment::ShaderSegment()
     : m_Attributes()
     , m_Uniforms()
    {

    }

    ShaderSegment::~ShaderSegment()
    {
        for (Attribute* attribute : m_Attributes)
        {
            delete attribute;
        }

        m_Attributes.clear();

        for (Uniform* uniform : m_Uniforms)
        {
            delete uniform;
        }

        m_Uniforms.clear();
    }

    void ShaderSegment::AddAttribute(Attribute* attribute)
    {
        m_Attributes.push_back(attribute);
    }

    void ShaderSegment::AddUniform(Uniform* uniform)
    {
        m_Uniforms.push_back(uniform);
    }
}
