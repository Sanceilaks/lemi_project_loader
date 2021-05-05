#pragma once
#include <string>
#include <Windows.h>

namespace inject
{
	DWORD find_process_id(const std::string& process_name);
	HMODULE get_module(DWORD pid, const std::string& name);
	bool is_process_ready(DWORD pid);
}
