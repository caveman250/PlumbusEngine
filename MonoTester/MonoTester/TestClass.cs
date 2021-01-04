using System;
using System.Runtime.InteropServices;
using GlmNet;
using PlumbusEngine;

namespace MonoTester
{
    public class TestClass : PlumbusComponent
    {
        TestClass(IntPtr nativeObject, IntPtr gameObject)
            : base(nativeObject, gameObject)
        {
            int lol = 1;
        }

        private int count = 0;
        private static vec3 translation = new vec3(0, 0, 0);

        [DllImport("__Internal", EntryPoint = "GetComponent", CallingConvention = CallingConvention.Cdecl)]
        private static extern void GetComponent(int type);
        void Update()
        {

            GetComponent(0);
            translation.y -= 0.01f;
            GetGameObject().GetComponent<TranslationComponent>().SetTranslation(translation);
        }
    }
}