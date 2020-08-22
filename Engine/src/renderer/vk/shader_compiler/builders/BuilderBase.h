#pragma once
#include "renderer/vk/shader_compiler/Tokenizer.h"
#include <vector>
#include "renderer/vk/shader_compiler/CommonTypes.h"

namespace plumbus::vk::shaders
{
    class BuilderBase
    {
    public:
        virtual Node* Build(const std::vector<std::unique_ptr<Token>>& tokens, int& currIndex);
    protected:
        virtual Node* ValidateAndBuild(int lineNumber, int position) = 0;
        virtual void HandleCommentToken(CommentToken& token) = 0;
        virtual void HandleIdentifierToken(IdentifierToken& token) = 0;
        virtual void HandleNewlineToken(NewlineToken& token) = 0;
        virtual void HandleSpaceToken(SpaceToken& token) = 0;
        virtual void HandleTabToken(TabToken& token) = 0;
        virtual void HandlePunctToken(PunctToken& token) = 0;
        virtual void HandleOperatorToken(OperatorToken& token) = 0;
        virtual void HandleNumberToken(NumberToken& token) = 0;

        bool m_Finished = false;
        bool m_Error = false;
    };
}
