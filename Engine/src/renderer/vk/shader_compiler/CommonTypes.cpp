//
// Created by ouchqt on 22/8/20.
//

#include "plumbus.h"
#include "CommonTypes.h"

namespace plumbus::vk::shaders
{
    std::unordered_map<std::string, VariableType> CommonTypes::s_TypeMap =
            {
                    {"vec2", VariableType::vec2},
                    {"vec3", VariableType::vec3},
                    {"vec4", VariableType::vec4},
                    {"mat3", VariableType::mat3},
                    {"mat4", VariableType::mat4}
            };

    VariableType CommonTypes::GetVariableType(std::string typeString)
    {
        if (s_TypeMap.count(typeString) > 0)
        {
            return s_TypeMap[typeString];
        }
        else
        {
            return VariableType::Invalid;
        }
    }

    std::string CommonTypes::GetVariableString(VariableType type)
    {
        switch (type)
        {
            case VariableType::vec2:
                return "vec2";
            case VariableType::vec3:
                return "vec3";
            case VariableType::vec4:
                return "vec4";
            case VariableType::mat3:
                return "mat3";
            case VariableType::mat4:
                return "mat4";
            case VariableType::Invalid:
                return "Error-Type";
            default:
            {
                PL_ASSERT(false, "CommonTypes::GetVariableString: Unknown VariableType");
                return "";
            }
        }


    }

    std::string CommonTypes::GetDirectionString(AttributeDirection direction)
    {
        switch (direction)
        {
            case AttributeDirection::In:
                return "in";
            case AttributeDirection::Out:
                return "out";
            case AttributeDirection::Unset:
                return "Error-Type";
            default:
            {
                PL_ASSERT(false, "CommonTypes::GetDirectionString: Unknown AttributeDirection");
                return "";
            }
        }
    }
}