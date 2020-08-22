#include "plumbus.h"
#include "AttributeBuilder.h"

namespace plumbus::vk::shaders
{
    AttributeBuilder::AttributeBuilder()
            : m_State(State::None)
            , m_Location(-1)
            , m_Type(VariableType::Invalid)
            , m_Name()
            , m_Direction(AttributeDirection::Unset)
    {

    }

    bool AttributeBuilder::IsAttributeLine(const std::vector<std::unique_ptr<Token>>& tokens, int currIndex)
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

                IdentifierToken& idToken = static_cast<IdentifierToken&>(*tokens[i]);
                if (idToken.contents == "location")
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

    Attribute* AttributeBuilder::BuildAttribute(const std::vector<std::unique_ptr<Token>>& tokens, int& currIndex)
    {
        for (currIndex; currIndex < tokens.size(); ++currIndex)
        {
            if (tokens[currIndex]->m_Type == TokenType::Punct)
            {
                PunctToken& punctToken = static_cast<PunctToken&>(*tokens[currIndex]);
                if (punctToken.token == '(')
                {
                    if (m_State != State::Foundlayout)
                    {
                        Log::Error("Shaders: unexpected token '(', %i:%i", punctToken.m_LineNumber,
                                   punctToken.m_Position);
                        return nullptr;
                    }

                    m_State = State::FoundOpenParenthisis;
                    continue;
                }
                else if (punctToken.token == ')')
                {
                    if (m_State != State::FoundLocationValue)
                    {
                        Log::Error("Shaders: unexpected token ')', %i:%i", punctToken.m_LineNumber,
                                   punctToken.m_Position);
                        return nullptr;
                    }

                    m_State = State::FoundCloseParenthisis;
                    continue;
                }
                else if (punctToken.token == ';')
                {
                    return ValidateAndBuild(punctToken.m_LineNumber, punctToken.m_Position);
                }
            }

            if (tokens[currIndex]->m_Type == TokenType::Identifier)
            {
                IdentifierToken& idToken = static_cast<IdentifierToken&>(*tokens[currIndex]);

                if (idToken.contents == "layout")
                {
                    if (m_State != State::None)
                    {
                        Log::Error("Shaders: unexpected identifier '%s', %i:%i", idToken.contents.c_str(),
                                   idToken.m_LineNumber, idToken.m_Position);
                        return nullptr;
                    }
                    m_State = State::Foundlayout;
                    continue;
                }
                else if (idToken.contents == "location")
                {
                    if (m_State != State::FoundOpenParenthisis)
                    {
                        Log::Error("Shaders: unexpected identifier '%s', %i:%i", idToken.contents.c_str(),
                                   idToken.m_LineNumber, idToken.m_Position);
                        return nullptr;
                    }

                    m_State = State::FoundLocationIdentifier;
                    continue;
                }
                else if (idToken.contents == "in")
                {
                    if (m_State != State::FoundCloseParenthisis)
                    {
                        Log::Error("Shaders: unexpected identifier '%s', %i:%i", idToken.contents.c_str(),
                                   idToken.m_LineNumber, idToken.m_Position);
                        return nullptr;
                    }

                    m_Direction = AttributeDirection::In;
                    m_State = State::FoundInOut;
                    continue;
                }
                else if (idToken.contents == "out")
                {
                    if (m_State != State::FoundCloseParenthisis)
                    {
                        Log::Error("Shaders: unexpected identifier '%s', %i:%i", idToken.contents.c_str(),
                                   idToken.m_LineNumber, idToken.m_Position);
                        return nullptr;
                    }

                    m_Direction = AttributeDirection::Out;
                    m_State = State::FoundInOut;
                    continue;
                }
                else if (m_State == State::FoundType)
                {
                    m_Name = idToken.contents;
                    m_State = State::FoundName;
                    continue;
                }
                else
                {
                    VariableType type = CommonTypes::GetVariableType(idToken.contents);
                    if (type != VariableType::Invalid)
                    {
                        m_Type = type;
                        m_State = State::FoundType;
                        continue;
                    }
                }

                Log::Error("Shaders: unexpected identifier '%s', %i:%i", idToken.contents.c_str(), idToken.m_LineNumber,
                           idToken.m_Position);
                return nullptr;
            }

            if (tokens[currIndex]->m_Type == TokenType::Operator)
            {
                OperatorToken& opToken = static_cast<OperatorToken&>(*tokens[currIndex]);
                if (m_State == State::FoundLocationIdentifier && opToken.op == '=')
                {
                    m_State = State::FoundAssignment;
                }
                else
                {
                    Log::Error("Shaders: unexpected operator '%c', %i:%i", opToken.op, opToken.m_LineNumber,
                               opToken.m_Position);
                    return nullptr;
                }
            }

            if (tokens[currIndex]->m_Type == TokenType::Number)
            {
                NumberToken& numberToken = static_cast<NumberToken&>(*tokens[currIndex]);
                if (m_State == State::FoundAssignment)
                {
                    m_Location = (int) numberToken.contents;
                    m_State = State::FoundLocationValue;
                }
                else
                {
                    if (numberToken.hasDecimal)
                    {
                        Log::Error("Shaders: unexpected number '%f', %i:%i", numberToken.contents, numberToken.m_LineNumber, numberToken.m_Position);
                    }
                    else
                    {
                        Log::Error("Shaders: unexpected number '%i', %i:%i", (int) numberToken.contents, numberToken.m_LineNumber, numberToken.m_Position);
                    }

                    return nullptr;
                }
            }
        }

        Log::Error("Shaders: ran out of tokens building attribute, Line:%i", tokens.back()->m_LineNumber);
        return nullptr;
    }

    Attribute* AttributeBuilder::ValidateAndBuild(int lineNumber, int position)
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


}
