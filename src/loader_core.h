#pragma once
#include "download.h"
#include "file_tools.h"
#include "hack_list.h"

#include "inject_staff/injections.h"
#include "windows/main_window.h"
#include "inject_staff/inject_staff.h"

namespace loader_core
{
	enum class e_inject_type
	{
		load_library,
		nt_open_file,
		manual_map,
	};

	const char* to_string(e_inject_type e)
	{
		switch (e)
		{
		case e_inject_type::load_library: return "load_library";
		case e_inject_type::nt_open_file: return "nt_open_file";
		case e_inject_type::manual_map: return "manual_map";
		default: return "unknown";
		}
	}

	inline bool is_loading = false;
	inline std::thread loading_thread;
	inline bool should_cancel = false;
	
	inline bool load(const c_base_hack& h)
	{
		is_loading = true;
		using load_fn = bool(const std::string&, const std::string&);
		load_fn* fn = nullptr;

		file_tools::log_to_file("Start loading");
		
		switch (h.get_inject_method())
		{
		case (int)e_inject_type::load_library:
			fn = inject::load_library::inject_dll;
			break;
		case (int)e_inject_type::nt_open_file:
			fn = inject::nt_open_file::inject_dll;
			break;
		default:
				fn = inject::nt_open_file::inject_dll;
		}

		file_tools::log_to_file("Trying to get hack path");
		
		auto file_name = h.get_name() + ".dll";
		file_name = file_tools::get_hack_directory_path().append("data").append(file_name).string();

		main_window::progress_procentage = 0.f;

		main_window::status = "Downloading";

		file_tools::log_to_file("Trying to download");
		
		if (!download::download_to_file(h.get_download_url(), file_name, &main_window::progress_procentage) || should_cancel)
		{
			should_cancel = false;
			return false;
		}
		auto pid = inject::find_process_id(h.get_process_name());
		if (pid == NULL)
			main_window::status = "Waiting " + h.get_process_name();
		
		while (pid == NULL)
		{
			if (should_cancel)
			{
				should_cancel = false;
				return false;
			}
			
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			pid = inject::find_process_id(h.get_process_name());
		}

		if (should_cancel)
		{
			should_cancel = false;
			return false;
		}
		
		auto window_ready = inject::is_process_ready(pid);
		if (!window_ready)
			main_window::status = "Waiting for ready";
		while (!window_ready)
		{
			if (should_cancel)
			{
				should_cancel = false;
				return false;
			}
			
			std::this_thread::sleep_for(std::chrono::milliseconds(10000));
			window_ready = inject::is_process_ready(pid);
		}

		if (should_cancel)
		{
			should_cancel = false;
			return false;
		}
		
		main_window::status = "Injecting";
		
		const auto res = fn(h.get_process_name(), file_name);
		ShellExecute(NULL, 0, "RunDll32.exe", "InetCpl.cpl,ClearMyTracksByProcess 8", 0, SW_HIDE);
		main_window::status = res ? "Injected" : "Failed";
		is_loading = false;
		should_cancel = false;
		return res;
	}
}
