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
	        if (Input.IsKeyDown(Input.KeyCode.One))
            {
	            GetGameObject().GetComponent<TranslationComponent>().Rotate(new vec3(0.0f, 0.001f, 0.0f));
            }

	        //if (Input.IsKeyDown(Input.KeyCode.Two))
	        //{
		       // GetGameObject().GetComponent<TranslationComponent>().Translate(new vec3(0.0f, 0.001f, 0.0f));
	        //}

	        //if (Input.IsKeyDown(Input.KeyCode.Three))
	        //{
		       // GetGameObject().GetComponent<TranslationComponent>().Scale(new vec3(0.001f, 0.001f, 0.001f));
	        //}
        }
    }
}