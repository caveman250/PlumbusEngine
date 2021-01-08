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

        [DllImport("__Internal", EntryPoint = "TranslationComponent_SetTranslation", CallingConvention = CallingConvention.Cdecl)]
        private static extern void TranslationComponent_SetTranslation(UInt64 nativeObject, vec3 translation);

        public vec3 Translation
        {
            get => TranslationComponent_GetTranslation(m_NativeObject);
            set => TranslationComponent_SetTranslation(m_NativeObject, value);
        }

        [DllImport("__Internal", EntryPoint = "TranslationComponent_GetRotation", CallingConvention = CallingConvention.Cdecl)]
        private static extern vec3 TranslationComponent_GetRotation(UInt64 nativeObject);

        [DllImport("__Internal", EntryPoint = "TranslationComponent_SetRotation", CallingConvention = CallingConvention.Cdecl)]
        private static extern void TranslationComponent_SetRotation(UInt64 nativeObject, vec3 rotation);

        public vec3 Rotation
        {
            get => TranslationComponent_GetRotation(m_NativeObject);
            set => TranslationComponent_SetRotation(m_NativeObject, value);
        }

        [DllImport("__Internal", EntryPoint = "TranslationComponent_GetScale", CallingConvention = CallingConvention.Cdecl)]
        private static extern vec3 TranslationComponent_GetScale(UInt64 nativeObject);

        [DllImport("__Internal", EntryPoint = "TranslationComponent_SetScale", CallingConvention = CallingConvention.Cdecl)]
        private static extern void TranslationComponent_SetScale(UInt64 nativeObject, vec3 scale);

        public vec3 Scale
        {
            get => TranslationComponent_GetScale(m_NativeObject);
            set => TranslationComponent_SetScale(m_NativeObject, value);
        }
    }
}