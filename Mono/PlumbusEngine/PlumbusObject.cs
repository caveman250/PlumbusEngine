using System;

namespace PlumbusEngine
{
    public class PlumbusObject
    {
        public PlumbusObject(UInt64 nativeObject)
        {
            m_NativeObject = nativeObject;
        }

        protected UInt64 m_NativeObject;
    }
}