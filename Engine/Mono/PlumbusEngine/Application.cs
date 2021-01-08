using System.Runtime.InteropServices;
using GlmNet;

namespace PlumbusEngineMono
{
    public static class Application
    {
        [DllImport("__Internal", EntryPoint = "Application_GetDeltaTime", CallingConvention = CallingConvention.Cdecl)]
        private static extern double Application_GetDeltaTime();
        public static double DeltaTime => Application_GetDeltaTime();
    }
}