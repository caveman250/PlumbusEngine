#pragma once
#include <fstream>
#include <string>
#include <functional>

namespace plumbus::vk::shaders
{
    enum class TokenType
    {
        Comment,
        Identifier,
        Number,
        Punct,
        Space,
        Tab,
        Newline,
        Operator
    };

    struct Token
    {
        Token(TokenType tokenType, int lineNumber, int linePosition) : m_Type(tokenType), m_LineNumber(lineNumber), m_Position(linePosition) {}
        TokenType m_Type;
        int m_LineNumber;
        int m_Position;

        virtual void PrintContents() = 0;
    };

    struct CommentToken : public Token
    {
        CommentToken(int lineNumber, int linePosition) : Token(TokenType::Comment, lineNumber, linePosition) {}
        std::string contents;

        void PrintContents() override { printf("Comment Token: %s", contents.c_str()); }
    };

    struct IdentifierToken : public Token
    {
        IdentifierToken(int lineNumber, int linePosition) : Token(TokenType::Identifier, lineNumber, linePosition) {}
        std::string contents;

        void PrintContents() override { printf("Identifier Token: %s\n", contents.c_str()); }
    };

    struct NewlineToken : public Token
    {
        NewlineToken(int lineNumber, int linePosition) : Token(TokenType::Newline, lineNumber, linePosition) {}
        void PrintContents() override { printf("Newline Token: \n"); }
    };

    struct SpaceToken : public Token
    {
        SpaceToken(int lineNumber, int linePosition) : Token(TokenType::Space, lineNumber, linePosition) {}
        void PrintContents() override { printf("Space Token: \n"); }
    };

    struct TabToken : public Token
    {
        TabToken(int lineNumber, int linePosition) : Token(TokenType::Tab, lineNumber, linePosition) {}
        void PrintContents() override { printf("Tab Token: \n"); }
    };

    struct PunctToken : public Token
    {
        PunctToken(int lineNumber, int linePosition) : Token(TokenType::Punct, lineNumber, linePosition), token() {}

        char token;
        void PrintContents() override { printf("Punct Token: %c\n", token); }
    };

    struct OperatorToken : public Token
    {
        OperatorToken(int lineNumber, int linePosition) : Token(TokenType::Operator, lineNumber, linePosition), op() {}

        char op;
        void PrintContents() override { printf("Operator Token: %c\n", op); }
    };

    struct NumberToken : public Token
    {
        NumberToken(int lineNumber, int linePosition) : Token(TokenType::Number, lineNumber, linePosition), contents(0.f) {}
        float contents;
        bool hasDecimal = false;

        void PrintContents() override 
        { 
            if(hasDecimal)
            {
                printf("Number Token: %f\n", contents); 
            }
            else
            {
                printf("Number Token: %i\n", (int)contents); 
            }
            
        }
    };

    class Tokenizer
    {
    public:
        Tokenizer();
        std::vector<std::unique_ptr<Token>> TokenizeStream(std::ifstream& stream);

    private:
        std::unique_ptr<Token> ReadNext(std::ifstream& stream);
        std::string ReadWhile(std::ifstream& stream, std::function<bool(int)> predicate);
        char Read(std::ifstream& stream);
        std::string ReadLine(std::ifstream& stream);

        std::unique_ptr<Token> ReadComment(std::ifstream& stream);
        std::unique_ptr<Token> ReadIdentifier(std::ifstream& stream);
        std::unique_ptr<Token> ReadNumber(std::ifstream& stream);
        std::unique_ptr<Token> ReadPunct(std::ifstream& stream);
        std::unique_ptr<Token> ReadOperator(std::ifstream& stream);

        bool IsWhiteSpace(char c);
        bool IsPunct(char c);
        bool IsOperator(char c);

        int m_LineNumber;
        int m_Position;
    };
}