#include "Application.h"
#include <iostream>
#include "imgui_impl/Log.h"

int main() 
{
	Application::CreateInstance();

	try 
	{
		Application::Get().Run();
	}
	catch (const std::runtime_error& e) 
	{
		Log::Fatal(e.what());
		throw e;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}