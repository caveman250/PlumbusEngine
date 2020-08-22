#include "plumbus.h"
#include "ShaderCompiler.h"

namespace plumbus::vk::shaders
{
    void ShaderCompiler::Compile(const char* shaderPath) 
    {
        std::ifstream stream;
        stream.open(shaderPath);

        std::vector<std::unique_ptr<Token>> tokens = m_Tokenizer.TokenizeStream(stream);

        for (std::unique_ptr<Token>& token : tokens)
        {
            //token->PrintContents();
        }

        m_Parser.Parse(tokens);
    }


}