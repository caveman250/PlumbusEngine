#include "GuiManager.h"
#include "GuiTool.h"

namespace plumbus::gui
{
    GuiManager* GuiManager::s_Instance = nullptr;

    GuiManager& GuiManager::Get()
    {
        if (s_Instance == nullptr)
        {
            s_Instance = new GuiManager();
        }

        return *s_Instance;
    }

    void GuiManager::RegisterGuiTool(GuiTool* tool)
    {
        m_GuiTools.push_back(tool);
    }

    void GuiManager::UnregisterGuiTool(GuiTool* tool)
    {
        m_GuiTools.erase(std::remove(m_GuiTools.begin(), m_GuiTools.end(), tool), m_GuiTools.end());
    }

    void GuiManager::OnUpdate()
    {
        for (GuiTool* guiTool : m_GuiTools)
        {
            guiTool->Update();
        }
    }

    void GuiManager::OnGui()
    {
        for (GuiTool* guiTool : m_GuiTools)
        {
            guiTool->OnGui();
        }
    }
}
