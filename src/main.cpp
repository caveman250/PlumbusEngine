#include "Application.h"
#include <iostream>

int main() 
{
	Application::CreateInstance();

	try 
	{
		Application::Get().Run();
	}
	catch (const std::runtime_error& e) 
	{
		std::cerr << e.what() << std::endl;
		throw e;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}