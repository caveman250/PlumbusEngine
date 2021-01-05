using System;

namespace PlumbusEngineMono
{
    public class PlumbusComponent : PlumbusObject
    {
        public PlumbusComponent(UInt64 nativeObject, UInt64 gameObject)
         : base(nativeObject)
        {
            m_GameObject = gameObject;
        }

        public GameObject GetGameObject()
        {
            return new GameObject(m_GameObject);
        }

        private UInt64 m_GameObject;
    }
}