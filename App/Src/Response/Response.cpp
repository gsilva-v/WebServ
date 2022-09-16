#include "Response.hpp"

Response::Response(){};

Response::Response(const Response &rhs){
	*this = rhs;
};

Response::~Response(){};

Response & Response::operator=(Response const &rhs){
	if(this != &rhs){
		autoindex = rhs.autoindex;
		redirection = rhs.redirection;
		cgiRequest = rhs.cgiRequest;
		headerSize = rhs.headerSize;
		bodySize = rhs.bodySize;
		request = rhs.request;
		path = rhs.path;
		upload_dir = rhs.upload_dir;
		header = rhs.header;
		status_code = rhs.status_code;
		server = rhs.server;
		conf = rhs.conf;
		status = rhs.status;
		body << rhs.body;
	}
	return *this;
};

/**
 * It's a constructor for the Response class
 * 
 * @param req The request object
 * @param serv The server object that is running the request.
 * 
 * @return The response is being returned.
 */
Response::Response(Request * req, Server *serv)
:autoindex(false), bodySize(0), request(req), status_code(serv->statusCode()), server(serv){
	if (server->getIsChunked() == true){
		responseMultipart();
		return ;
	}
	setConfig();
	path = lookForRoot(conf.locations);
	boost::string method = req->getMethod();
	if (method.find("GET") != npos)
		responseGet();
	else if (method.find("POST") != npos)
		responsePost();
	else if (method.find("DELETE") != npos)
		responseDelete();
	else if (method.find("GET") == npos || method.find("POST") == npos || method.find("DELETE") == npos){
// COlocar mensgem de erro
		return ;
	}
};

/**
 * It sets the configuration of the response
 * 
 * @return the root directory of the server.
 */
void Response::setConfig(){
	int i = findHost();
	if (i != -1){
		server_info tmp = server->sockVec.at(i).getServInfo();
		conf = tmp;
	} else {
		status_code = STATUS_SERVER_ERROR;
		return ; 
	}
	if (lookForRoot(conf.locations) == ""){
		i = findHost();
		if(i != -1){
			locationVector tmp_locations = server->sockVec.at(i).getServInfo().locations;
			if (lookForRoot(tmp_locations) != "")
				conf = server->sockVec.at(i).getServInfo();
			else
				status_code = STATUS_NOT_FOUND;
		}
	}
};

/**
 * It looks for the root of the request
 * 
 * @param location a vector of location objects
 * 
 * @return A string
 */
boost::string Response::lookForRoot(locationVector& location){
	boost::string path = "";
	stringVector urlVec = request->getUrl().split("/");
	for (size_t i = 0; i < location.size() && path == ""; i++){
		if (!urlVec.empty() && location.at(i).name == "/" + urlVec[0]){
			path = setPath(location, urlVec, i, false);
		}
		if (location.at(i).name == "/"){
			path = setPath(location, urlVec, i, true);
		}
		if (request->getUrl() == location.at(i).name && location.at(i).redirect){
			path = location.at(i).redirect_path;
			break ;
		}
		if (!validFolderFile(path)){
			path = "";
		}
	}
	return path;
};

/**
 * It takes a locationVector, a stringVector, a size_t, and a bool, and returns a boost::string
 * 
 * @param location A vector of location objects.
 * @param urlVec A vector of strings that contains the url split by '/'
 * @param i The index of the locationVector that we're currently working with.
 * @param var If the location is a variable location, this will be true.
 * 
 * @return A string
 */
boost::string Response::setPath(locationVector &location, stringVector &urlVec, size_t i, bool var){
	boost::string path = location.at(i).root;
	if (var && location.at(i).index != "" && request->getUrl() == "/"){
		if (!path.ends_with('/'))
			path.append("/");
		path.append(location.at(i).index);
	}
	if (!var && location.at(i).index != ""){
		if (!path.ends_with('/'))
			path.append("/");
		path.append(location.at(i).index);
	} 
	else {
		for(size_t y = 0; y < urlVec.size(); y++){
			if (!path.ends_with('/'))
				path.append("/");
			path.append(urlVec.at(y));
		}
	}
	autoindex = location.at(i).autoindex;
	redirection = location.at(i).redirect;
	return path;
};


/**
 * It creates the header for the response
 * 
 * @param code The HTTP status code.
 */
void Response::makeHeader(){
	mapIterator it = status.code.find(status_code);
	std::stringstream stream_header;
	if (status_code != STATUS_OK && status_code != STATUS_REDIRECT){
		errorBody();
	}
	stream_header << "HTTP/1.1 " << it->first << " " << it->second << "\r\nContent-Length: " << bodySize;
	if (status_code == STATUS_REDIRECT)
		stream_header << "\r\nLocation: " + path;
	stream_header << "\r\n\r\n";
	headerSize = stream_header.str().length();
	header = stream_header.str();
};

/**
 * 
 * It reads the image file from the path, and writes it to the body
 */
void Response::makeImage(){
	locationVector location = conf.locations;
	ImgInfo img = getImageBinary(path.c_str());
	body.clear();
	body.write(img.first, img.second);
	bodySize = img.second;
	status_code = STATUS_OK;
	delete[] img.first;
};

/**
 * It takes a path, opens the directory, and then creates a list of the files in the directory
 * 
 * @param path the path of the directory to be listed
 * 
 * @return A string containing the response header.
 */
void Response::handleAutoindex(boost::string path){
	DIR *dir;
	dirent *dire;
	boost::string line, value;

	dir = opendir(path.c_str());
	if (!dir){
		status_code = STATUS_NOT_FOUND;
		errorBody();
		std::cout << "Auto Index error" << std::endl;
		return ;
	}
	status_code = STATUS_OK;
	value.assign(HTML_HEADER);
	value.append("<h1>" + path + "</h1>\n<ul>");
	path.erase(0, path.find("/upload"));
	while ((dire = readdir(dir)) != NULL){
		value.append("<li><a href=\"");
		value.append(path);
		if (value.ends_with('/') == false)
			value.append("/");
		value.append(dire->d_name);
		if (dire->d_type == DT_DIR)
			value.append("/");
		value.append("\">");
		value.append(dire->d_name);
		if (dire->d_type == DT_DIR)
			value.append("/");
		value.append("</a></li>\n");
	}
	value.append("</ul></body></html>");
	closedir(dir);
	body << value;
	bodySize = body.str().length();
};

/**
 * It opens the error page file and reads it into a stringstream
 * 
 * @param code the error code
 */
void Response::errorBody(){
	boost::string line;
	std::fstream file;
	int i = 0;
	boost::string error_path;

	if (conf.error_pages.empty()){
		if ((i = findHost()))
			error_path = server->sockVec.at(i).getServInfo().error_pages.at(status_code);
	} else 
		error_path = conf.error_pages.at(status_code);
	if (request->getUrl().find("favicon.ico") != npos){
		path = conf.root + request->getUrl();
		makeImage();
	} else {
		body.clear();
		file.open(error_path.c_str(), std::ios::in);
		if (!file.good()){
			status_code = STATUS_NOT_FOUND;
		}
		while (getline(file, line))
			body << line << '\n';
		bodySize = body.str().length();
	}
};

/**
 * It finds the socket that corresponds to the server name and port number in the configuration file
 * 
 * @return The index of the socket in the vector of sockets.
 */
int Response::findHost(){
	boost::string host = request->getHost();
	for (size_t i = 0; i < server->sockVec.size() ; i++){
		std::vector<boost::string> vec = host.split(":");
		if (vec.empty())
			return vec.size(); 
		int port = atoi(host.split(":")[1].c_str());
		if (host == server->sockVec.at(i).getHostName() || \
			(host.find("localhost") != npos && \
			server->sockVec.at(i).getHostName().find("127.0.0.1") != npos && \
			port == server->sockVec.at(i).getServInfo().listen_port)){
			return i;
		}
	}
	return -1;
};

/**
 * It reads a file into a char array and returns the array and the size of the file
 * 
 * @param path The path to the image file.
 * 
 * @return A pair of a char* and a long.
 */
Response::ImgInfo Response::getImageBinary(const char *path){
	ImgInfo imgInfo;
	std::ifstream f(path, std::ios::in | std::ios::binary | std::ios::ate);

	if (!f.is_open()){
		status_code = STATUS_BAD_REQUEST;
		imgInfo.first = NULL;
		return imgInfo;
	}
	imgInfo.second = f.tellg();
	imgInfo.first = new char[static_cast<long>(imgInfo.second)];
	f.seekg(0, std::ios::beg);
	f.read(imgInfo.first, imgInfo.second);
	f.close();
	return imgInfo;
};

/**
 * It checks if the path is a directory or a file
 * 
 * @param path The path to the file or folder you want to check.
 * 
 * @return A bool
 */
bool Response::validFolderFile(boost::string path){
	struct stat s;
	if (stat(path.c_str(), &s) == 0){
		if (S_ISDIR(s.st_mode) || S_ISREG(s.st_mode))
			return true;
	}
	return false;
};

/**
 * It checks if a folder exists
 * 
 * @param path The path to the file or directory you want to check.
 * 
 * @return A string
 */
bool Response::folderExist(boost::string path){
	struct stat s;
	return stat(path.c_str(), &s) == 0 && S_ISDIR(s.st_mode);
}

/**
 * It checks if a file exists
 * 
 * @param path The path to the file you want to check.
 * 
 * @return A string
 */
bool Response::fileExist(boost::string path){
	struct stat s;
	return stat(path.c_str(), &s) == 0;
}

/**
 * It creates a CGI object, gets the output from the CGI object, sets the body to the output, sets the
 * status code to 200, and makes the header.
 */
void Response::handleCgi(){
	CGI cgi(request, conf);
	body.clear();
	bodySize = cgi.getOutput().length();
	body << cgi.getOutput();
	status_code = STATUS_OK;
	makeHeader();
};
