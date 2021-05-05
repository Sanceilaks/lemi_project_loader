#pragma once
#include <memory>
#include <string>
#include <vector>

class c_base_hack
{
	int inject_method;
	std::string name;
	std::string download_url;
	std::string devlog_url;
	std::string process_name;
	std::string version;

public:
	c_base_hack() {}
	
	c_base_hack(int im, const std::string& name, const std::string& download_url, const std::string& devlog_url, const std::string& process_name, const std::string& version) :
		inject_method(im), name(name), download_url(download_url), devlog_url(devlog_url), process_name(process_name),
		version(version) {}

	
	[[nodiscard]] int get_inject_method() const { return inject_method; }
	[[nodiscard]] std::string get_name() const { return name; }
	[[nodiscard]] std::string get_download_url() const { return download_url; }
	[[nodiscard]] std::string get_devlog_url() const { return devlog_url; }
	[[nodiscard]] std::string get_process_name() const { return process_name; }
	[[nodiscard]] std::string get_version() const { return version; }

	bool operator==(const c_base_hack& h) const;
};


namespace hack_list
{
	void init_hacks(const std::string& _uri = "https://raw.githubusercontent.com/Sanceilaks/LemiProject/main/meta.xml");

	std::vector<c_base_hack> get_hacks();
	[[nodiscard]] std::string get_url_content();

	c_base_hack get_current_hack();
	[[nodiscard]] uint32_t get_current_hack_index();

	void set_current_hack(const c_base_hack& hack);
	void set_current_hack(uint32_t idx);

	c_base_hack get_hack_by_name(const std::string& name);
}
