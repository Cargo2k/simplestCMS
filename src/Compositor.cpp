#include <Compositor.h>

void logMessage(std::string message);

Compositor::Compositor() {}

/**
 * returns the complete response for the request
 */
std::string Compositor::response(void) {
	return this->http_headers() + this->reply_content(); 
}

/**
 * returns the headers for the requested
 * TODO: should change if 404
 * TODO: should include cookie data
 */ 
std::string Compositor::http_headers(void) {
	return "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n";
}

/**
 * aggrigates the template and the page contents giving the HTML string of the page
 *
 * @returns an html string of the page requested
 */
std::string Compositor::reply_content(void) {
	this->renderedPage = mstch::render(this->pageTemplate,
									   this->pageContext);
	return this->renderedPage;
}

std::string Compositor::page_template(std::string pageTemplate) {
	this->pageTemplate = pageTemplate;
	
	return this->pageTemplate;
}

std::string Compositor::page_template(void) { return this->pageTemplate; }

std::string Compositor::content_path(std::string path) {
	this->contentPath = path;
	this->read_page(contentPath);
	
	return this->contentPath;
}

void Compositor::content_emplace(std::string key, std::string val) {
	this->pageContext.emplace(key, val);
}

std::string Compositor::page_content(void) { return this->pageContent; }

StrMap Compositor::cookie_data(std::string cookieData) {
	this->formatCookieData(this->cookie, cookieData);
	
	return this->cookie;
}
StrMap Compositor::cookie_data(void) { return this->cookie; }

StrMap Compositor::post_data(std::string postData) {
	this->formatGetData(this->post, postData);
	
	return this->post;
}

StrMap Compositor::post_data(void) { return this->post; }

StrMap Compositor::get_data(std::string getData) {
	formatGetData(this->get, getData);
	
	return this->get;
}

StrMap Compositor::get_data(void) { return this->get; }

/**
 * reads in a file, and processes it thourgh cpp-markdown
 *
 * @param filePath the path of the file to load
 */
void Compositor::read_page(std::string filePath) {
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
		// TODO: file access error 
		logMessage("File access error: " + filePath);
		return;
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
		if (commentEnd == std::string::npos) {
			// TODO: file format errors
			return;
		}
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
				this->pageContext.emplace(option, val);
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
	std::getline(mdStream, lineBuf); 
	while (mdStream.good()) {
		htmlString += "\n";
		htmlString += lineBuf;
		std::getline(mdStream, lineBuf); 
	}
	
	//expose the html version of the md file
//	pageData.emplace("content", htmlString);
	this->pageContent = htmlString;
	this->pageContext.emplace("content", htmlString);
}

/**
 * formats URL encode GET and Post strings into a StrMap
 *
 * @param map the out StrMap
 * @param query the URL encoded string
 */
void Compositor::formatGetData(StrMap &map, std::string query) {
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

/**
 * Formats raw HTTP_COOKIE data and stores it in map
 *
 * @param map the where to store the output
 * @param query the HTTP_COOKIE string
 */
void Compositor::formatCookieData(StrMap &map, std::string query) {
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

/** 
 * Decodes URL encoding
 * 
 * adapted from http://stackoverflow.com/a/14530993/3999005
 * the dst buffer should be the length of src + 1
 *
 * @param dst the alocated buffer to copy to
 * @param the encoded URL
 */ 
void Compositor::urldecode2(char *dst, const char *src) {
	char a, b;
	while (*src) {
		if ((*src == '%') && ((a = src[1]) && (b = src[2])) && (isxdigit(a) && isxdigit(b))) {
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
