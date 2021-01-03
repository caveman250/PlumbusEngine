#include "GuiTool.h"
#include "GuiManager.h"

namespace plumbus::gui
{
    GuiTool::GuiTool()
    {
    }

    GuiTool::~GuiTool()
    {
        GuiManager::Get().UnregisterGuiTool(this);
    }
}
