#include "ParserBase.h"
#include "renderer/vk/shader_compiler/ShaderSegment.h"
#include "renderer/vk/shader_compiler/CommonTypes.h"

namespace plumbus::vk::shaders
{
    class UniformParser : public ParserBase
    {
    public:
        UniformParser();

        static bool IsUniformLine(const std::vector<std::unique_ptr<Token>>& tokens, int currIndex);

    private:
        Node* ValidateAndFinalise(int lineNumber, int position) override;
        void HandleCommentToken(CommentToken& token) override;
        void HandleIdentifierToken(IdentifierToken& token) override;
        void HandleNewlineToken(NewlineToken& token) override;
        void HandleSpaceToken(SpaceToken& token) override;
        void HandleTabToken(TabToken& token) override;
        void HandlePunctToken(PunctToken& token) override;
        void HandleOperatorToken(OperatorToken& token) override;
        void HandleNumberToken(NumberToken& token) override;

        enum class State
        {
            None,
            FoundLayout,
            FoundOpenParentheses,
            FoundLocationIdentifier,
            FoundAssignment,
            FoundLocationValue,
            FoundCloseParentheses,
            FoundUniform,
            FoundType,
            FoundName,
            //Following states are only used for buffers.
            FoundOpenBrace,
            FoundVariableType,
            FoundVariableName,
            FoundVariableSemicolon,
            FoundCloseBrace,
            FoundBufferName,
        };

        State m_State;
        int m_Location;
        UniformType m_Type;
        std::string m_Name;

        std::vector<std::pair<VariableType, std::string>> m_BufferValues;
        std::string m_BufferName;
    };
}