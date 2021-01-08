using System;
using GlmNet;
using PlumbusEngineMono;

namespace PlumbusTesterMono
{
    public class TestClass : PlumbusComponent
    {
        private readonly TranslationComponent m_TranslationComponent;
        private vec2 m_MousePos; 
        private const float s_CameraSpeed = 5.0f;

        TestClass(UInt64 nativeObject, UInt64 gameObject)
            : base(nativeObject, gameObject)
        {
            m_TranslationComponent = GetGameObject().GetComponent<TranslationComponent>();
        }

        void Update()
        {
            float deltaTime = (float)Application.DeltaTime;
            UpdateCamera(deltaTime);
            UpdateModel(deltaTime);
        }

        private void UpdateCamera(float deltaTime)
        {
            vec2 mousePos = Input.GetMousePos();
            float dx = m_MousePos.x - mousePos.x;
            float dy = m_MousePos.y - mousePos.y;
            m_MousePos = mousePos;

            if (Input.IsMouseButtonDown(0))
            {
                Camera.Rotation += new vec3(dy * 1.0f, -dx * 1.0f, 0.0f);
            }
            
            vec3 cameraForward = Camera.Forward;

            if (Input.IsKeyDown(Input.KeyCode.W))
            {
                Camera.Position += cameraForward * s_CameraSpeed * deltaTime;
            }

            if (Input.IsKeyDown(Input.KeyCode.S))
            {
                Camera.Position -= cameraForward * s_CameraSpeed * deltaTime;
            }

            if (Input.IsKeyDown(Input.KeyCode.A))
            {
                Camera.Position -= glm.normalize(glm.cross(cameraForward, new vec3(0.0f, 1.0f, 0.0f))) * s_CameraSpeed * deltaTime;
            }

            if (Input.IsKeyDown(Input.KeyCode.D))
            {
                Camera.Position += glm.normalize(glm.cross(cameraForward, new vec3(0.0f, 1.0f, 0.0f))) * s_CameraSpeed * deltaTime;
            }
        }

        private void UpdateModel(float deltaTime)
        {
            m_TranslationComponent.Rotation += new vec3(0.0f, 5.0f * deltaTime, 0.0f);
        }
    }
}