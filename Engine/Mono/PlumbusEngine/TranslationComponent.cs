using System;
using System.Runtime.InteropServices;
using GlmNet;

namespace PlumbusEngineMono
{
    public class TranslationComponent : PlumbusObject
    {
        public TranslationComponent(UInt64 nativeObj)
            : base(nativeObj)
        {
        }

        [DllImport("__Internal", EntryPoint = "TranslationComponent_GetTranslation", CallingConvention = CallingConvention.Cdecl)]
        private static extern vec3 TranslationComponent_GetTranslation(UInt64 nativeObject);

        public vec3 GetTranslation()
        {
            return TranslationComponent_GetTranslation(m_NativeObject);
        }

        [DllImport("__Internal", EntryPoint = "TranslationComponent_GetRotation", CallingConvention = CallingConvention.Cdecl)]
        private static extern vec3 TranslationComponent_GetRotation(UInt64 nativeObject);

        public vec3 GetRotation()
        {
            return TranslationComponent_GetRotation(m_NativeObject);
        }

        [DllImport("__Internal", EntryPoint = "TranslationComponent_GetScale", CallingConvention = CallingConvention.Cdecl)]
        private static extern vec3 TranslationComponent_GetScale(UInt64 nativeObject);

        public vec3 GetScale()
        {
            return TranslationComponent_GetScale(m_NativeObject);
        }

        [DllImport("__Internal", EntryPoint = "TranslationComponent_SetTranslation", CallingConvention = CallingConvention.Cdecl)]
        private static extern void TranslationComponent_SetTranslation(UInt64 nativeObject, vec3 translation);

        public void SetTranslation(vec3 translation)
        {
	        TranslationComponent_SetTranslation(m_NativeObject, translation);
        }

        [DllImport("__Internal", EntryPoint = "TranslationComponent_SetRotation", CallingConvention = CallingConvention.Cdecl)]
        private static extern void TranslationComponent_SetRotation(UInt64 nativeObject, vec3 rotation);

        public void SetRotation(vec3 rotation)
        {
	        TranslationComponent_SetRotation(m_NativeObject, rotation);
        }

        [DllImport("__Internal", EntryPoint = "TranslationComponent_SetScale", CallingConvention = CallingConvention.Cdecl)]
        private static extern void TranslationComponent_SetScale(UInt64 nativeObject, vec3 scale);

        public void SetScale(vec3 scale)
        {
	        TranslationComponent_SetScale(m_NativeObject, scale);
        }

        [DllImport("__Internal", EntryPoint = "TranslationComponent_Translate", CallingConvention = CallingConvention.Cdecl)]
        private static extern void TranslationComponent_Translate(UInt64 nativeObject, vec3 translation);

        public void Translate(vec3 translation)
        {
	        TranslationComponent_Translate(m_NativeObject, translation);
        }

        [DllImport("__Internal", EntryPoint = "TranslationComponent_Rotate", CallingConvention = CallingConvention.Cdecl)]
        private static extern void TranslationComponent_Rotate(UInt64 nativeObject, vec3 rotation);

        public void Rotate(vec3 rotation)
        {
	        TranslationComponent_Rotate(m_NativeObject, rotation);
        }

        [DllImport("__Internal", EntryPoint = "TranslationComponent_Scale", CallingConvention = CallingConvention.Cdecl)]
        private static extern void TranslationComponent_Scale(UInt64 nativeObject, vec3 scale);

        public void Scale(vec3 scale)
        {
	        TranslationComponent_Scale(m_NativeObject, scale);
        }
    }
}