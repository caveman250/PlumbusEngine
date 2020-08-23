#pragma once
#include "renderer/vk/shader_compiler/Tokenizer.h"
#include <vector>
#include "renderer/vk/shader_compiler/CommonTypes.h"

namespace plumbus::vk::shaders
{
    class ParserBase
    {
    public:
        Node* Parse(const std::vector<std::unique_ptr<Token>>& tokens, int& currIndex);
    protected:
        virtual Node* ValidateAndFinalise(int lineNumber, int position) = 0;
        virtual void HandleCommentToken(CommentToken& token) = 0;
        virtual void HandleIdentifierToken(IdentifierToken& token) = 0;
        virtual void HandleNewlineToken(NewlineToken& token) = 0;
        virtual void HandleSpaceToken(SpaceToken& token) = 0;
        virtual void HandleTabToken(TabToken& token) = 0;
        virtual void HandlePunctToken(PunctToken& token) = 0;
        virtual void HandleOperatorToken(OperatorToken& token) = 0;
        virtual void HandleNumberToken(NumberToken& token) = 0;

        void LogIdentifierError(IdentifierToken& token);
        void LogNewlineError(NewlineToken& token);
        void LogPunctError(PunctToken& token);
        void LogOperatorError(OperatorToken& token);
        void LogNumberError(NumberToken& token);

        bool m_Finished = false;
        bool m_Error = false;
    };
}
