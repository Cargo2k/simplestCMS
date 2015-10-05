#include <fcgio.h>
//#include <stdlib.h>
#include <string>
#include <fstream>
#include <iostream>
#include <map>

#include <mstch/mstch.hpp>
#include "markdown.h"

#include "Compositor.h"

std::string webRoot = "/var/www/html";
std::string dataRoot = "/var/www/html-data";
std::string msgLogPath = "/var/log/simplestCMS/message.log";

void logMessage(std::string message);

int main(int argc, char** argv) {
	int counter = 0;
	std::string templateString;
	std::streambuf* defaultCout = std::cout.rdbuf();
	std::streambuf* defaultCin = std::cin.rdbuf();
	std::ifstream templateFile;
	
	FCGX_Request request;
	FCGX_Init();
	FCGX_InitRequest(&request, 0, 0);
	
	logMessage(" ------ new run ------");
	
	templateFile.open(dataRoot + "/themes/default/template.mstch");
	if (!templateFile) {
		std::cerr << "Could not open the site template" << std::endl;
		return 1;
	}
	
	while (templateFile.good()) {
		std::string lineBuf;
		std::getline(templateFile, lineBuf);
		templateString += lineBuf;
		templateString += "\n";
	}
	
	while (FCGX_Accept_r(&request) == 0) {
		Compositor response;
		StrMap _GET;
		StrMap _POST;
		StrMap _COOKIE;
		fcgi_streambuf fcgiCout(request.out);
		fcgi_streambuf fcgiCin(request.in);
		std::cout.rdbuf(&fcgiCout);
		std::cin.rdbuf(&fcgiCin);
		
		response.page_template(templateString);
		response.content_path(dataRoot + "/index.md");
		
// ------------ get data
		response.get_data(std::string(FCGX_GetParam("QUERY_STRING", request.envp)));
// ------------ post data
		int postLen = std::atoi(FCGX_GetParam("CONTENT_LENGTH", request.envp));
		if (postLen) {
			std::string postData;
			postData.resize(postLen + 1);
			std::cin.read(&postData[0], postLen);
			response.post_data(postData);
		}

// ------------ cookie data
		if (FCGX_GetParam("HTTP_COOKIE", request.envp) != NULL) {
			response.cookie_data(std::string(FCGX_GetParam("HTTP_COOKIE", request.envp)));
		}

		response.content_emplace("counter", std::to_string(counter));
		
		std::cout << response.response();
		counter++;
		FCGX_Finish_r(&request);
	}
	
	std::cin.rdbuf(defaultCin);
	std::cout.rdbuf(defaultCout);
	return 0;
}

void logMessage(std::string message) {
	std::ofstream msgLog;
	
	msgLog.open(msgLogPath.c_str(), std::ofstream::app);
	
	if (msgLog.good())
		msgLog << message << std::endl;
		
	msgLog.close();
}