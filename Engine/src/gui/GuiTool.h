#pragma once
namespace plumbus::gui
{
    class GuiTool
    {
    public:
        GuiTool();
        virtual ~GuiTool();

        virtual void Update() {}
        virtual void OnGui() {}
    };
}
