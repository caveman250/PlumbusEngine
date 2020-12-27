#include "ShaderSettings.h"

#include "Helpers.h"

namespace plumbus::vk::shaders
{
	void ShaderSettings::SetValue(std::string setting, int value)
	{
		m_IntSettings[setting] = value;
	}

	void ShaderSettings::SetValue(std::string setting, float value)
	{
		m_FloatSettings[setting] = value;
	}

	void ShaderSettings::SetValue(std::string setting, bool value)
	{
		m_BoolSettings[setting] = value;
	}

	void ShaderSettings::SetValue(std::string setting, glm::vec2 value)
	{
		m_Vec2Settings[setting] = value;
	}

	void ShaderSettings::SetValue(std::string setting, glm::vec3 value)
	{
		m_Vec3Settings[setting] = value;
	}

	void ShaderSettings::SetValue(std::string setting, glm::vec4 value)
	{
		m_Vec4Settings[setting] = value;
	}

	void ShaderSettings::SetValue(std::string setting, glm::mat3 value)
	{
		m_Mat3Settings[setting] = value;
	}

	void ShaderSettings::SetValue(std::string setting, glm::mat4 value)
	{
		m_Mat4Settings[setting] = value;
	}

	std::string ShaderSettings::GetSettingsGLSL()
	{
		std::string ret = "#version 450\n";
		for (const auto& [key, value] : m_IntSettings)
		{
			ret += GetGLSL(key, value);
		}
		for (const auto& [key, value] : m_FloatSettings)
		{
			ret += GetGLSL(key, value);
		}
		for (const auto& [key, value] : m_BoolSettings)
		{
			ret += GetGLSL(key, value);
		}
		for (const auto& [key, value] : m_Vec2Settings)
		{
			ret += GetGLSL(key, value);
		}
		for (const auto& [key, value] : m_Vec3Settings)
		{
			ret += GetGLSL(key, value);
		}
		for (const auto& [key, value] : m_Vec4Settings)
		{
			ret += GetGLSL(key, value);
		}
		for (const auto& [key, value] : m_Mat3Settings)
		{
			ret += GetGLSL(key, value);
		}
		for (const auto& [key, value] : m_Mat4Settings)
		{
			ret += GetGLSL(key, value);
		}

		return ret;
	}

	std::string ShaderSettings::GetGLSL(std::string setting, int value)
	{
		return Helpers::FormatStr("#define %s %i\n", setting.c_str(), value); 
	}

	std::string ShaderSettings::GetGLSL(std::string setting, float value)
	{
		return Helpers::FormatStr("#define %s %f\n", setting.c_str(), value);
	}

	std::string ShaderSettings::GetGLSL(std::string setting, bool value)
	{
		return Helpers::FormatStr("#define %s %i\n", setting.c_str(), value ? 1 : 0);
	}

	std::string ShaderSettings::GetGLSL(std::string setting, glm::vec2 value)
	{
		return Helpers::FormatStr("#define %s vec2(%f, %f)\n", setting.c_str(), value.x, value.y);
	}

	std::string ShaderSettings::GetGLSL(std::string setting, glm::vec3 value)
	{
		return Helpers::FormatStr("#define %s vec3(%f, %f, %f)\n", setting.c_str(), value.x, value.y, value.z);
	}

	std::string ShaderSettings::GetGLSL(std::string setting, glm::vec4 value)
	{
		return Helpers::FormatStr("#define %s vec4(%f, %f, %f, %f)\n", setting.c_str(), value.x, value.y, value.z, value.w);
	}

	std::string ShaderSettings::GetGLSL(std::string setting, glm::mat3 value)
	{
		return Helpers::FormatStr("#define %s mat3(%f, %f, %f, %f, %f, %f, %f, %f, %f)\n", setting.c_str(),
			value[0].x, value[0].y, value[0].z,
			value[1].x, value[1].y, value[1].z,
			value[2].x, value[2].y, value[2].z);
	}

	std::string ShaderSettings::GetGLSL(std::string setting, glm::mat4 value)
	{
		return Helpers::FormatStr("#define %s mat4(%f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f)\n", setting.c_str(),
		    value[0].x, value[0].y, value[0].z, value[0].w,
		    value[1].x, value[1].y, value[1].z, value[1].w,
		    value[2].x, value[2].y, value[2].z, value[2].w,
		    value[3].x, value[3].y, value[3].z, value[3].w);
	}
}
