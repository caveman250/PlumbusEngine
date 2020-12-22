#include "plumbus.h"
#include "Tokenizer.h"

namespace plumbus::vk::shaders
{
    Tokenizer::Tokenizer()
            : m_LineNumber(0), m_Position(0)
    {

    }

    std::vector<std::unique_ptr<Token>> Tokenizer::TokenizeStream(std::ifstream& stream)
    {
        std::vector<std::unique_ptr<Token>> tokens;

        while (std::unique_ptr<Token> token = ReadNext(stream))
        {
            tokens.push_back(std::move(token));
        }

        return tokens;
    }

    bool Tokenizer::IsWhiteSpace(char c)
    {
#if PL_PLATFORM_WINDOWS
        return isspace(c);
#else
        return std::isspace(c);
#endif
    }

    bool Tokenizer::IsDigit(char c)
    {
#if PL_PLATFORM_WINDOWS
        return isdigit(c);
#else
        return std::isdigit(c);
#endif
    }

    bool Tokenizer::IsPunct(char c)
    {
        return c == ','
               || c == ';'
               || c == '('
               || c == ')'
               || c == '{'
               || c == '}'
               || c == '['
               || c == ']';
    }

    bool Tokenizer::IsOperator(char c)
    {
        return c == '+'
               || c == '-'
               || c == '*'
               || c == '/'
               || c == '%'
               || c == '='
               || c == '&'
               || c == '|'
               || c == '<'
               || c == '>'
               || c == '!';
    }

    std::string Tokenizer::ReadWhile(std::ifstream& stream, std::function<bool(int)> predicate)
    {
        std::string ret;

        int peek = stream.peek();
        while (peek != EOF && predicate(peek))
        {
            if (peek == '\n')
            {
                m_LineNumber++;
                m_Position = 0;
            }
            else
            {
                m_Position++;
            }

            ret += stream.get();
            peek = stream.peek();
        }

        return ret;
    }

    char Tokenizer::Read(std::ifstream& stream)
    {
        char ret = stream.get();
        if (ret == '\n')
        {
            m_LineNumber++;
            m_Position = 0;
        }
        else
        {
            m_Position++;
        }

        return ret;
    }

    std::string Tokenizer::ReadLine(std::ifstream& stream)
    {
        std::string retVal;
        m_LineNumber++;
        m_Position = 0;

        std::getline(stream, retVal);
        return retVal;

    }

    std::unique_ptr<Token> Tokenizer::ReadComment(std::ifstream& stream)
    {
        std::string comment = ReadLine(stream);

        std::unique_ptr<CommentToken> token = std::make_unique<CommentToken>(m_LineNumber, m_Position - comment.length());
        comment += '\n';
        token->contents = comment;
        return token;
    }

    std::unique_ptr<Token> Tokenizer::ReadIdentifier(std::ifstream& stream)
    {
        std::string identifier = ReadWhile(stream, [this](int peek)
        {
            return !IsWhiteSpace(peek) && !IsPunct(peek);
        });

        std::unique_ptr<IdentifierToken> token = std::make_unique<IdentifierToken>(m_LineNumber, m_Position - identifier.length());
        token->contents = identifier;
        return token;
    }

    std::unique_ptr<Token> Tokenizer::ReadNumber(std::ifstream& stream)
    {
        bool hasDecimal = false;
        std::string numberString = ReadWhile(stream, [&hasDecimal](int peek)
        {
            if (peek == '.')
            {
                hasDecimal = true;
            }

            return IsDigit(peek) || peek == '.';
        });
        float number = stof(numberString);

        std::unique_ptr<NumberToken> token = std::make_unique<NumberToken>(m_LineNumber, m_Position - numberString.length());
        token->contents = number;
        token->hasDecimal = hasDecimal;
        return token;
    }

    std::unique_ptr<Token> Tokenizer::ReadPunct(std::ifstream& stream)
    {
        char punct = Read(stream);
        std::unique_ptr<PunctToken> token = std::make_unique<PunctToken>(m_LineNumber, m_Position - 1);
        token->token = punct;
        return token;
    }

    std::unique_ptr<Token> Tokenizer::ReadOperator(std::ifstream& stream)
    {
        char op = Read(stream);
        std::unique_ptr<OperatorToken> token = std::make_unique<OperatorToken>(m_LineNumber, m_Position - 1);
        token->op = op;
        return token;
    }

    std::unique_ptr<Token> Tokenizer::ReadNext(std::ifstream& stream)
    {
        int peek = stream.peek();

        if (peek == std::char_traits<char>::eof())
        {
            return nullptr;
        }

        if (peek == ' ')
        {
            Read(stream);
            return std::make_unique<SpaceToken>(m_LineNumber, m_Position - 1);
        }
        else if (peek == '\t')
        {
            Read(stream);
            return std::make_unique<TabToken>(m_LineNumber, m_Position - 1);
        }
        else if (peek == '#' || IsComment(peek, stream))
        {
            return ReadComment(stream);
        }
        else if (peek == '\n')
        {
            Read(stream);
            return std::make_unique<NewlineToken>(m_LineNumber - 1, m_Position - 1);
        }
        else if (IsPunct(peek))
        {
            return ReadPunct(stream);
        }
        else if (IsOperator(peek))
        {
            return ReadOperator(stream);
        }
        else if (!IsDigit(peek))
        {
            return ReadIdentifier(stream);
        }
        else
        {
            return ReadNumber(stream);
        }
    }

    bool Tokenizer::IsComment(char c, std::ifstream& stream)
    {
        if (c == '/')
        {
            char first = stream.get();
            if (stream.peek() == '/')
            {
                stream.putback(first);
                return true;
            }
        }

        return false;
    }


}