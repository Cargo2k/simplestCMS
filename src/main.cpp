#include <fcgio.h>

#include <string>
#include <fstream>
#include <iostream>
#include <map>

#include <mstch/mstch.hpp>
#include <libconfig.h++>

#include "markdown.h"
#include "Compositor.h"
#include "option.h"

std::string msgLogPath = "/var/log/simplestCMS/message.log";
std::string errorLogPath = "/var/log/simplestCMS/error.log";
int logLevel = 3;

void log_message(std::string message);

int main(int argc, char** argv) {
	int counter = 0;
	std::streambuf* defaultCout = std::cout.rdbuf();
	std::streambuf* defaultCin = std::cin.rdbuf();
	HostSetting defaultHost;
	libconfig::Config cfg;
	
	FCGX_Request request;
	FCGX_Init();
	FCGX_InitRequest(&request, 0, 0);
	
	if (!load_config(cfg))
		return -1;

	cfg.lookupValue("log.error", errorLogPath);
	cfg.lookupValue("log.message", msgLogPath);
	cfg.lookupValue("log.level", logLevel);
	
	std::cout << msgLogPath << std::endl;

	cfg.lookupValue("hosts.default.dataPath", defaultHost.dataPath);
	cfg.lookupValue("hosts.default.htmlPath", defaultHost.htmlPath);
	cfg.lookupValue("hosts.default.theme.dir", defaultHost.themePath);
	cfg.lookupValue("hosts.default.theme.name", defaultHost.themeName);
	
	log_message(" ------ new run ------");
	log_message("Configured as:");
	log_message("log.error: " + errorLogPath);
	log_message("log.message: " + msgLogPath);
	log_message("log.level: " + logLevel);
	
	log_message("hosts.default.dataPath: " + defaultHost.dataPath);
	log_message("hosts.default.htmlPath: " + defaultHost.htmlPath);
	log_message("hosts.default.theme.dir: " + defaultHost.themePath);
	log_message("hosts.default.theme.name: " + defaultHost.themeName);
	
// wiat for and accept requests
	while (FCGX_Accept_r(&request) == 0) {
		Compositor response;
		StrMap _GET;
		StrMap _POST;
		StrMap _COOKIE;
		fcgi_streambuf fcgiCout(request.out);
		fcgi_streambuf fcgiCin(request.in);
		std::cout.rdbuf(&fcgiCout);
		std::cin.rdbuf(&fcgiCin);
		
		response.context_emplace("counter", std::to_string(counter));
		response.template_path(defaultHost.themePath + "/" + defaultHost.themeName);
		response.data_path(defaultHost.dataPath);
		response.web_path(defaultHost.htmlPath);
		
// ------------ get data
		if (argc > 1) {
			log_message("Sending request = " + std::string(argv[1]));
			response.content_request(argv[1]);
		} else {
			response.get_data(std::string(FCGX_GetParam("QUERY_STRING", request.envp)));
		}
// ------------ post data
		log_message("Sending Post data");
		if (FCGX_GetParam("CONTENT_LENGTH", request.envp)) {
			int postLen = std::atoi(FCGX_GetParam("CONTENT_LENGTH", request.envp));
			std::string postData;
			postData.resize(postLen + 1);
			std::cin.read(&postData[0], postLen);
			response.post_data(postData);
		}

// ------------ cookie data
		log_message("Sending Cookie data");
		if (FCGX_GetParam("HTTP_COOKIE", request.envp) != NULL) {
			response.cookie_data(std::string(FCGX_GetParam("HTTP_COOKIE", request.envp)));
		}

		std::cout << response.response();
		log_message(response.response());
		counter++;

		FCGX_Finish_r(&request);
	}

	std::cin.rdbuf(defaultCin);
	std::cout.rdbuf(defaultCout);
	return 0;
}

void log_message(std::string message) {
	std::ofstream msgLog;
	
	msgLog.open(msgLogPath.c_str(), std::ofstream::app);
	
	if (msgLog.good())
		msgLog << message << std::endl;
		
	msgLog.close();
}
