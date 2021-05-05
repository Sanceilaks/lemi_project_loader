#pragma once
#include <string>
#include <Windows.h>


namespace inject
{
	namespace load_library
	{
		bool inject_dll(const std::string& process_name, const std::string& path);
	}

	namespace manual_map
	{
		bool inject_dll(const std::string& process_name, const std::string& path); //TODO: Create this
	}

	namespace nt_open_file
	{
		bool inject_dll(const std::string& process_name, const std::string& path);
	}
}