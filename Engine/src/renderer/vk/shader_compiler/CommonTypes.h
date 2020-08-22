#pragma once

#include <unordered_map>
#include <string>

namespace plumbus::vk::shaders
{
    enum class VariableType
    {
        vec2,
        vec3,
        vec4,
        Invalid
    };

    enum class AttributeDirection
    {
        In,
        Out,
        Unset
    };

    enum class UniformType
    {
        sampler2D,
        buffer,
    };

    enum class NodeType
    {
        Uniform,
        Attribute,
    };

    struct Node
    {
        Node(NodeType type) : m_Type(type) {}

        NodeType m_Type;
    };

    struct Uniform : public Node
    {
        Uniform() : Node(NodeType::Uniform) {}
        int binding;
        std::string name;
        UniformType type;
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

        int m_Location;
        AttributeDirection m_Direction;
        std::string m_Name;
        VariableType m_Type;
    };

    class CommonTypes
    {
    public:
        static VariableType GetVariableType(std::string typeString);

    private:
        static std::unordered_map<std::string, VariableType> s_TypeMap;
    };
}