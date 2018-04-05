#pragma once
#include <vector>
#include <fstream>
#include <assert.h>

class Helpers
{
public:
	static std::vector<char> ReadFile(const std::string& filename)
	{
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open())
		{
			Helpers::LogFatal("failed to open file!");
		}

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	}

	static void LogInfo(std::string msg)
	{
		msg += "\n";
		printf(msg.c_str());
	}

	static void LogFatal(std::string msg)
	{
		msg += "\n";
		printf(msg.c_str());
		assert(false);
	}
};