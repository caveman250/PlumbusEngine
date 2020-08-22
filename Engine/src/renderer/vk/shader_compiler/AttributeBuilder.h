#include "Tokenizer.h"
#include "ShaderSegment.h"
#include "CommonTypes.h"

namespace plumbus::vk::shaders
{
    class AttributeBuilder
    {
    public:
        AttributeBuilder();

        static bool IsAttributeLine(const std::vector<std::unique_ptr<Token>>& tokens, int currIndex);
        Attribute* BuildAttribute(const std::vector<std::unique_ptr<Token>>& tokens, int& currIndex);

    private:
        Attribute* ValidateAndBuild(int lineNumber, int position);

        void HandleToken(Token* token);
        void HandleCommentToken(CommentToken* token);
        void HandleIdentifierToken(IdentifierToken* token);


        enum class State
        {
            None,
            Foundlayout,
            FoundOpenParenthisis,
            FoundLocationIdentifier,
            FoundAssignment,
            FoundLocationValue,
            FoundCloseParenthisis,
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