#pragma once

#include <unordered_map>
#include <string>

namespace plumbus::vk::shaders
{
    //if you add to this make sure you update s_TypeMap & GetVariableString
    enum class VariableType
    {
        vec2,
        vec3,
        vec4,
        mat3,
        mat4,
        Invalid
    };

    enum class AttributeDirection
    {
        In,
        Out,
        Unset
    };

    class CommonTypes
    {
    public:
        static VariableType GetVariableType(std::string typeString);
        static std::string GetVariableString(VariableType type);
        static std::string GetDirectionString(AttributeDirection direction);

    private:
        static std::unordered_map<std::string, VariableType> s_TypeMap;
    };

    enum class UniformType
    {
        Sampler2D,
        Buffer,
        Invalid
    };

    enum class NodeType
    {
        SamplerUniform,
        BufferUniform,
        Attribute,
    };

    struct Node
    {
        explicit Node(NodeType type) : m_Type(type) {}
        virtual std::string ToGLSL() = 0;

        NodeType m_Type;
    };

    struct Sampler2DUniform : public Node
    {
        Sampler2DUniform(int binding, std::string name)
        : Node(NodeType::SamplerUniform)
        , m_Binding(binding)
        , m_BindingName(std::move(name))
        {

        }

        std::string ToGLSL() override
        {
            char buffer[1024];
            sprintf(buffer, "layout (binding = %i) uniform sampler2D %s;", m_Binding, m_BindingName.c_str());
            return std::string(buffer);
        }

        int m_Binding;
        std::string m_BindingName;
    };

    struct BufferUniform : public Node
    {
        BufferUniform(int binding,
                      std::string bindingName,
                      std::vector<std::pair<VariableType, std::string>> bufferValues,
                      std::string bufferName)
        : Node(NodeType::BufferUniform)
        , m_Binding(binding)
        , m_BindingName(std::move(bindingName))
        , m_BufferValues(std::move(bufferValues))
        , m_BufferName(std::move(bufferName))
        {}

        std::string ToGLSL() override
        {
            std::string ret = "layout (binding = ";
            ret += std::to_string(m_Binding);
            ret += ") uniform ";
            ret += m_BindingName;
            ret += "\n{\n";

            for (auto& [type, name] : m_BufferValues)
            {
                ret += "\t";
                ret += CommonTypes::GetVariableString(type);
                ret += " ";
                ret += name;
                ret += ";\n";
            }

            ret += "} ";
            ret += m_BufferName;
            ret += ";";

            return ret;
        }

        int m_Binding;
        std::string m_BindingName;
        std::vector<std::pair<VariableType, std::string>> m_BufferValues;
        std::string m_BufferName;
    };

    struct Attribute : public Node
    {
        Attribute(int location, AttributeDirection direction, std::string name, VariableType type)
        : Node(NodeType::Attribute)
        , m_Location(location)
        , m_Direction(direction)
        , m_Name(std::move(name))
        , m_Type(type)
        {

        }

        std::string ToGLSL() override
        {
            char buffer[1024];
            sprintf(buffer, "layout (location = %i) %s %s %s;", m_Location, CommonTypes::GetDirectionString(m_Direction).c_str(), CommonTypes::GetVariableString(m_Type).c_str(), m_Name.c_str());
            return std::string(buffer);
        }

        int m_Location;
        AttributeDirection m_Direction;
        std::string m_Name;
        VariableType m_Type;
    };
}