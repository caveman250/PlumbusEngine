using System;
using GlmNet;
using PlumbusEngineMono;

namespace PlumbusTesterMono
{
    public class TestClass : PlumbusComponent
    {
        TestClass(UInt64 nativeObject, UInt64 gameObject)
            : base(nativeObject, gameObject)
        {
        }
        
        void Update()
        {
            GetGameObject().GetComponent<TranslationComponent>().Rotate(new vec3(0.0f, 0.001f, 0.0f));
        }
    }
}