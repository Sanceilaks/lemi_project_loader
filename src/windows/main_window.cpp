#include "main_window.h"


#include <map>
#include <thread>



#include "../hack_list.h"
#include "../fonts.h"
#include "../hack_list.h"
#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"
#include "../imgui/im_tools.h"

#include "../globals.h"
#include "../loader_core.h"

using namespace ImGui;

void bottom_button(const std::string& str);
ImVec2 calc_bottom_button_size(const std::string& str);

std::map<std::string, std::string> infos;

void main_window::draw_main_window()
{
	SetNextWindowSize(GetIO().DisplaySize, ImGuiCond_Always);
	SetNextWindowPos({ 0, 0 }, ImGuiCond_Always);
	
	Begin("##MAIN_WINDOW", static_cast<bool*>(nullptr), ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollWithMouse);


	if (BeginListBox("##HACK_LIST_BOX", { GetWindowSize().x / 2.f, GetWindowSize().y - GetStyle().WindowPadding.y - GetStyle().ItemSpacing.y}))
	{	
		for (auto i : hack_list::get_hacks())
		{
			if (Selectable((i.get_name() + "##HACK_LIST_BOX").c_str(), hack_list::get_current_hack() == i))
				hack_list::set_current_hack(i);
		}
		EndListBox();
	}
	auto hack_list_size = GetItemRectSize();

	SameLine();

	BeginGroup();
	{

		/*Text("name: %s\nversion: %s\nprocess: %s\nstatus: %s", hack_list::get_current_hack().get_name().c_str(),
			 hack_list::get_current_hack().get_version().c_str(),
		     hack_list::get_current_hack().get_process_name().c_str(),
		     status.c_str());*/
		BeginGroup();
		
		auto name = hack_list::get_current_hack().get_name();

		PushFont(fonts::ubuntu_big);
		{
			SetCursorPosX(
				hack_list_size.x + GetStyle().FramePadding.x - GetStyle().WindowPadding.x + GetContentRegionAvail().x /
				2.f - CalcTextSize(name.c_str()).x / 2.f);
			TextColored({ 0, 0.6f, 0.4f, 1 }, name.c_str());			
		}
		PopFont();
	
		Separator();

		EndGroup();
		
		PushFont(fonts::ubuntu_small);
			auto version_size = CalcTextSize(hack_list::get_current_hack().get_version().c_str());
			GetWindowDrawList()->AddText(
				{ GetWindowSize().x - GetStyle().WindowPadding.x - version_size.x, GetItemRectMax().y + 2.f},
				GetColorU32(ImGuiCol_Text), hack_list::get_current_hack().get_version().c_str());
		PopFont();

		Dummy({ GetContentRegionAvail().x, version_size.y });

		//BeginChild("hinfo", {GetContentRegionAvail().x - calc_bottom_button_size("Load##LOAD_HACK").x, 0});
		//	if (infos.find(hack_list::get_current_hack().get_name()) == infos.end())
		//	{
		//		std::string info;
		//		if (download::download_to_memory(hack_list::get_current_hack().get_devlog_url(), info, nullptr))
		//			infos.emplace(hack_list::get_current_hack().get_name(), info);
		//	}
		//	if (infos.find(hack_list::get_current_hack().get_name()) != infos.end())
		//		Text(infos[hack_list::get_current_hack().get_name()].c_str());
		//
		//EndChild();
		
		bottom_button("Load##LOAD_HACK");
	}
	EndGroup();

	End();

	if (loader_core::is_loading)
	{
		SetNextWindowSize(GetIO().DisplaySize, ImGuiCond_Always);
		SetNextWindowPos({ 0, 0 }, ImGuiCond_Always);
		Begin("##LOADING_POPUP_WINDOW", static_cast<bool*>(nullptr), ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollWithMouse);

		auto radius = GetWindowSize().y / 4.f;
		
		SetCursorPos({ GetWindowSize().x / 2.f - radius, GetWindowSize().y / 2.f - radius * 1.2f });

		LoadingIndicatorCircle("##LOADING_INDICATOR", radius, GetStyle().Colors[ImGuiCol_Text],
			GetStyle().Colors[ImGuiCol_Button], 8, 4.f);

		Dummy({GetWindowSize().x, radius / 3.8f});
		
		auto text_size = CalcTextSize(status.c_str());
		SetCursorPosX(GetWindowSize().x / 2.f - GetStyle().WindowPadding.x / 2.f - text_size.x / 2.f);
		SetCursorPosY(GetCursorPosY() + text_size.y / 2.f);
		TextColored({ 0, 0.6f, 0.4f, 1 }, status.c_str());
		
		if (CenterButton("Cancel##CANCEL_BUTTON"))
			loader_core::should_cancel = true;
		
		End();
	}
}

void load_hack()
{
	if (!loader_core::load(hack_list::get_current_hack()))
	{
		loader_core::is_loading = false;
		loader_core::should_cancel = false;
		main_window::status = "";
	}
}

void bottom_button(const std::string& str)
{
	PushFont(fonts::ubuntu_big);
	
	auto button_size = ImVec2{ GetContentRegionAvail().x, 0 };
	
	auto style = GetStyle();
	auto label_size = CalcTextSize(str.c_str(), 0, true);

	ImVec2 size = CalcItemSize(button_size, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

	auto window_size = GetIO().DisplaySize;
	
	SetCursorPosY(window_size.y - size.y - style.WindowPadding.y);
	if (Button(str.c_str(), button_size) && !loader_core::is_loading)
		std::thread(load_hack).detach();

	button_size = GetItemRectSize();

	PopFont();
}

ImVec2 calc_bottom_button_size(const std::string& str)
{
	PushFont(fonts::ubuntu_big);
	auto button_size = ImVec2{ GetContentRegionAvail().x, 0 };
	auto style = GetStyle();
	auto label_size = CalcTextSize(str.c_str(), 0, true);
	ImVec2 size = CalcItemSize(button_size, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);
	PopFont();
	return size;
}
