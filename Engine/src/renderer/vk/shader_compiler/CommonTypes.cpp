//
// Created by ouchqt on 22/8/20.
//

#include "CommonTypes.h"

namespace plumbus::vk::shaders
{
    std::unordered_map<std::string, VariableType> CommonTypes::s_TypeMap =
            {
                    {"vec2", VariableType::vec2},
                    {"vec3", VariableType::vec3},
                    {"vec4", VariableType::vec4},
            };

    VariableType CommonTypes::GetVariableType(std::string typeString)
    {
        if(s_TypeMap.count(typeString) > 0)
        {
            return s_TypeMap[typeString];
        }
        else
        {
            return VariableType::Invalid;
        }
    }
}