using System;
using System.IO;
using System.Reflection;
using System.Runtime.InteropServices;
using NativeLibraryManager;

namespace EngineDotnet
{
    internal class Program
    {
        [DllImport("PlumbusEngine")]
        private static extern void RunApplication();
        
        private static void Main(string[] args)
        {
            var accessor = new ResourceAccessor(Assembly.GetExecutingAssembly());
            var libManager = new LibraryManager(
                Assembly.GetExecutingAssembly(),
                // new LibraryItem(Platform.MacOs, Bitness.x64,
                //     new LibraryFile("libTestLib.dylib", accessor.Binary("libTestLib.dylib"))),
                // new LibraryItem(Platform.Windows, Bitness.x64,
                //     new LibraryFile("TestLib.dll", accessor.Binary("TestLib.dll"))),
                new LibraryItem(Platform.Linux, Bitness.x64,
                    new LibraryFile("libPlumbusEngine.so", File.ReadAllBytes("libPlumbusEngine.so"))));

            libManager.LoadNativeLibrary();

            Environment.CurrentDirectory = Directory.GetCurrentDirectory() + "/../../../../../PlumbusTester";
            RunApplication();
        }
    }
}