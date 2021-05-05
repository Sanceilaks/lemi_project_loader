#include "inject_staff.h"
#include "injections.h"

#include <iostream>
#include <windows.h>
#include <TlHelp32.h>

DWORD inject::find_process_id(const std::string& process_name)
{
	PROCESSENTRY32 process_info;
	process_info.dwSize = sizeof(process_info);

	const HANDLE process_snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (process_snapshot == INVALID_HANDLE_VALUE)
		return 0;

	Process32First(process_snapshot, &process_info);
	if (!process_name.compare(process_info.szExeFile))
	{
		CloseHandle(process_snapshot);
		return process_info.th32ProcessID;
	}

	while (Process32Next(process_snapshot, &process_info))
	{
		if (!process_name.compare(process_info.szExeFile))
		{
			CloseHandle(process_snapshot);
			return process_info.th32ProcessID;
		}
	}

	CloseHandle(process_snapshot);
	return 0;
}


HMODULE inject::get_module(DWORD pid, const std::string& name)
{
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
	if (snapshot == INVALID_HANDLE_VALUE)
		return nullptr;

	MODULEENTRY32 entry;
	if (Module32First(snapshot, &entry))
	{
		do
		{
			if (name.find(entry.szModule))
			{
				CloseHandle(snapshot);
				return (HMODULE)entry.modBaseAddr;
			}
		} while (Module32Next(snapshot, &entry));
	}
	CloseHandle(snapshot);
	return nullptr;
}

bool inject::is_process_ready(DWORD pid)
{
	HWND hwnd = NULL;
	do
	{
		hwnd = FindWindowEx(0, hwnd, 0, 0);
		DWORD cpid = 0;
		GetWindowThreadProcessId(hwnd, &cpid);
		if (cpid == pid)
			break;
	} while (hwnd != nullptr);
	if (!hwnd)
		return false;
	
	return !IsHungAppWindow(hwnd);
}

bool inject::load_library::inject_dll(const std::string& process_name, const std::string& path)
{
	const auto pid = find_process_id(process_name);
	if (!pid)
	{
		std::cout << "Cannot find process" << std::endl;
		return false;
	}

	const HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	if (!process)
	{
		std::cout << "Cannot open process" << std::endl;
		return false;
	}

	LPVOID memory = (LPVOID)VirtualAllocEx(process, nullptr, MAX_PATH, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (!memory)
	{
		std::cout << "Cannot alloc memory" << std::endl;
		return false;
	}

	if (!WriteProcessMemory(process, memory, path.c_str(), MAX_PATH, nullptr))
	{
		std::cout << "Cannot get access to process memory";
		return false;
	}

	HANDLE thread = CreateRemoteThread(process, nullptr, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, memory, 0, nullptr);
	if (!thread)
	{
		std::cout << "Cannot get access to process memory";
		return false;
	}

	CloseHandle(process);
	VirtualFreeEx(process, (LPVOID)memory, 0, MEM_RELEASE);

	return true;
}


bool inject::manual_map::inject_dll(const std::string& process_name, const std::string& path)
{
	const auto pid = find_process_id(process_name);
	if (!pid)
	{
		std::cout << "Cannot find process" << std::endl;
		return false;
	}


	return true;
}

bool inject::nt_open_file::inject_dll(const std::string& process_name, const std::string& path)
{
	
	const auto pid = find_process_id(process_name);
	if (!pid)
	{
		std::cout << "Cannot find process" << std::endl;
		return false;
	}

	const HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	if (!process)
	{
		std::cout << "Cannot open process" << std::endl;
		return false;
	}

	
	LPVOID nt_open_file = GetProcAddress(LoadLibraryA("ntdll.dll"), "NtOpenFile");
	if (nt_open_file)
	{
		char fb[5];
		memcpy(fb, nt_open_file, 5);
		WriteProcessMemory(process, nt_open_file, fb, 5, nullptr);
	}

	auto* mem = VirtualAllocEx(process, nullptr, MAX_PATH, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	WriteProcessMemory(process, mem, path.c_str(), path.size(), nullptr);

	auto* t = CreateRemoteThread(process, 0, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, mem, 0, 0);
	if (t)
		CloseHandle(t);

	CloseHandle(process);
	return true;
}