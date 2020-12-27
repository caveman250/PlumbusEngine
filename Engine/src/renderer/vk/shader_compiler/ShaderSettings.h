#pragma once
#include <map>
#include <string>
#include <glm/fwd.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat3x3.hpp>

namespace plumbus::vk::shaders
{
	class ShaderSettings
	{
	public:
		void SetValue(std::string setting, int value);
		void SetValue(std::string setting, float value);
		void SetValue(std::string setting, bool value);
		void SetValue(std::string setting, glm::vec2 value);
		void SetValue(std::string setting, glm::vec3 value);
		void SetValue(std::string setting, glm::vec4 value);
		void SetValue(std::string setting, glm::mat3 value);
		void SetValue(std::string setting, glm::mat4 value);

		std::string GetSettingsGLSL();
	
	private:
		std::string GetGLSL(std::string setting, int value);
		std::string GetGLSL(std::string setting, float value);
		std::string GetGLSL(std::string setting, bool value);
		std::string GetGLSL(std::string setting, glm::vec2 value);
		std::string GetGLSL(std::string setting, glm::vec3 value);
		std::string GetGLSL(std::string setting, glm::vec4 value);
		std::string GetGLSL(std::string setting, glm::mat3 value);
		std::string GetGLSL(std::string setting, glm::mat4 value);
		
		std::map<std::string, int> m_IntSettings;
		std::map<std::string, float> m_FloatSettings;
		std::map<std::string, bool> m_BoolSettings;
		std::map<std::string, glm::vec2> m_Vec2Settings;
		std::map<std::string, glm::vec3> m_Vec3Settings;
		std::map<std::string, glm::vec3> m_Vec4Settings;
		std::map<std::string, glm::mat3> m_Mat3Settings;
		std::map<std::string, glm::mat4> m_Mat4Settings;
	};
}
