#include "Compositor.h"

void log_message(std::string message); // defined in main.cpp

Compositor::Compositor() {}

/**
 * returns the complete response for the request
 */
std::string Compositor::response(void) {
	return this->http_headers() + this->renderedPage;
}

/**
 * returns the headers for the requested
 * TODO: should include cookie data
 */ 
std::string Compositor::http_headers(void) {
	return "HTTP/1.1\r\n" + this->statusCode + "\r\nContent-Type: " + this->mimetype +"\r\n\r\n";
}

std::string Compositor::template_path(std::string path) {
	this->read_file(path + "/template.mstch", this->pageTemplate);
	this->pageTemplate;
	
	return this->pageTemplate;
}

std::string Compositor::page_template(void) { return this->pageTemplate; }

void Compositor::content_request(std::string request) {
	std::string::size_type extPos;
	std::string ext;
	std::string baseFile;
	uint extGroup = 0;
	
	// setup our extention mapping
	const std::vector<Renderable> extMap = {
		{},
		{"html", "md"},
		{"htm", "md"}
	};
	
	if (request[0] != '/')
		request.insert(0, "/");
	
	// check for file ext
	extPos = request.rfind(".");

	if (extPos == std::string::npos) { // no extention, check to see if is a directory
		// check dir
			// check for index 
				// do file map
	}

	// map file to pre render format
	baseFile = request.substr(0, extPos);
	ext = request.substr(extPos + 1);
	for (uint i = 0; i < extMap.size(); ++i) {
		if (extMap[i].to == ext) {
			extGroup = i;
		}
		if (extGroup) { break; }
	}
	// if we have a render group check both files
	if (extGroup) {
		// get the file details
		FileStat rendered = (FileStat(this->webRoot + baseFile + "." + extMap[extGroup].to));
		FileStat data = (FileStat(this->dataRoot + baseFile + "." + extMap[extGroup].from));
		log_message("Testing for file: " + rendered.path);
		log_message("Testing for file: " + data.path);
		this->file_check(rendered);
		this->file_check(data);
		// check change times
		if (rendered.modified && rendered.modified >= data.modified) {
			// serve the exisiting
			log_message("serving " + rendered.path);
			this->serve_existing(rendered.path);
		} else if (data.modified) {
			// render and serve
			std::string render;
			this->read_file(data.path, render);
			// TODO: make this easier to extend render types, adding scss atleast would be nice
			// TODO: how could i account for a context to render mstch also? 
			render = this->render_md(render);
			log_message("serving: " + data.path);
			if (render != "") {
				this->context_emplace("content", render);
				this->renderedPage = this->render_mstch(this->pageTemplate, this->pageContext);
				this->write_file(rendered.path, this->renderedPage);
				this->serve_existing(rendered.path, &this->renderedPage);
			}// else TODO: there was prolly an error in the syntax of the md file
		}
		// if we get here neither file should exist
		// just returning should force a 404 due to default values
	} else {
		// no ext group check for an existing file		
		FileStat rendered(this->webRoot + baseFile);
		if (this->file_check(rendered)) {
			this->serve_existing(rendered.path);
		} // else file should not exist, return to force 404
	}
}

/**
 * Check for the existance of a file
 *
 * @param path the file to check for
 * @returns the last modified date, or 0 if the file does not exist
 */
bool Compositor::file_check(FileStat& target) {
	struct stat statRes;
	if (stat(target.path.c_str(), &statRes) != 0) {
		target.modified = 0;
		target.directory = false;
		log_message("Could not stat: " + target.path);
		switch (errno) { // stat failure 
		case (EACCES):
			log_message("Permissions Error accessing: " + target.path);
			break;
		}
		return false;
	}
	if (S_ISDIR(statRes.st_mode)) {
		target.directory = true;
	} else {
		target.directory = false;
	}
	target.modified = statRes.st_mtime;
	return true;
}

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
	for (auto& getField : this->get) {
		if (getField.first == "request") {
			log_message("Got request = " + getField.second);
			this->content_request(getField.second);
			break;
		}
	}

	return this->get;
}

StrMap Compositor::get_data(void) { return this->get; }

std::string Compositor::content_path(std::string path) {
	this->contentPath = path;
	return this->contentPath;
}

std::string Compositor::content_path() {
	return this->contentPath;
}

std::string Compositor::data_path(std::string path) {
	this->dataRoot = path;
	return this->dataRoot;
}

std::string Compositor::web_path(void) { return this->dataRoot; }

std::string Compositor::web_path(std::string path) {
	this->webRoot = path;
	return this->webRoot;
}

std::string Compositor::data_path(void) { return this->dataRoot; }

/** renders an html string from a string containing mstch template and a mstch::map containing a variable context
 *
 * @param rawString a string containting an mstch template
 * @param contextMap the context to render in the template
 * @returns an html string
 */
std::string Compositor::render_mstch(std::string rawString, mstch::map contextMap) {
	return mstch::render(rawString, contextMap);
}

/**
 * Extracts context vairables, page scalars, and page arrays from templates
 *
 * @param tmplString a string representing a template
 */
void Compositor::extract_page_modifiers(std::string& tmplString) {
	std::string lineBuf;
	std::string::size_type commentBegin;
	std::string::size_type commentEnd;

// look for comments in the string
	commentBegin = tmplString.find("<!--", 0, 4);
	while (commentBegin != std::string::npos) {
		commentEnd = tmplString.find("-->", 0, 3);
		if (commentEnd == std::string::npos) {
			// TODO: file format errors
			// remove the comment begining that we didn't find an end for
			tmplString.replace(commentBegin, 4, "");
			std::string::size_type msgBegin = commentBegin - 10;
			std::string::size_type msgEnd = commentBegin + 10;
			if (msgBegin < 0) msgBegin = 0;
			if (msgEnd > tmplString.size()) msgEnd = tmplString.size() - 1; 
			log_message("Could not match comment tag near: " + tmplString.substr(msgBegin, msgEnd));
		} else {
			//check the comment for context values to pass through
			lineBuf = tmplString.substr(commentBegin + 4, commentEnd - commentBegin - 4);
			
			std::string::size_type eqMarker = lineBuf.find("=", 0, 1);
			while (eqMarker != std::string::npos) {
				std::string::size_type oStart = lineBuf.find_last_of(" \n\t", eqMarker); // find the word boundry of the option name
				std::string::size_type vEnd = lineBuf.find_first_of("\n", eqMarker); // find the end of the opion line
	
				if (oStart != std::string::npos) { //looks like a valid option name
					std::string option = lineBuf.substr(oStart + 1, eqMarker - oStart - 1);
					std::string val = lineBuf.substr(eqMarker + 1, vEnd - eqMarker - 1);
					if(option[0] == '@') {
						//TODO: push back an array value
					} else if(option[0] == '$') {
						//TODO: push back a scalar value
					} else {
						this->pageContext.emplace(option, val);
					}
				}
				if (vEnd == std::string::npos)
					break;
				eqMarker = lineBuf.find("=", vEnd, 1);
			}
			//clear out the comment
			tmplString.replace(commentBegin, commentEnd - commentBegin + 3, "");
		}
		// check for another comment
		commentBegin = tmplString.find("<!--", commentBegin, 4);
	}
}


/**
 * converts a string from md to html, extracts html style comments, and checks the comments for values formatted to be passed to the context
 *
 * @param path a file path in relation to the OS
 */
std::string Compositor::render_md(std::string rawString) {	
	markdown::Document mdData;
	std::stringstream mdStream;
	std::string retVal;
	std::string lineBuf;
	
	this->extract_page_modifiers(rawString);
	log_message("rawString <<< EOL\n" + rawString + "\nEOL");
	
	//process the md string
	mdData.read(rawString);
	mdData.write(mdStream);
	
	//prep the data to be exposed to the view
	std::getline(mdStream, lineBuf); 
	while (mdStream.good()) {
		retVal += lineBuf;
		retVal += "\n";
		std::getline(mdStream, lineBuf); 
	}
	
	//expose the html version of the md file
//	this->pageContext.emplace("content", retVal);

	return retVal;
}

void Compositor::serve_existing(std::string path, std::string* existingPage) {
// look for an extention to set the mime type off of
	std::string ext;
	std::string::size_type pos = path.rfind(".");

// set the mime type
	if (pos == std::string::npos) {
		this->mimetype = this->guess_mimetype(path);
	} else {
		ext = path.substr(pos + 1, path.size() - pos);
		if (ext == "js")
			this->mimetype = "text/javascript";
		else if (ext == "css")
			this->mimetype = "text/css";
		else if (ext == "html")
			this->mimetype = "text/html";
		else if (ext == "htm")
			this->mimetype = "text/html";
		else 
			this->mimetype = this->guess_mimetype(path);
	}
	
	this->statusCode = "200 OK";
	this->content_path(path);
	if (existingPage == NULL) {
		log_message("Reading in existing: " + path);
		this->read_file(path, this->renderedPage);
	} else {
		this->renderedPage = *existingPage;
	}
}

/** uses the cli program file to guess the mime type
 *
 * this is a potentialy awful way, but should do for now. file is on most every POSIX based system
 *
 * @param path the path of the file to test
 */
std::string Compositor::guess_mimetype(std::string path) {
	std::string cmd = "file -b --mime-type " + path;
	FILE* pipe = popen(cmd.c_str(), "r");
	if (!pipe) return "application/octet-stream";
	char buffer[128];
	std::string result = "";
	while (!feof(pipe)) {
		if (fgets(buffer, 128, pipe) != NULL)
			result += buffer;
		}
	pclose(pipe);

	return result;
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

bool Compositor::read_file(std::string path, std::string& dst) {
	std::ifstream inFile;
	std::string lineBuf;
	dst = "";
	 
	inFile.open(path);
	if (!inFile) {
		// TODO: file access error 
		log_message("File access error: " + path);
		return false;
	}

	std::getline(inFile, lineBuf);
	while (inFile.good()) { // copy over mdFile
		dst += "\n";
		dst += lineBuf;

		std::getline(inFile, lineBuf);
	}
	inFile.close();
	return true;
}

bool Compositor::write_file(std::string path, std::string src) {
	std::ofstream oFile;
	oFile.open(path, std::ofstream::out | std::ofstream::trunc);
	if (oFile.is_open()) {
		oFile << src;
	} else {
		log_message("failed to write: " + path);
		return false;
	}
 
	oFile.close();
	return true;
}


void Compositor::context_emplace(std::string key, std::string val) {
	this->pageContext.emplace(key, val);
}
