#include "plumbus.h"
#include "ParserBase.h"

namespace plumbus::vk::shaders
{
    Node* ParserBase::Parse(const std::vector<std::unique_ptr<Token>>& tokens, int& currIndex)
    {
        for (currIndex; currIndex < tokens.size(); ++currIndex)
        {
            if (m_Finished || m_Error)
            {
                break;
            }

            switch (tokens[currIndex]->m_Type)
            {
                case TokenType::Comment:
                {
                    CommentToken& token = static_cast<CommentToken&>(*tokens[currIndex]);
                    HandleCommentToken(token);
                    continue;
                }
                case TokenType::Identifier:
                {
                    IdentifierToken& token = static_cast<IdentifierToken&>(*tokens[currIndex]);
                    HandleIdentifierToken(token);
                    continue;
                }
                case TokenType::Newline:
                {
                    NewlineToken& token = static_cast<NewlineToken&>(*tokens[currIndex]);
                    HandleNewlineToken(token);
                    continue;
                }
                case TokenType::Space:
                {
                    SpaceToken& token = static_cast<SpaceToken&>(*tokens[currIndex]);
                    HandleSpaceToken(token);
                    continue;
                }
                case TokenType::Tab:
                {
                    TabToken& token = static_cast<TabToken&>(*tokens[currIndex]);
                    HandleTabToken(token);
                    continue;
                }
                case TokenType::Punct:
                {
                    PunctToken& token = static_cast<PunctToken&>(*tokens[currIndex]);
                    HandlePunctToken(token);
                    continue;
                }
                case TokenType::Operator:
                {
                    OperatorToken& token = static_cast<OperatorToken&>(*tokens[currIndex]);
                    HandleOperatorToken(token);
                    continue;
                }
                case TokenType::Number:
                {
                    NumberToken& token = static_cast<NumberToken&>(*tokens[currIndex]);
                    HandleNumberToken(token);
                    continue;
                }
            }
        }

        if (m_Finished)
        {
            return ValidateAndFinalise(tokens[currIndex - 1]->m_LineNumber, tokens[currIndex - 1]->m_Position);
        }
        else
        {
            Log::Error("Shaders: Unexpected end of file");
            return nullptr;
        }
    }

    void ParserBase::LogIdentifierError(IdentifierToken& token)
    {
        Log::Error("Shaders: unexpected token '%s', Line:%i:%i", token.contents.c_str(), token.m_LineNumber, token.m_Position);
        m_Error = true;
    }

    void ParserBase::LogNewlineError(NewlineToken& token)
    {
        Log::Error("Shaders: unexpected newline, Line:%i:%i", token.m_LineNumber, token.m_Position);
        m_Error = true;
    }

    void ParserBase::LogPunctError(PunctToken& token)
    {
        Log::Error("Shaders: unexpected token '%c', Line:%i:%i", token.token, token.m_LineNumber, token.m_Position);
        m_Error = true;
    }

    void ParserBase::LogOperatorError(OperatorToken& token)
    {
        Log::Error("Shaders: unexpected token '%c', Line:%i:%i", token.op, token.m_LineNumber, token.m_Position);
        m_Error = true;
    }

    void ParserBase::LogNumberError(NumberToken& token)
    {
        if(token.hasDecimal)
        {
            Log::Error("Shaders: unexpected token '%f', Line:%i:%i", token.contents, token.m_LineNumber, token.m_Position);
        }
        else
        {
            Log::Error("Shaders: unexpected token '%i', Line:%i:%i", token.contents, token.m_LineNumber, token.m_Position);
        }

        m_Error = true;
    }
}