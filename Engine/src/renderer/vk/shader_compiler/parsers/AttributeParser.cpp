#include "plumbus.h"
#include "AttributeParser.h"

namespace plumbus::vk::shaders
{
    AttributeParser::AttributeParser()
            : m_State(State::None)
            , m_Location(-1)
            , m_Type(VariableType::Invalid)
            , m_Name()
            , m_Direction(AttributeDirection::Unset)
    {

    }

    bool AttributeParser::IsAttributeLine(const std::vector<std::unique_ptr<Token>>& tokens, int currIndex)
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
                if (token.contents == "location")
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

    Node* AttributeParser::ValidateAndFinalise(int lineNumber, int position)
    {
        if (m_Type != VariableType::Invalid &&
            m_Location > -1 &&
            m_Direction != AttributeDirection::Unset &&
            !m_Name.empty())
        {
            return new Attribute(m_Location, m_Direction, m_Name, m_Type);
        }

        Log::Error("Shaders: hit newline while parsing attribute, %i:%i", lineNumber, position);
        return nullptr;
    }

    void AttributeParser::HandleCommentToken(CommentToken& token)
    {

    }

    void AttributeParser::HandleIdentifierToken(IdentifierToken& token)
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
        else if (token.contents == "location")
        {
            if (m_State != State::FoundOpenParentheses)
            {
                LogIdentifierError(token);
                return;
            }

            m_State = State::FoundLocationIdentifier;
            return;
        }
        else if (token.contents == "in")
        {
            if (m_State != State::FoundCloseParentheses)
            {
                LogIdentifierError(token);
                return;
            }

            m_Direction = AttributeDirection::In;
            m_State = State::FoundInOut;
            return;
        }
        else if (token.contents == "out")
        {
            if (m_State != State::FoundCloseParentheses)
            {
                LogIdentifierError(token);
                return;
            }

            m_Direction = AttributeDirection::Out;
            m_State = State::FoundInOut;
            return;
        }
        else if (m_State == State::FoundType)
        {
            m_Name = token.contents;
            m_State = State::FoundName;
            return;
        }
        else
        {
            VariableType type = CommonTypes::GetVariableType(token.contents);
            if (type != VariableType::Invalid)
            {
                m_Type = type;
                m_State = State::FoundType;
                return;
            }
        }

        LogIdentifierError(token);
    }

    void AttributeParser::HandleNewlineToken(NewlineToken& token)
    {
        //do nothing
    }

    void AttributeParser::HandleSpaceToken(SpaceToken& token)
    {
        // do nothing
    }

    void AttributeParser::HandleTabToken(TabToken& token)
    {
        // do nothing
    }

    void AttributeParser::HandlePunctToken(PunctToken& token)
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
            m_Finished = true;
        }
    }

    void AttributeParser::HandleOperatorToken(OperatorToken& token)
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

    void AttributeParser::HandleNumberToken(NumberToken& token)
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
