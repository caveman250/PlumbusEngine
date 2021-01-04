using System;

namespace PlumbusEngine
{
    public class PlumbusComponent : PlumbusObject
    {
        public PlumbusComponent(IntPtr nativeObject, IntPtr gameObject)
         : base(nativeObject)
        {
            m_GameObject = gameObject;
        }

        public GameObject GetGameObject()
        {
            return new GameObject(m_GameObject);
        }

        private IntPtr m_GameObject;
    }
}