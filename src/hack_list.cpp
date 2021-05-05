#include "hack_list.h"

#include <string>
#include <sstream>

#include <tinyxml2.h>

#include <vector>

#include "download.h"

std::string uri;
std::vector<c_base_hack> hacks;
uint32_t current_hack;

bool c_base_hack::operator==(const c_base_hack& h) const
{
	return h.devlog_url == devlog_url && h.inject_method == inject_method && h.download_url == download_url && h.name ==
		name && h.process_name == process_name && h.version == version;
}


void hack_list::init_hacks(const std::string& _uri)
{
	uri = _uri;
	
	tinyxml2::XMLDocument doc;
	doc.Parse(get_url_content().c_str());

	auto* hacks_array = doc.FirstChildElement("ArrayOfHack");
	auto* element = hacks_array->FirstChildElement("Hack");

	std::vector<c_base_hack> out;

	while (element)
	{
		const auto inject_method = element->FirstChildElement("Method")->IntText();
		const auto* name = element->FirstChildElement("Name")->GetText();
		const auto* download_url = element->FirstChildElement("Url")->GetText();
		const auto* devlog_url = element->FirstChildElement("DevlogUrl")->GetText();
		const auto* game_process = element->FirstChildElement("GameProcess")->GetText();
		const auto* version = element->FirstChildElement("Version")->GetText();

		out.emplace_back(inject_method, name, download_url, devlog_url, game_process, version);
		element = element->NextSiblingElement("Hack");
	}

	out.swap(hacks);
}

std::vector<c_base_hack> hack_list::get_hacks()
{
	return hacks;
}

std::string hack_list::get_url_content()
{
	std::string s;
	download::download_to_memory(uri, s, nullptr);
	return s;
}

c_base_hack hack_list::get_current_hack()
{
	return hacks.at(current_hack);
}

uint32_t hack_list::get_current_hack_index()
{
	return current_hack;
}

void hack_list::set_current_hack(const c_base_hack& hack)
{
	current_hack = std::distance(hacks.begin(), std::find(hacks.begin(), hacks.end(), hack));
}

void hack_list::set_current_hack(uint32_t idx)
{
	current_hack = idx;
}

c_base_hack hack_list::get_hack_by_name(const std::string& name)
{
	return *std::find_if(hacks.begin(), hacks.end(), [&](const c_base_hack& h){return h.get_name() == name; });
}