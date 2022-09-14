#include "Response.hpp"

Response::Response(){};

Response::Response(const Response &rhs){
	*this = rhs;
};

Response::~Response(){};

Response & Response::operator=(Response const &rhs){
	if(this != &rhs){
		
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
 * It handles the multipart/form-data request
 * 
 * @return the response of the request.
 */
void Response::responseMultipart(){
	boost::string boundary("");
	size_t start = 0, pos = 0;

	boost::string host = request->getHost();
	for (size_t i = 0; i < server->sockVec.size(); i++) {
		std::vector<boost::string> vec = host.split(":");
		if (vec.empty())
			break;
		int port = atoi(vec[1].c_str());

		if (host == server->sockVec.at(i).getHostName() || \
			(host.find("localhost") != npos && \
			server->sockVec.at(i).getHostName().find("127.0.0.1") != npos && \
			port == server->sockVec.at(i).getServInfo().listen_port)){
				for (size_t j = 0; j < server->sockVec.at(i).getServInfo().locations.size(); j++){
					if (request->getUrl() == server->sockVec.at(i).getServInfo().locations.at(j).name){
						upload_dir = server->sockVec.at(i).getServInfo().locations.at(j).upload_dir;
						break;
					}
				}
				break ;
			}
		}
	boundary = request->getContentType();
	size_t n = boundary.find("--");
	boundary.erase(0, n);
	boundary.insert(0, "--");
	boundary.append("--");

	if (server->getBinBoundary() == "")
		server->setBinBoundary(boundary);
	if (server->getIsChunked() == false){
		if (conf.client_max_body_size < request->getContentLength()){
			status_code = "413";
			makeHeader(status_code);
			server->setTooLarge(true);
			return ;
		}
		setFileName();
		start = findBodyStart();
		if (!folderExist(path)){
			handleFile();
		}
		if (server->getBytes() == 4096)
			server->setIsChunked(true);
	}
	pos = findBodyEnd(start, server->getBinBoundary());
	writeToFile(start, pos);
	if (server->getBytes() != 4096){
		makeAutoindex("./www/upload");
		makeHeader(status_code);
		server->setIsChunked(false);
	}
};

/**
 * It sets the configuration of the response
 * 
 * @return the root directory of the server.
 */
void Response::setConfig(){
	boost::string host = request->getHost();
	stringVector hostVec = host.split(":");

	if (hostVec.empty())
		return ;
	int port = atoi(hostVec[1].c_str());
	
	for (size_t i = 0; i < server->sockVec.size(); i++){
		server_info tmp = server->sockVec.at(i).getServInfo();
		if ((hostVec[0] == tmp.host || hostVec[0] == "localhost") && port == tmp.listen_port){
			conf = tmp;
			break ;
		}
	}
	if (lookForRoot(conf.locations) == ""){
		int i = findSocket();
		if(i >= 0){
			locationVector tmp_locations = server->sockVec.at(i).getServInfo().locations;
			if (lookForRoot(tmp_locations) != ""){
				conf = server->sockVec.at(i).getServInfo();
			} else
				status_code = "404";
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
 * It deletes the file that was uploaded
 */
void Response::handleFile(){
	std::ifstream infile(server->uploadPath().c_str());
	if (infile.good()){
		std::cout << "O arquivo já exite" << std::endl;
		if (remove(server->uploadPath().c_str()) == -1)
			std::cout << "nao foi possivel apagar o arquivo" << std::endl;
		else
			std::cout << "arquivo apagado com sucesso" << std::endl;
	}
};

/**
 * It parses the request buffer and extracts the filename of the uploaded file
 */
void Response::setFileName(){
	size_t start = 0, pos = 0;
	std::stringstream hold;
	hold << request->buffer;
	pos = hold.str().find("filename=\"", 0);
	if (pos != npos){
		pos += 10;
		start = hold.str().find('"', pos);
		if (start != npos){
			for (size_t i = 0; i < conf.locations.size(); i++){
				if (conf.locations.at(i).upload_dir != ""){
					path.assign(conf.locations.at(i).upload_dir);
					break;
				}
			}
			struct stat s;
			if (!folderExist(path))
				mkdir(path.c_str(), ACCESSPERMS);
			path.append(hold.str().substr(pos, start - pos));
			server->setUploadPath(path);
		}
	}
}

/**
 * It creates the header for the response
 * 
 * @param code The HTTP status code.
 */
void Response::makeHeader(boost::string code){
	mapIterator it = status.code.find(code);
	std::stringstream stream_header;
	if (code != "200" && code != "301"){
		errorBody(code);
	}
	stream_header << "HTTP/1.1 " << it->first << " " << it->second << "\r\nContent-Length: " << bodySize;
	if (code == "301")
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
	status_code = "200";
	delete[] img.first;
};

/**
 * It takes a path, opens the directory, and then creates a list of the files in the directory
 * 
 * @param path the path of the directory to be listed
 * 
 * @return A string containing the response header.
 */
void Response::makeAutoindex(boost::string path){
	DIR *dir;
	dirent *dire;
	boost::string line, value;

	dir = opendir(path.c_str());
	if (!dir){
		status_code = "404";
		std::cout << path << std::endl;
		std::cout << "Auto Index error" << std::endl;
		return ;
	}
	status_code = "200";
	value.assign("<html>\n<head>\n<meta charset=\"utf-8\">\n\
				<title>Directory Listing</title>\n</head>\n<body>\n<h1>" + path + \
				"</h1>\n<ul>");
	path.clear();
	while ((dire = readdir(dir))){
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
void Response::errorBody(boost::string &code){
	boost::string line;
	std::fstream file;
	int i = 0;
	boost::string error_path;

	if (conf.error_pages.empty()){
		if ((i = findSocket())){
			std::cout << i << std::endl;
			error_path = server->sockVec.at(i).getServInfo().error_pages.at(code);
		}
	} else 
		error_path = conf.error_pages.at(code);
	if (request->getUrl().find("/image") != npos || request->getUrl().find("favicon.ico") != npos){
		path = conf.root + request->getUrl();
		makeImage();
	} else {
		body.clear();
		file.open(error_path.c_str(), std::ios::in);
		if (!file.good()){
			status_code = "404";
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
int Response::findSocket(){
	for (size_t i = 0; i < server->sockVec.size() ; i++){
		boost::string host = request->getHost();
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
		status_code = "400";
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
 * It finds the start of the body of the request
 * 
 * @return The start of the body of the request.
 */
size_t Response::findBodyStart(){
	size_t start = 0;
	std::stringstream hold;
	hold << request->buffer;
	start = hold.str().find("filename=\"", 0);
	start = hold.str().find("\r\n\r\n", start);
	start += 4;
	return start;
};

/**
 * It finds the end of the body
 * 
 * @param start The starting point of the search
 * @param boundary The boundary string that separates the body from the headers.
 * 
 * @return The size of the buffer.
 */
size_t Response::findBodyEnd(size_t start, boost::string boundary){
	char *ptr;
	size_t max_length = 0;
	while (start < sizeof(request->buffer)){
		max_length = sizeof(request->buffer) - start;
		ptr = (char*)memchr(request->buffer + start, '-', max_length);
		if (!ptr || start > sizeof(request->buffer)){
			start = server->getBytes();
			return start;
		}
		start = ptr - request->buffer + 1;
		if (memcmp(ptr, boundary.c_str(), boundary.size()) == 0)
			break;
	}
	return start + 1;
};

/**
 * It writes the data from the buffer to the file
 * 
 * @param start the start of the file in the buffer
 * @param end the end of the file
 */
void Response::writeToFile(size_t start , size_t end){
	std::ofstream ofs(server->uploadPath().c_str() , std::ofstream::out | std::ofstream::app | std::ofstream::binary);
	if (!ofs.good() || !ofs.is_open())
		std::cout << "Error in file path out" << std::endl;	
	const char * addr = &request->buffer[start];
	size_t len = 0;
	if (server->getBytes() != 4096)
		len = end - 5 - start;
	else 
		len = end - start;
	ofs.write(addr, len);
	ofs.close();
	if (!ofs.good())
		std::cout << "Error writing in file" << std::endl;
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
 * If the request is a CGI request, handle it; if the request is a request for a file, read the file
 * and make the response; if the request is a request for a directory, make an autoindex; if the
 * request is a request for a file that doesn't exist, make a 404 response
 * 
 * @return The response header and body.
 */
void Response::responseGet(){
	body.clear();

	// std::cout << "request get " << request->getUrl() << std::endl;
	if (request->getCgiRequest()){
		handleCgi();
		return ;
	}
	if ((!folderExist(path) && request->getUrl().find("/upload") != npos) || \
		request->getUrl().find("/image") != npos || \
		request->getUrl().find("favicon.ico") != npos){
			path = conf.root + request->getUrl();
			std::cout <<"path "<< path << std::endl;
			makeImage();
	} else if (autoindex)
		makeAutoindex(path);
	else if (redirection)
		status_code = "301";
	else
		readHTML(path);
	if (bodySize > conf.client_max_body_size)
		status_code = "413";
	makeHeader(status_code);
};

/**
 * It creates a CGI object, gets the output from the CGI object, sets the body to the output, sets the
 * status code to 200, and makes the header.
 */
void Response::handleCgi(){
	CGI cgi(request, conf);
	body.clear();
	bodySize = cgi.getOutput().length();
	body << cgi.getOutput();
	status_code = "200";
	makeHeader(status_code);
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
		status_code = "404";

		return ;
	}
	status_code = "200";
	while (getline(file, line))	{
		body << line << '\n';		
	}
	bodySize = body.str().length();	
};

/**
 * If the request is a multipart/form-data request, then call responseMultipart(), otherwise call
 * handleCgi()
 */
void Response::responsePost(){
	if(request->getContentType().find("multipart/form-data") != npos)
		responseMultipart();
	else
		handleCgi();
};

/**
 * The function deletes the file at the path specified in the request
 */
void Response::responseDelete(){
	deletePath(path);
	status_code = "200";
	makeHeader(status_code);
};

/**
 * It deletes a directory and all its contents
 * 
 * @param path the path of the folder to be deleted
 */
void Response::deletePath(boost::string path){
	DIR *dir = opendir(path.c_str());
	dirent * dire;
	int ret = 0;
	boost::string line;

	if (!dir){
		if (fileExist(path)){
			unlink(path.c_str());
			return ;
		}
		status_code = "404";
		std::cout << "delete error" << std::endl;
		return ;
	}
	while ((dire = readdir(dir)) != NULL) {
		line.clear();
		line = path;
		if (!line.ends_with('/'))
			line.append("/");
		line.append(dire->d_name);
		if (line.ends_with('.'))
			continue;
		if (dire->d_type == DT_DIR)
			line.append("/");
		
		if (folderExist(line)){
			deletePath(line);
			ret = rmdir(line.c_str());
		} else 
			ret = unlink(line.c_str());
		if (ret < 0)
			std::cout << "Delete error" << std::endl;
	}
	closedir(dir);	
};
