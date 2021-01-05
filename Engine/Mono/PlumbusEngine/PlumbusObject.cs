using System;

namespace PlumbusEngineMono
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