#include "main_window.h"

#include <thread>



#include "../hack_list.h"
#include "../fonts.h"
#include "../hack_list.h"
#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"

#include "../globals.h"
#include "../loader_core.h"

using namespace ImGui;

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
	auto sz = GetItemRectSize();

	SameLine();

	BeginGroup();
	{

		Text("name: %s\nversion: %s\nprocess: %s\nstatus: %s", hack_list::get_current_hack().get_name().c_str(),
			 hack_list::get_current_hack().get_version().c_str(),
		     hack_list::get_current_hack().get_process_name().c_str(),
		     status.c_str());

		ProgressBar(progress_procentage, ImVec2(-FLT_MIN, 0), "");
		sz = GetItemRectSize();
		
		if (Button("Load##LOAD_HACK", { sz.x, sz.y }) && !loader_core::is_loading)
			std::thread(loader_core::load, hack_list::get_current_hack()).detach();
	}
	EndGroup();
	
	End();
}
