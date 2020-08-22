#include "plumbus.h"
#include "ShaderParser.h"

namespace plumbus::vk::shaders
{
    ShaderParser::ShaderParser()
        : m_ShaderSegment()
    {

    }

    void ShaderParser::TryStartNewNode(std::vector<std::unique_ptr<Token>>& tokens, int& currIndex)
    {
        std::unique_ptr<Token>& token = tokens[currIndex];
        switch (token->m_Type)
        {
            case TokenType::Identifier:
            {
                IdentifierToken& identifierToken = static_cast<IdentifierToken&>(*token);
                if (identifierToken.contents == "layout")
                {
                    if (AttributeBuilder::IsAttributeLine(tokens, currIndex))
                    {
                        AttributeBuilder builder;
                        if (Node* attribute = builder.Build(tokens, currIndex))
                        {
                            m_ShaderSegment.AddAttribute(static_cast<Attribute*>(attribute));
                        }
                    }
                }
                break;
            }
            case TokenType::Punct:
            default:
            {
                return;
            }
        }
    }

    void ShaderParser::Parse(std::vector<std::unique_ptr<Token>>& tokens)
    {
        for (int i = 0; i < tokens.size(); ++i)
        {
            TryStartNewNode(tokens, i);
        }
    }

}
