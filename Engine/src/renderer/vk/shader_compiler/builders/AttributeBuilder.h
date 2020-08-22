#include "BuilderBase.h"
#include "renderer/vk/shader_compiler/ShaderSegment.h"
#include "renderer/vk/shader_compiler/CommonTypes.h"

namespace plumbus::vk::shaders
{
    class AttributeBuilder : public BuilderBase
    {
    public:
        AttributeBuilder();

        static bool IsAttributeLine(const std::vector<std::unique_ptr<Token>>& tokens, int currIndex);

    private:
        Node* ValidateAndBuild(int lineNumber, int position) override;
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
            FoundInOut,
            FoundType,
            FoundName
        };

        State m_State;
        int m_Location;
        VariableType m_Type;
        std::string m_Name;

        AttributeDirection m_Direction;
    };
}