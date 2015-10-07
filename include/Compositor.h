#ifndef COMPOSITOR_H_KC
#define COMPOSITOR_H_KC

#include <string>
#include <fstream>
#include <map>
#include <vector>
#include <algorithm>

#include <sys/stat.h>
#include <dirent.h>
#include <cerrno>

#include <mstch/mstch.hpp>
#include "markdown.h"

typedef std::map<std::string, std::string> StrMap;
typedef unsigned int uint;

// a struct to hold file data
struct FileStat {
	time_t modified;
	bool directory;
	std::string path;
	
	FileStat(void) {};
	FileStat(std::string pathIn):path(pathIn) {};
};

struct Renderable {
	std::string to;
	std::string from;
};

class Compositor {
public:
	Compositor();
	
	std::string response(void);
	std::string http_headers(void);

	std::string template_path(std::string path);
	std::string page_template(void);
	std::string content_path(std::string path);
	std::string content_path();
	std::string data_path(std::string path);
	std::string data_path(void);
	std::string web_path(std::string path);
	std::string web_path(void);
	void content_request(std::string request);
	
	void context_emplace(std::string key, std::string val);

	StrMap cookie_data(std::string cookieData);
	StrMap cookie_data(void);
	StrMap post_data(std::string postData);
	StrMap post_data(void);
	StrMap get_data(std::string getData);
	StrMap get_data(void);
	
// pathes who's usage can change, just going to access them directly for now
	std::string webRoot = "/var/www/html";
	std::string dataRoot = "/var/www/html-data"; 
protected:
// page and content data
	std::string pageTemplate; // the site template
	std::string renderedPage; // the finalized content, no headers
	std::string contentPath; // file path of the servable content
	
	bool notFound = false;
	
// header data
	std::string mimetype = "text/html";
	std::string statusCode = "Status: 404 Not Found"; // set as a default so Compositor::content_request just returns if a file is not found
	
// enviroment data
	StrMap cookie;
	StrMap post;
	StrMap get;

void serve_existing(std::string path, std::string* existingPage = NULL);

// page context (data exposed to the view)
	mstch::map pageContext;

// tools to render file types
	std::string render_md(std::string rawString);
	std::string render_mstch(std::string rawString, mstch::map contextMap);

// tools to manage file types
	std::string guess_mimetype(std::string path);
	bool file_check(FileStat& target);
	
// tools to manage fcgi input data
	void urldecode2(char *dst, const char *src);
	void formatGetData(StrMap &map, std::string query);
	void formatCookieData(StrMap &map, std::string query);
	
// utility functions
	bool read_file(std::string path, std::string& dst);
	bool write_file(std::string path, std::string src);
};

#endif // COMPOSITOR_H_KC