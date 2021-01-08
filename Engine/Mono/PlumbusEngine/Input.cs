using System;
using System.Runtime.InteropServices;
using GlmNet;

namespace PlumbusEngineMono
{
    public static class Input
    {
	    public enum KeyCode : int
	    {
		    Unknown = -1,
		    Space = 0,
		    Apostrophe = 1,
		    Comma = 3,
		    Minus = 4,
		    Period = 5,
		    Slash = 6,
		    Zero = 7,
		    One = 8,
		    Two = 9,
		    Three = 10,
		    Four = 11,
		    Five = 12,
		    Six = 13,
		    Seven = 14,
		    Eight = 15,
		    Nine = 16,
		    SemiColon = 17,
		    Equal = 18,
		    A = 19,
		    B = 20,
		    C = 21,
		    D = 22,
		    E = 23,
		    F = 24,
		    G = 25,
		    H = 26,
		    I = 27,
		    J = 28,
		    K = 29,
		    L = 30,
		    M = 31,
		    N = 32,
		    O = 33,
		    P = 34,
		    Q = 35,
		    R = 36,
		    S = 37,
		    T = 38,
		    U = 39,
		    V = 40,
		    W = 41,
		    X = 42,
		    Y = 43,
		    Z = 44,
		    LeftBracket = 45,
		    Backslash = 46,
		    RightBracket = 47,
		    GraceAccent = 48,
		    World1 = 49,
		    World2 = 50,
		    Escape = 51,
		    Enter = 52,
		    Tab = 53,
		    Backspace = 54,
		    Insert = 55,
		    Delete = 56,
		    Right = 57,
		    Left = 58,
		    Down = 59,
		    Up = 60,
		    PageUp = 61,
		    PageDown = 62,
		    Home = 63,
		    End = 64,
		    CapsLock = 65,
		    ScrollLock = 66,
		    NumLock = 67,
		    PrintScreen = 68,
		    Pause = 69,
		    F1 = 70,
		    F2 = 71,
		    F3 = 72,
		    F4 = 73,
		    F5 = 74,
		    F6 = 75,
		    F7 = 76,
		    F8 = 77,
		    F9 = 78,
		    F10 = 79,
		    F11 = 80,
		    F12 = 81,
		    F13 = 82,
		    F14 = 83,
		    F15 = 84,
		    F16 = 85,
		    F17 = 86,
		    F18 = 87,
		    F19 = 88,
		    F20 = 89,
		    F21 = 90,
		    F22 = 91,
		    F23 = 92,
		    F24 = 93,
		    F25 = 94,
		    Num0 = 95,
		    Num1 = 96,
		    Num2 = 97,
		    Num3 = 98,
		    Num4 = 99,
		    Num5 = 100,
		    Num6 = 101,
		    Num7 = 102,
		    Num8 = 103,
		    Num9 = 104,
		    NumDecimal = 105,
		    NumDivide = 106,
		    NumMultiply = 107,
		    NumSubtract = 108,
		    NumAdd = 109,
		    NumEnter = 110,
		    NumEqual = 111,
		    LeftShift = 112,
		    LeftControl = 113,
		    LeftAlt = 114,
		    LeftSuper = 115,
		    RightShift = 116,
		    RightControl = 117,
		    RightAlt = 118,
		    RightSuper = 119,
		    Menu = 120,
		    COUNT = Menu
	    };

        [DllImport("__Internal", EntryPoint = "IsKeyDown", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool IsKeyDown(Input.KeyCode keyCode);
        //public static bool IsKeyDown(KeyCode key)
        //{
	       // return Input_IsKeyDown(key);
        //}

        [DllImport("__Internal", EntryPoint = "Input_IsKeyUp", CallingConvention = CallingConvention.Cdecl)]
        private static extern bool Input_IsKeyUp(int keyCode);
        public static bool IsKeyUp(KeyCode key)
        {
	        return Input_IsKeyUp((int)key);
        }

		[DllImport("__Internal", EntryPoint = "Input_IsMouseButtonDown", CallingConvention = CallingConvention.Cdecl)]
        private static extern bool Input_IsMouseButtonDown(int button);
        public static bool IsMouseButtonDown(int button)
        {
	        return Input_IsMouseButtonDown(button);
        }

		[DllImport("__Internal", EntryPoint = "Input_IsMouseButtonUp", CallingConvention = CallingConvention.Cdecl)]
        private static extern bool Input_IsMouseButtonUp(int button);
        public static bool IsMouseButtonUp(int button)
        {
	        return Input_IsMouseButtonUp(button);
        }

		[DllImport("__Internal", EntryPoint = "Input_GetMousePos", CallingConvention = CallingConvention.Cdecl)]
        private static extern vec2 Input_GetMousePos();
        public static vec2 GetMousePos()
        {
	        return Input_GetMousePos();
        }
	}
}