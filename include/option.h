#ifndef OPTION_H_KC
#define OPTION_H_KC

#include <iostream>
#include <string>
#include <libconfig.h++>

struct HostSetting {
	std::string dataPath;
	std::string htmlPath;
	std::string themePath;
	std::string themeName;
};

bool load_config(libconfig::Config& cfg);

#endif // OPTION_H_KC
