#pragma once
#include <string>
#include <vector>
#include "Tokenizer.h"
#include "ShaderSegment.h"
#include "renderer/vk/shader_compiler/builders/AttributeBuilder.h"
#include "UniformBuilder.h"

namespace plumbus::vk::shaders
{
    class ShaderParser
    {
    public:
        ShaderParser();
        void Parse(std::vector<std::unique_ptr<Token>>& tokens);

    private:
        void TryStartNewNode(std::vector<std::unique_ptr<Token>>& tokens, int& currIndex);

        ShaderSegment m_ShaderSegment;
    };
}
