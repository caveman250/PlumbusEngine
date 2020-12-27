#pragma once

#include "tests/Test.h"

namespace plumbus
{
	namespace base
	{
		class Material;
	}

	namespace tester::tests
	{
		class DeferredLights : public Test
		{
		public:
			DeferredLights();
			~DeferredLights();
			void Init() override;
			void Update() override;
			void Shutdown() override;
			void OnGui() override;
		private:
			double m_LightTime;
			bool m_LightsPaused;
			float m_LightSpeed;
			float m_LightHeight;
			float m_LightRadius;
			float m_LightsDistanceFromCenter;

			vk::MaterialRef m_DeferredLightMaterial;

			vk::MaterialInstanceRef m_AlbedoTextureDescSet;
			vk::MaterialInstanceRef m_WorldPosTextureDescSet;
			vk::MaterialInstanceRef m_NormalsTextureDescSet;
		};
	}
}

