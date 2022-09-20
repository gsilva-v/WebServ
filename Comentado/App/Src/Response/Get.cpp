#include "Response.hpp"

/**
 * If the request is a CGI request, handle it; if the request is a request for a file, read the file
 * and make the response; if the request is a request for a directory, make an autoindex; if the
 * request is a request for a file that doesn't exist, make a 404 response
 * 
 * @return The response header and body.
 */
void Response::responseGet(){
	body.clear();
	if (request->getCgiRequest()){
		if (!fileExist(conf.root + request->getScriptPath())){
			status_code = STATUS_NOT_FOUND;
			makeHeader();
		}
		else
			handleCgi();
		return ;
	}
	if ((!folderExist(path) && request->getUrl().find("/upload") != npos) || \
		request->getUrl().find("favicon.ico") != npos){
			if (request->getUrl().find("favicon.ico") != npos)
				path =  conf.root + request->getUrl();
			else
				path =  "./www" + request->getUrl();
			if (!folderExist(path) && !fileExist(path))
				status_code = STATUS_NOT_FOUND;
			else
				makeImage();
	} else if (autoindex)
		handleAutoindex(path);
	else if (redirection)
		status_code = STATUS_REDIRECT;
	else
		readHTML(path);
	if (bodySize > conf.client_max_body_size)
		status_code = STATUS_TOO_LARGE;
	makeHeader();
};

/**
 * It reads the file at the given path and stores it in the body of the response
 * 
 * @param path the path to the file to be read
 * 
 * @return The response object is being returned.
 */
void Response::readHTML(boost::string path){
	boost::string line;
	std::fstream file;
	file.open(path.c_str(), std::ios::in);
	if (!file.good()){
		status_code = STATUS_NOT_FOUND;
		return ;
	}
	status_code = STATUS_OK;
	while (getline(file, line))	{
		body << line << '\n';		
	}
	bodySize = body.str().length();	
};
