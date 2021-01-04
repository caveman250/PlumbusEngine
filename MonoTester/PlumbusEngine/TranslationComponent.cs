using System;
using System.Runtime.InteropServices;
using GlmNet;

namespace PlumbusEngine
{
    public class TranslationComponent : PlumbusObject
    {
        public TranslationComponent(IntPtr nativeObj)
         : base(nativeObj)
        {
        }

        [DllImport("__Internal", EntryPoint = "GetTranslation", CallingConvention = CallingConvention.Cdecl)]
        private static extern vec3 GetTranslation(IntPtr nativeObject);
        public vec3 GetTranslation()
        {
            return GetTranslation(m_NativeObject);
        }

        [DllImport("__Internal", EntryPoint = "GetRotation", CallingConvention = CallingConvention.Cdecl)]
        private static extern vec3 GetRotation(IntPtr nativeObject);
        public vec3 GetRotation()
        {
            return GetRotation(m_NativeObject);
        }

        [DllImport("__Internal", EntryPoint = "GetScale", CallingConvention = CallingConvention.Cdecl)]
        private static extern vec3 GetScale(IntPtr nativeObject);
        public vec3 GetScale()
        {
            return GetScale(m_NativeObject);
        }

        [DllImport("__Internal", EntryPoint = "SetTranslation", CallingConvention = CallingConvention.Cdecl)]
        private static extern void SetTranslation(IntPtr nativeObject, vec3 translation);
        public void SetTranslation(vec3 translation)
        {
            SetTranslation(m_NativeObject, translation);
        }

        [DllImport("__Internal", EntryPoint = "SetRotation", CallingConvention = CallingConvention.Cdecl)]
        private static extern void SetRotation(IntPtr nativeObject, vec3 rotation);
        public void SetRotation(vec3 rotation)
        {
            SetRotation(m_NativeObject, rotation);
        }

        [DllImport("__Internal", EntryPoint = "SetScale", CallingConvention = CallingConvention.Cdecl)]
        private static extern void SetScale(IntPtr nativeObject, vec3 scale);
        public void SetScale(vec3 scale)
        {
            SetScale(m_NativeObject, scale);
        }

        [DllImport("__Internal", EntryPoint = "Translate", CallingConvention = CallingConvention.Cdecl)]
        private static extern void Translate(IntPtr nativeObject, vec3 translation);
        public void Translate(vec3 translation)
        {
            Translate(m_NativeObject, translation);
        }
        
        [DllImport("__Internal", EntryPoint = "Rotate", CallingConvention = CallingConvention.Cdecl)]
        private static extern void Rotate(IntPtr nativeObject, vec3 rotation);
        public void Rotate(vec3 rotation)
        {
            Rotate(m_NativeObject, rotation);
        }

        [DllImport("__Internal", EntryPoint = "Scale", CallingConvention = CallingConvention.Cdecl)]
        private static extern void Scale(IntPtr nativeObject, vec3 scale);
        public void Scale(vec3 scale)
        {
            Scale(m_NativeObject, scale);
        }
    }
}