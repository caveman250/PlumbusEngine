#pragma once
namespace plumbus::gui
{
    class GuiTool;
    class GuiManager
    {
    public:
        static GuiManager& Get();

        void RegisterGuiTool(GuiTool* tool);
        void UnregisterGuiTool(GuiTool* tool);

        void OnUpdate();
        void OnGui();
    private:
        static GuiManager *s_Instance;

        std::vector<GuiTool*> m_GuiTools;
    };
}
