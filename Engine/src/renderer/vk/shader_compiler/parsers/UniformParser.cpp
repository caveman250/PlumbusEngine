
#include "plumbus.h"
#include "UniformParser.h"

namespace plumbus::vk::shaders
{
    UniformParser::UniformParser()
            : m_State(State::None), m_Location(-1), m_Type(UniformType::Invalid), m_Name()
    {

    }

    bool UniformParser::IsUniformLine(const std::vector<std::unique_ptr<Token>>& tokens, int currIndex)
    {
        bool hasOpenParenthesis = false;
        for (int i = currIndex + 1; i < tokens.size(); ++i)
        {
            if (tokens[i]->m_Type == TokenType::Newline)
            {
                //hit newline while trying to figure out token, error.
                return -1;
            }

            if (tokens[i]->m_Type == TokenType::Punct)
            {
                PunctToken& punctToken = static_cast<PunctToken&>(*tokens[i]);
                if (punctToken.token == '(')
                {
                    if (hasOpenParenthesis)
                    {
                        return -1;
                    }

                    hasOpenParenthesis = true;
                }
            }

            if (tokens[i]->m_Type == TokenType::Identifier)
            {
                if (!hasOpenParenthesis)
                {
                    return -1;
                }

                IdentifierToken& token = static_cast<IdentifierToken&>(*tokens[i]);
                if (token.contents == "binding")
                {
                    return true;
                }
                else
                {
                    return false;
                }
            }
        }

        return false;
    }

    Node* UniformParser::ValidateAndFinalise(int lineNumber, int position)
    {
        if (m_Error)
        {
            return nullptr;
        }

        if (m_Type == UniformType::Buffer)
        {
            if (m_Location != -1 &&
                m_Name.length() > 0 &&
                m_BufferName.length() > 0 &&
                !m_BufferValues.empty())
            {
                return new BufferUniform(m_Location, m_Name, m_BufferValues, m_BufferName);
            }
        }
        else if (m_Type == UniformType::Sampler2D)
        {
            if (m_Location != -1 &&
                m_Name.length() > 0)
            {
                return new Sampler2DUniform(m_Location, m_Name);
            }
        }

        Log::Error("Shaders: finalising uniform failed. Line: %i:%i", lineNumber, position);
        return nullptr;
    }

    void UniformParser::HandleCommentToken(CommentToken& token)
    {

    }

    void UniformParser::HandleIdentifierToken(IdentifierToken& token)
    {
        if (token.contents == "layout")
        {
            if (m_State != State::None)
            {
                LogIdentifierError(token);
                return;
            }
            m_State = State::FoundLayout;
            return;
        }
        else if (token.contents == "binding")
        {
            if (m_State != State::FoundOpenParentheses)
            {
                LogIdentifierError(token);
                return;
            }

            m_State = State::FoundLocationIdentifier;
            return;
        }
        else if (token.contents == "uniform")
        {
            if (m_State != State::FoundCloseParentheses)
            {
                LogIdentifierError(token);
                return;
            }

            m_State = State::FoundUniform;
            return;
        }
        else if (m_State == State::FoundUniform)
        {
            if (token.contents == "sampler2D")
            {
                m_Type = UniformType::Sampler2D;
                m_State = State::FoundType;
                return;
            }
            else
            {
                m_Type = UniformType::Buffer;
                m_Name = token.contents;
                m_State = State::FoundName;
                return;
            }
        }
        else if (m_State == State::FoundType)
        {
            m_Name = token.contents;
            m_State = State::FoundName;
            return;
        }
        else if (m_State == State::FoundOpenBrace || m_State == State::FoundVariableSemicolon)
        {
            VariableType type = CommonTypes::GetVariableType(token.contents);
            if (type != VariableType::Invalid)
            {
                std::pair<VariableType, std::string> pair = std::make_pair<VariableType, std::string>(std::move(type), std::string());
                m_BufferValues.push_back(pair);
                m_State = State::FoundVariableType;
                return;
            }
        }
        else if (m_State == State::FoundVariableType)
        {
            m_BufferValues.back().second = token.contents;
            m_State = State::FoundVariableName;
            return;
        }
        else if (m_State == State::FoundCloseBrace)
        {
            m_BufferName = token.contents;
            m_State = State::FoundBufferName;
            return;
        }

        LogIdentifierError(token);
    }

    void UniformParser::HandleNewlineToken(NewlineToken& token)
    {
        //do nothing
    }

    void UniformParser::HandleSpaceToken(SpaceToken& token)
    {
        // do nothing
    }

    void UniformParser::HandleTabToken(TabToken& token)
    {
        // do nothing
    }

    void UniformParser::HandlePunctToken(PunctToken& token)
    {
        if (token.token == '(')
        {
            if (m_State != State::FoundLayout)
            {
                LogPunctError(token);
                return;
            }

            m_State = State::FoundOpenParentheses;
            return;
        }
        else if (token.token == ')')
        {
            if (m_State != State::FoundLocationValue)
            {
                LogPunctError(token);
                return;
            }

            m_State = State::FoundCloseParentheses;
            return;
        }
        else if (token.token == ';')
        {
            if ((m_Type == UniformType::Buffer && m_State == State::FoundBufferName) ||
                (m_Type == UniformType::Sampler2D && m_State == State::FoundName))
            {
                m_Finished = true;
                return;
            }
            else if (m_State == State::FoundVariableName)
            {
                m_State = State::FoundVariableSemicolon;
                return;
            }
            else
            {
                LogPunctError(token);
                return;
            }
        }
        else if (token.token == '{')
        {
            if (m_State != State::FoundName || m_Type != UniformType::Buffer)
            {
                LogPunctError(token);
                return;
            }

            m_State = State::FoundOpenBrace;
            return;
        }
        else if (token.token == '}')
        {
            if (m_State != State::FoundVariableSemicolon || m_Type != UniformType::Buffer)
            {
                LogPunctError(token);
                return;
            }

            m_State = State::FoundCloseBrace;
            return;
        }
        else
        {
            LogPunctError(token);
        }
    }

    void UniformParser::HandleOperatorToken(OperatorToken& token)
    {
        if (m_State == State::FoundLocationIdentifier && token.op == '=')
        {
            m_State = State::FoundAssignment;
        }
        else
        {
            LogOperatorError(token);
            return;
        }
    }

    void UniformParser::HandleNumberToken(NumberToken& token)
    {
        if (m_State == State::FoundAssignment)
        {
            m_Location = (int)token.contents;
            m_State = State::FoundLocationValue;
        }
        else
        {
            LogNumberError(token);
            return;
        }
    }
}
