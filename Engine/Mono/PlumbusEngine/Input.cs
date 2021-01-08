using System;
using System.Runtime.InteropServices;
using GlmNet;

namespace PlumbusEngineMono
{
    public static class Input
    {
        [DllImport("__Internal", EntryPoint = "IsKeyDown", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool IsKeyDown(int keyCode);

        [DllImport("__Internal", EntryPoint = "IsKeyUp", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool IsKeyUp(int keyCode);

        [DllImport("__Internal", EntryPoint = "IsMouseButtonDown", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool IsMouseButtonDown(int button);

        [DllImport("__Internal", EntryPoint = "IsMouseButtonUp", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool IsMouseButtonUp(int button);

        [DllImport("__Internal", EntryPoint = "GetMousePos", CallingConvention = CallingConvention.Cdecl)]
        public static extern vec2 GetMousePos();
    }
}