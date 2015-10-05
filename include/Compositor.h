#ifndef COMPOSITOR_H_KC
#define COMPOSITOR_H_KC

#include <string>
#include <fstream>
#include <map>

#include <mstch/mstch.hpp>
#include "markdown.h"

typedef std::map<std::string, std::string> StrMap;

class Compositor {
public:
	Compositor();
	
	std::string response(void);
	std::string http_headers(void);
	std::string reply_content(void);

	std::string page_template(std::string path);
	std::string page_template(void);
	std::string content_path(std::string path);
	void content_emplace(std::string key, std::string val);
	
	std::string page_content(void);

	StrMap cookie_data(std::string cookieData);
	StrMap cookie_data(void);
	StrMap post_data(std::string postData);
	StrMap post_data(void);
	StrMap get_data(std::string getData);
	StrMap get_data(void);
protected:
	std::string body;
	std::string pageTemplate;
	std::string pageContent;
	std::string renderedPage;
	std::string contentPath;
	
	StrMap cookie;
	StrMap post;
	StrMap get;
	
	mstch::map pageContext;
	
	void read_page(std::string filePath);
	void urldecode2(char *dst, const char *src);
	void formatGetData(StrMap &map, std::string query);
	void formatCookieData(StrMap &map, std::string query);
};

#endif // COMPOSITOR_H_KC