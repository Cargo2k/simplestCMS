#include "option.h"

#ifndef CFGDIR
#define CFGDIR "/etc/"
#endif // CFGDIR

#ifndef BINNAME
#define BINNAME "simplestCMS"
#endif // BINNAME

void log_message(std::string message);

bool load_config(libconfig::Config& cfg) {
	std::string cfgPath = std::string(CFGDIR) + "/" + std::string(BINNAME) + ".conf";
	try { 
		cfg.readFile(cfgPath.c_str());
	} catch (const libconfig::FileIOException& e) {
		std::cerr << "Error: Could not read config file: " << cfgPath << std::endl;
		return false;
	} catch (const libconfig::ParseException& e) {
		std::cerr << "Parser Error: " << e.getFile() << " line: " << e.getLine() << std::endl;
		return false;
	}
	
	return true;
}