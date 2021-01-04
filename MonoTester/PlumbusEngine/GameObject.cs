using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using GlmNet;

namespace PlumbusEngine
{
    public class GameObject : PlumbusObject
    {
        public GameObject(IntPtr nativeObject)
            : base(nativeObject)
        {
        }
        
        public static GameObject FindGameObject(string name)
        {
            return s_GameObjects.Last();
        }

        public static void RegisterGameObject(IntPtr nativeObject)
        {
            s_GameObjects.Add(new GameObject(nativeObject));
        }

        enum ComponentType
        {
            None,
            TranslationComponent,
        };
        [DllImport("__Internal", EntryPoint = "GetComponent", CallingConvention = CallingConvention.Cdecl)]
        private static extern void GetComponent(int type);
        public T GetComponent<T>()
        {
            T obj = default(T);
            ComponentType type = ComponentType.None;
            var getTypeSwitch = new Dictionary<Type, Action>
            {
                {typeof(TranslationComponent), () => type = ComponentType.TranslationComponent},
            };
            getTypeSwitch[typeof(T)]();
            
            GetComponent((int) type);
            //IntPtr nativeObj = 
            //var contructorSwitch = new Dictionary<Type, Action>
            //{
            //    {typeof(TranslationComponent), () => obj = (T)(object)new TranslationComponent(nativeObj) },
            //};
            //
            //contructorSwitch[typeof(T)]();
            return obj;
        }

        private static List<GameObject> s_GameObjects = new List<GameObject>();
    }
}