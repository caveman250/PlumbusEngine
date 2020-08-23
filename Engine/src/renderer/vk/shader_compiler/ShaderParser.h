#pragma once
#include <string>
#include <vector>
#include "Tokenizer.h"
#include "ShaderSegment.h"
#include "renderer/vk/shader_compiler/parsers/AttributeParser.h"
#include "renderer/vk/shader_compiler/parsers/UniformParser.h"

namespace plumbus::vk::shaders
{
    class ShaderParser
    {
    public:
        ShaderParser();
        bool Parse(std::vector<std::unique_ptr<Token>>& tokens);

    private:
        bool TryStartNewNode(std::vector<std::unique_ptr<Token>>& tokens, int& currIndex);

        ShaderSegment m_ShaderSegment;
    };
}
