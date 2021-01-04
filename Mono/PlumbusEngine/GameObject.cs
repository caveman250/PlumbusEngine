using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using GlmNet;

namespace PlumbusEngine
{
    public class GameObject : PlumbusObject
    {
        public GameObject(UInt64 nativeObject)
            : base(nativeObject)
        {
        }
        
        public static GameObject FindGameObject(string name)
        {
            return s_GameObjects.Last();
        }

        public static void RegisterGameObject(System.UInt64 nativeObject)
        {
            s_GameObjects.Add(new GameObject(nativeObject));
        }

        enum ComponentType
        {
            TranslationComponent,
            None,
        };
        [DllImport("__Internal", EntryPoint = "GetComponent", CallingConvention = CallingConvention.Cdecl)]
        private static extern UInt64 GetComponent(UInt64 nativeObj, int type);
        public T GetComponent<T>()
        {
            T obj = default(T);
            ComponentType type = ComponentType.None;
            var getTypeSwitch = new Dictionary<Type, Action>
            {
                {typeof(TranslationComponent), () => type = ComponentType.TranslationComponent},
            };
            getTypeSwitch[typeof(T)]();
            
            UInt64 nativeObj = GetComponent(m_NativeObject, (int)type);
            var contructorSwitch = new Dictionary<Type, Action>
            {
                {typeof(TranslationComponent), () => obj = (T)(object)new TranslationComponent(nativeObj) },
            };
            
            contructorSwitch[typeof(T)]();
            return obj;
        }

        private static List<GameObject> s_GameObjects = new List<GameObject>();
    }
}