#include "plumbus.h"
#include "platform/Input.h"
#include "renderer/vk/VulkanRenderer.h"

namespace plumbus
{
    std::unordered_map<KeyCode, int> s_KeyCodeToGLFW =
    {
	    {KeyCode::Unknown, GLFW_KEY_UNKNOWN},
	    {KeyCode::Space, GLFW_KEY_SPACE},
	    {KeyCode::Apostrophe, GLFW_KEY_APOSTROPHE /* ' */},
	    {KeyCode::Comma, GLFW_KEY_COMMA /* , */},
	    {KeyCode::Minus, GLFW_KEY_MINUS /* - */},
	    {KeyCode::Period, GLFW_KEY_PERIOD /* . */},
	    {KeyCode::Slash, GLFW_KEY_SLASH /* / */},
	    {KeyCode::Zero, GLFW_KEY_0},
	    {KeyCode::One, GLFW_KEY_1},
	    {KeyCode::Two, GLFW_KEY_2},
	    {KeyCode::Three, GLFW_KEY_3},
	    {KeyCode::Four, GLFW_KEY_4},
	    {KeyCode::Five, GLFW_KEY_5},
	    {KeyCode::Six, GLFW_KEY_6},
	    {KeyCode::Seven, GLFW_KEY_7},
	    {KeyCode::Eight, GLFW_KEY_8},
	    {KeyCode::Nine, GLFW_KEY_9},
	    {KeyCode::SemiColon, GLFW_KEY_SEMICOLON /* ; */},
	    {KeyCode::Equal, GLFW_KEY_EQUAL /* = */},
	    {KeyCode::A, GLFW_KEY_A},
	    {KeyCode::B, GLFW_KEY_B},
	    {KeyCode::C, GLFW_KEY_C},
	    {KeyCode::D, GLFW_KEY_D},
	    {KeyCode::E, GLFW_KEY_E},
	    {KeyCode::F, GLFW_KEY_F},
	    {KeyCode::G, GLFW_KEY_G},
	    {KeyCode::H, GLFW_KEY_H},
	    {KeyCode::I, GLFW_KEY_I},
	    {KeyCode::J, GLFW_KEY_J},
	    {KeyCode::K, GLFW_KEY_K},
	    {KeyCode::L, GLFW_KEY_L},
	    {KeyCode::M, GLFW_KEY_M},
	    {KeyCode::N, GLFW_KEY_N},
	    {KeyCode::O, GLFW_KEY_O},
	    {KeyCode::P, GLFW_KEY_P},
	    {KeyCode::Q, GLFW_KEY_Q},
	    {KeyCode::R, GLFW_KEY_R},
	    {KeyCode::S, GLFW_KEY_S},
	    {KeyCode::T, GLFW_KEY_T},
	    {KeyCode::U, GLFW_KEY_U},
	    {KeyCode::V, GLFW_KEY_V},
	    {KeyCode::W, GLFW_KEY_W},
	    {KeyCode::X, GLFW_KEY_X},
	    {KeyCode::Y, GLFW_KEY_Y},
	    {KeyCode::Z, GLFW_KEY_Z},
	    {KeyCode::LeftBracket, GLFW_KEY_LEFT_BRACKET /* [ */},
	    {KeyCode::Backslash, GLFW_KEY_BACKSLASH /* \ */},
	    {KeyCode::RightBracket, GLFW_KEY_RIGHT_BRACKET /* ] */},
	    {KeyCode::GraceAccent, GLFW_KEY_GRAVE_ACCENT /* ` */},
	    {KeyCode::World1, GLFW_KEY_WORLD_1 /* non-US #1 */},
	    {KeyCode::World2, GLFW_KEY_WORLD_2 /* non-US #2 */},
	    {KeyCode::Escape, GLFW_KEY_ESCAPE},
	    {KeyCode::Enter, GLFW_KEY_ENTER},
	    {KeyCode::Tab, GLFW_KEY_TAB},
	    {KeyCode::Backspace, GLFW_KEY_BACKSPACE},
	    {KeyCode::Insert, GLFW_KEY_INSERT},
	    {KeyCode::Delete, GLFW_KEY_DELETE},
	    {KeyCode::Right, GLFW_KEY_RIGHT},
	    {KeyCode::Left, GLFW_KEY_LEFT},
	    {KeyCode::Down, GLFW_KEY_DOWN},
	    {KeyCode::Up, GLFW_KEY_UP},
	    {KeyCode::PageUp, GLFW_KEY_PAGE_UP},
	    {KeyCode::PageDown, GLFW_KEY_PAGE_DOWN},
	    {KeyCode::Home, GLFW_KEY_HOME},
	    {KeyCode::End, GLFW_KEY_END},
	    {KeyCode::CapsLock, GLFW_KEY_CAPS_LOCK},
	    {KeyCode::ScrollLock, GLFW_KEY_SCROLL_LOCK},
	    {KeyCode::NumLock, GLFW_KEY_NUM_LOCK},
	    {KeyCode::PrintScreen, GLFW_KEY_PRINT_SCREEN},
	    {KeyCode::Pause, GLFW_KEY_PAUSE},
	    {KeyCode::F1, GLFW_KEY_F1},
	    {KeyCode::F2, GLFW_KEY_F2},
	    {KeyCode::F3, GLFW_KEY_F3},
	    {KeyCode::F4, GLFW_KEY_F4},
	    {KeyCode::F5, GLFW_KEY_F5},
	    {KeyCode::F6, GLFW_KEY_F6},
	    {KeyCode::F7, GLFW_KEY_F7},
	    {KeyCode::F8, GLFW_KEY_F8},
	    {KeyCode::F9, GLFW_KEY_F9},
	    {KeyCode::F10, GLFW_KEY_F10},
	    {KeyCode::F11, GLFW_KEY_F11},
	    {KeyCode::F12, GLFW_KEY_F12},
	    {KeyCode::F13, GLFW_KEY_F13},
	    {KeyCode::F14, GLFW_KEY_F14},
	    {KeyCode::F15, GLFW_KEY_F15},
	    {KeyCode::F16, GLFW_KEY_F16},
	    {KeyCode::F17, GLFW_KEY_F17},
	    {KeyCode::F18, GLFW_KEY_F18},
	    {KeyCode::F19, GLFW_KEY_F19},
	    {KeyCode::F20, GLFW_KEY_F20},
	    {KeyCode::F21, GLFW_KEY_F21},
	    {KeyCode::F22, GLFW_KEY_F22},
	    {KeyCode::F23, GLFW_KEY_F23},
	    {KeyCode::F24, GLFW_KEY_F24},
	    {KeyCode::F25, GLFW_KEY_F25},
	    {KeyCode::Num0, GLFW_KEY_KP_0},
	    {KeyCode::Num1, GLFW_KEY_KP_1},
	    {KeyCode::Num2, GLFW_KEY_KP_2},
	    {KeyCode::Num3, GLFW_KEY_KP_3},
	    {KeyCode::Num4, GLFW_KEY_KP_4},
	    {KeyCode::Num5, GLFW_KEY_KP_5},
	    {KeyCode::Num6, GLFW_KEY_KP_6},
	    {KeyCode::Num7, GLFW_KEY_KP_7},
	    {KeyCode::Num8, GLFW_KEY_KP_8},
	    {KeyCode::Num9, GLFW_KEY_KP_9},
	    {KeyCode::NumDecimal, GLFW_KEY_KP_DECIMAL},
	    {KeyCode::NumDivide, GLFW_KEY_KP_DIVIDE},
	    {KeyCode::NumMultiply, GLFW_KEY_KP_MULTIPLY},
	    {KeyCode::NumSubtract, GLFW_KEY_KP_SUBTRACT},
	    {KeyCode::NumAdd, GLFW_KEY_KP_ADD},
	    {KeyCode::NumEnter, GLFW_KEY_KP_ENTER},
	    {KeyCode::NumEqual, GLFW_KEY_KP_EQUAL},
	    {KeyCode::LeftShift, GLFW_KEY_LEFT_SHIFT},
	    {KeyCode::LeftControl, GLFW_KEY_LEFT_CONTROL},
	    {KeyCode::LeftAlt, GLFW_KEY_LEFT_ALT},
	    {KeyCode::LeftSuper, GLFW_KEY_LEFT_SUPER},
	    {KeyCode::RightShift, GLFW_KEY_RIGHT_SHIFT},
	    {KeyCode::RightControl, GLFW_KEY_RIGHT_CONTROL},
	    {KeyCode::RightAlt, GLFW_KEY_RIGHT_ALT},
	    {KeyCode::RightSuper, GLFW_KEY_RIGHT_SUPER},
	    {KeyCode::Menu, GLFW_KEY_MENU}
    };

    bool Input::IsKeyDown(KeyCode keyCode)
    {
        return glfwGetKey(vk::VulkanRenderer::Get()->GetWindow()->GetWindow(), s_KeyCodeToGLFW[keyCode]) == GLFW_PRESS;
    }

    bool Input::IsKeyUp(plumbus::KeyCode keyCode)
    {
        return glfwGetKey(vk::VulkanRenderer::Get()->GetWindow()->GetWindow(), s_KeyCodeToGLFW[keyCode]) == GLFW_RELEASE;
    }

    glm::vec2 Input::GetMousePos()
    {
        vk::Window* window = vk::VulkanRenderer::Get()->GetWindow();

        double xpos, ypos;
        glfwGetCursorPos(window->GetWindow(), &xpos, &ypos);

        return glm::vec2(xpos * window->GetContentScaleX(), ypos * window->GetContentScaleY());
    }

    bool Input::IsMouseButtonDown(int button)
    {
        return glfwGetMouseButton(vk::VulkanRenderer::Get()->GetWindow()->GetWindow(), button) == GLFW_PRESS;
    }

    bool Input::IsMouseButtonUp(int button)
    {
        return glfwGetMouseButton(vk::VulkanRenderer::Get()->GetWindow()->GetWindow(), button) == GLFW_RELEASE;
    }
}

bool IsKeyDown(int keyCode)
{
    return plumbus::Input::IsKeyDown(static_cast<plumbus::KeyCode>(keyCode));
}

bool Input_IsKeyUp(int keyCode)
{
    return plumbus::Input::IsKeyUp(static_cast<plumbus::KeyCode>(keyCode));
}

bool Input_IsMouseButtonDown(int button)
{
    return plumbus::Input::IsMouseButtonDown(button);
}

bool Input_IsMouseButtonUp(int button)
{
    return plumbus::Input::IsMouseButtonUp(button);
}

mono_vec2 Input_GetMousePos()
{
    return plumbus::Input::GetMousePos();
}