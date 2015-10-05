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

/*
int readPage(mstch::map& pageData, std::string filePath);
void urldecode2(char *dst, const char *src);
void formatGetData(StrMap &map, std::string query);
void formatCookieData(StrMap &map, std::string query);
*/
void logMessage(std::string message);

int main(int argc, char** argv) {
	int counter = 0;
	std::string templateString;// = "<html><body>title = {{ title }} <br> {{ query }} <br> {{{ content }}} <br> hit #{{ counter }}</body></html>";
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
//		mstch::map context;
		
		response.page_template(templateString);
		response.content_path(dataRoot + "index.md");
		
// ------------ get data
		response.get_data(std::string(FCGX_GetParam("QUERY_STRING", request.envp)));		
/*		std::string queryString = std::string(FCGX_GetParam("QUERY_STRING", request.envp));
		formatGetData(_GET, queryString);
*/		
// ------------ post data
		int postLen = std::atoi(FCGX_GetParam("CONTENT_LENGTH", request.envp));
		if (postLen) {
			std::string postData;
			postData.resize(postLen + 1);
			std::cin.read(&postData[0], postLen);
//			formatGetData(_POST, postData);
			response.post_data(postData);
		}

// ------------ cookie data
		if (FCGX_GetParam("HTTP_COOKIE", request.envp) != NULL) {
/*			std::string cookieData = FCGX_GetParam("HTTP_COOKIE", request.envp);
			formatCookieData(_COOKIE, cookieData);
*/			response.cookie_data(std::string(FCGX_GetParam("HTTP_COOKIE", request.envp)));
		}
/*
		int readError = readPage(context, "testFile.md");
		if (readError != 0) {
			switch(readError) {
			case (1):
				context.emplace("content", std::string("Error Opening target file<br>"));
				break;
			case (2):
				context.emplace("content", std::string("Error parsing target file<br>"));
				break;
			default:
				context.emplace("content", std::string("Error loading page<br>"));
				break;
			}
		}
*/
		
//		context.emplace("counter", counter);
		response.content_emplace("counter", std::to_string(counter));

//		std::cout << "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n";
//		std::cout << mstch::render(templateString, context) << std::endl;
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

/*

int readPage(mstch::map& pageData, std::string filePath) {
	markdown::Document mdData;
	std::ifstream mdFile;
	std::stringstream mdStream;
	std::string htmlString;
	std::string lineBuf;
	std::string fileBuf;
	std::string::size_type commentBegin;
	std::string::size_type commentEnd;
		
	mdFile.open(filePath.c_str());
	if (!mdFile) { 
		return 1;
	}
	
	std::getline(mdFile, lineBuf); 
	while (mdFile.good()) { // copy over mdFile
		fileBuf += "\n";
		fileBuf += lineBuf;		
		
		std::getline(mdFile, lineBuf); 
	}
	
	commentBegin = fileBuf.find("<!--", 0, 4);
	while (commentBegin != std::string::npos) {
		commentEnd = fileBuf.find("-->", 0, 3);
		if (commentEnd == std::string::npos)
			return 2;
		//check the comment for values to pass through
		lineBuf = fileBuf.substr(commentBegin + 4, commentEnd - commentBegin - 4);
		
		std::string::size_type eqMarker = lineBuf.find("=", 0, 1);
		while (eqMarker != std::string::npos) {
			static bool firstMatch = true; 
			std::string::size_type oStart = lineBuf.find_last_of(" \n\t", eqMarker); // find the word boundry of the option name
			std::string::size_type vEnd = lineBuf.find_first_of("\n", eqMarker);

			if (oStart != std::string::npos) { //looks like a valid option name
				std::string option = lineBuf.substr(oStart + 1, eqMarker - oStart - 1);
				std::string val = lineBuf.substr(eqMarker + 1, vEnd - eqMarker - 1);
				
				pageData.emplace(option, val);
			}
			if (vEnd == std::string::npos)
				break;
			eqMarker = lineBuf.find("=", vEnd, 1);
			firstMatch = false;
		}
		//clear out the comment
		fileBuf.replace(commentBegin, commentEnd - commentBegin + 3, "");
		commentBegin = fileBuf.find("<!--", commentBegin, 4);
	}
	
	
	//process the md file
	mdData.read(fileBuf);
	mdData.write(mdStream);
	
	//prep the data to be exposed to the view
	std::getline(mdStream, fileBuf); 
	while (mdStream.good()) {
		htmlString += "\n";
		htmlString += fileBuf;
		std::getline(mdStream, fileBuf); 
	}
	
	//expose the html version of the md file
	pageData.emplace("content", htmlString);
	
	return 0;
}

void formatGetData(StrMap &map, std::string query)
{
	if (query == "")
		return;

	std::string::size_type eqMarker = query.find("=");
	do {
		std::string::size_type pairMarker = query.find("&"); 
		std::string key = query.substr(0, eqMarker);
		std::string val = query.substr(eqMarker + 1, pairMarker - eqMarker - 1);
		
		char* keyFinal = new char[key.size() + 1];
		char* valFinal = new char[val.size() + 1];
		
		urldecode2(keyFinal, key.c_str());
		urldecode2(valFinal, val.c_str());

		map.emplace(keyFinal, valFinal);
		
		delete [] keyFinal;
		delete [] valFinal;
		
		if (pairMarker == std::string::npos)
			pairMarker = query.size() - 1;
		query.replace(0, pairMarker + 1, "");
		eqMarker = query.find("=");
	} while (eqMarker != std::string::npos);
}

void formatCookieData(StrMap &map, std::string query)
{
	if (query == "")
		return;

	std::string::size_type eqMarker = query.find("=");
	do {
		std::string::size_type pairMarker = query.find("; "); 
		std::string key = query.substr(0, eqMarker);
		std::string val = query.substr(eqMarker + 1, pairMarker - eqMarker - 1);
		
		char* keyFinal = new char[key.size() + 1];
		char* valFinal = new char[val.size() + 1];
		
		urldecode2(keyFinal, key.c_str());
		urldecode2(valFinal, val.c_str());

		map.emplace(keyFinal, valFinal);
		
		delete [] keyFinal;
		delete [] valFinal;
		
		if (pairMarker == std::string::npos)
			pairMarker = query.size() - 1;
		query.replace(0, pairMarker + 2, "");
		eqMarker = query.find("=");
	} while (eqMarker != std::string::npos);
}

// adapted from http://stackoverflow.com/a/14530993/3999005
void urldecode2(char *dst, const char *src)
{
        char a, b;
        while (*src) {
                if ((*src == '%') &&
                    ((a = src[1]) && (b = src[2])) &&
                    (isxdigit(a) && isxdigit(b))) {
                        if (a >= 'a')
                                a -= 'a'-'A';
                        if (a >= 'A')
                                a -= ('A' - 10);
                        else
                                a -= '0';
                        if (b >= 'a')
                                b -= 'a'-'A';
                        if (b >= 'A')
                                b -= ('A' - 10);
                        else
                                b -= '0';
                        *dst++ = 16*a+b;
                        src+=3;
				} else if (*src == '+') {
					*dst = ' ';
					dst++;
					src++;
                } else {
                        *dst++ = *src++;
                }
        }
        *dst++ = '\0';
}
*/