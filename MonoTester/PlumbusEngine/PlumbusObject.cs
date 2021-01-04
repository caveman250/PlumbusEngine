using System;

namespace PlumbusEngine
{
    public class PlumbusObject
    {
        public PlumbusObject(IntPtr nativeObject)
        {
            m_NativeObject = nativeObject;
        }

        protected IntPtr m_NativeObject;
    }
}