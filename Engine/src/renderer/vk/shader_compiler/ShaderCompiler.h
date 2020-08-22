#include "Tokenizer.h"
#include "ShaderParser.h"

namespace plumbus::vk::shaders
{
    class ShaderCompiler
    {
    public:
        void Compile(const char* shaderPath);
    private:

        Tokenizer m_Tokenizer;
        ShaderParser m_Parser;
    };
}