#include "plumbus.h"
#include "ShaderParser.h"

namespace plumbus::vk::shaders
{
    ShaderParser::ShaderParser()
            : m_ShaderSegment()
    {

    }

    bool ShaderParser::TryStartNewNode(std::vector<std::unique_ptr<Token>>& tokens, int& currIndex)
    {
        std::unique_ptr<Token>& token = tokens[currIndex];
        switch (token->m_Type)
        {
            case TokenType::Identifier:
            {
                IdentifierToken& identifierToken = static_cast<IdentifierToken&>(*token);
                if (identifierToken.contents == "layout")
                {
                    if (AttributeParser::IsAttributeLine(tokens, currIndex))
                    {
                        AttributeParser builder;
                        if (Node* attribute = builder.Parse(tokens, currIndex))
                        {
                            m_ShaderSegment.AddAttribute(static_cast<Attribute*>(attribute));
                        }
                        else
                        {
                            return false;
                        }
                    }
                    else if (UniformParser::IsUniformLine(tokens, currIndex))
                    {
                        UniformParser builder;
                        if (Node* uniform = builder.Parse(tokens, currIndex))
                        {
                            switch (uniform->m_Type)
                            {
                                case NodeType::SamplerUniform:
                                    m_ShaderSegment.AddSamplerUniform(static_cast<Sampler2DUniform*>(uniform));
                                    break;
                                case NodeType::BufferUniform:
                                    m_ShaderSegment.AddBufferUniform(static_cast<BufferUniform*>(uniform));
                                    break;
                                default:
                                    Log::Error("Unhandled uniform type");
                                    return false;
                            }

                        }
                        else
                        {

                            return false;
                        }
                    }
                }
                break;
            }
            default:
            {
                return true;
            }
        }

        return true;
    }

    bool ShaderParser::Parse(std::vector<std::unique_ptr<Token>>& tokens)
    {
        bool didError = false;
        for (int i = 0; i < tokens.size(); ++i)
        {
            didError |= !TryStartNewNode(tokens, i);
        }

        if (!didError)
        {
            Log::Info("Compiled Shader Code:\n%s", m_ShaderSegment.ToGLSL().c_str());
        }

        return !didError;
    }

}
