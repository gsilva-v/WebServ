#include "Request.hpp"

Request::Request(){};

Request::Request(const Request &rhs){
	*this = rhs;
};

Request::~Request(){};

/**
 * It takes a string and a buffer, copies the buffer into the buffer member variable, splits the string
 * into a vector of strings, and then calls the RequestInfo function
 * 
 * @param request The request string
 * @param buf The buffer that contains the request.
 */
Request::Request(boost::string &request, char *buf,  SocketVector &server)
: cgiRequest(false), isCgiUpload(false), servers(server) {
	memcpy(buffer, buf, sizeof(buffer));
	stringVector content = request.split("\r\n");
	std::cout << "Request" << std::endl;
	std::cout << content[0] << std::endl;

	if (content.empty()){
		throw std::runtime_error("Empty Request");
	}
	RequestInfo(content);

};

Request & Request::operator=(Request const &rhs){
	if(this != &rhs){
		scriptType = rhs.scriptType;
		method = rhs.method;
		url = rhs.url;
		host = rhs.host;
		user_agent = rhs.user_agent;
		contentType = rhs.contentType;
		body = rhs.body;
		QueryString = rhs.QueryString;
		scriptName = rhs.scriptName;
		scriptPath = rhs.scriptPath;
		accept = rhs.accept;
		language = rhs.language;
		char_set = rhs.char_set;
		connection = rhs.connection;
		cgiRequest = rhs.cgiRequest;
		isCgiUpload = rhs.isCgiUpload;
		contentLength = rhs.contentLength;
	}
	return *this;
};

/**
 * It parses the request header and stores the information in the class variables
 * 
 * @param content the request content
 */
void Request::RequestInfo(stringVector &content){
	stringVector::iterator start, end;
	
	start = content.begin();
	end = content.end();
	for (; start != end; start++){
		if (start->find("GET") != npos || \
		start->find("POST") != npos || \
		start->find("DELETE") != npos){
			ParseFirstLine(start);
		} else if (start->find("Host:") != npos) {
			host.assign(start->erase(0, 6));
			for (size_t i = 0; i < servers.size(); i++) {
				int port = atoi(host.split(":")[1].c_str());
				if (host == servers.at(i).getHostName() || \
					(host.find("localhost") != npos && \
					servers.at(i).getHostName().find("127.0.0.1") != npos && \
					port == servers.at(i).getServInfo().listen_port)){
					root = servers.at(i).getServInfo().root;
					break ;
				}
			}
		} else if (start->find("User-Agent:") != npos) {
			user_agent.assign(start->erase(0, 12));
		} else if (start->find("Accept:") != npos) {
			start->erase(0, 8);
			accept = start->split(",");
		} else if (start->find("Accept-Language:") != npos) {
			start->erase(0, 17);
			language = start->split(",");
		} else if (start->find("Accept-Charset:") != npos) {
			char_set = start->split(",");
		} else if (start->find("Content-Type:") != npos) {
			contentType.clear();
			contentType.assign(start->erase(0, 15));
			if (contentType.find("multipart/form-data") != npos)
				isCgiUpload = true;
		} else if (start->find("Content-Length:") != npos) {
			contentLength = atoi (start->split(":")[1].c_str());
		} else if (start->find("Connection:") != npos) {
			(start->find("keep-alive") != npos) ? connection = true : connection = false;
		} else if (start->find("----:") != npos) {
			for (;start != end;start++){
				body.append(*start);
				body.append("\r\n");
				if (start->find("Content-Type") != npos)
					body.append("\r\n");
			}
			start--;
		}
	}
	if (method == "POST" && isCgiUpload == false)
		body.append(*(start - 1));
};


/**
 * It parses the first line of the request, and sets the method, url, and cgiRequest variables
 * 
 * @param line the iterator of the first line of the request
 * 
 * @return Nothing.
 */
void Request::ParseFirstLine(stringVector::iterator &line){
	stringVector strVec;
	stringVector::iterator start, end;

	strVec = line->split(" ");
	start = strVec.begin();
	end = strVec.end();

	for (; start != end; start++){
		if (start->find("cgi-bin") != npos){
			cgiRequest = true;
			if (start->find("?") != npos){
				stringVector tmp = start->split("?");
				if (tmp.size() > 1)
					QueryString = tmp[1];
				cgiEnvGet(start);
				continue ;
			}
			cgiEnvPost(start, strVec);
			continue ;
		} else if (start->find("GET") != npos || \
			start->find("POST") != npos || \
			start->find("DELETE") != npos || \
			start->find("OPTIONS") != npos ){
			if (start->find("GET") != npos)
				method = "GET";
			else if (start->find("POST") != npos)
				method = "POST";
			else if (start->find("DELETE") != npos)
				method = "DELETE";
			else
				return ;
		} else if (start->at(0) == '/')
			url = *start;
		else if (start->find("HTTP/1.1") == npos)
			return ;
		else 
			return ;
	}
};

/**
 * It returns the file extension of the script that the user is trying to run
 * 
 * @param line the line of the request
 * 
 * @return The file extension of the script.
 */
boost::string Request::findScriptType(boost::string &line){
	if (line.find(".py") != npos)
		return ".py";
	if (line.find(".pl") != npos)
		return ".pl";
	if (line.find(".php") != npos)
		return ".php";
	return "";
};

/**
 * It takes the first line of the request and parses it to find the script name, script type, and
 * script path
 * 
 * @param begin the iterator pointing to the first element of the string vector
 * @param strVec The vector of strings that contains the request.
 */
void Request::cgiEnvPost(stringVector::iterator &begin, stringVector &strVec){
	scriptPath.append(root);
	scriptPath.append(strVec[1]);
	// std::cout << "scriptPath " << scriptPath << std::endl;
	stringVector tmp = begin->split("/");
	scriptName = tmp[1];
	scriptType = findScriptType(*begin);
};

/**
 * It takes the first element of the stringVector and parses it into the url, scriptPath, scriptName,
 * and scriptType
 * 
 * @param begin the iterator pointing to the first element of the stringVector
 */
void Request::cgiEnvGet(stringVector::iterator &begin){
	url = *begin;

	stringVector tmp = begin->split("?");
	scriptPath.append(root);
	scriptPath.append(tmp[0]);
	if (tmp.size() > 1)
		QueryString = tmp[1];
	tmp = begin->split("/");
	scriptName = tmp[1].erase(tmp[1].find("?"), tmp[1].length());
	scriptType = findScriptType(*begin);
	std::cout << "Request script type " << scriptType << std::endl;
};

// Accessors
boost::string Request::getScriptType() const {
	return scriptType;
};

boost::string Request::getMethod() const {
	return method;
};

boost::string Request::getUrl() const {
	return url;
};

boost::string Request::getHost() const {
	return host;
};

boost::string Request::getUserAgent() const {
	return user_agent;
};

boost::string Request::getContentType() const {
	return contentType;
};

boost::string Request::getBody() const {
	return body;
};

boost::string Request::getQueryString() const {
	return QueryString;
};

boost::string Request::getScriptName() const {
	return scriptName;
};

boost::string Request::getScriptPath() const {
	return scriptPath;
};

stringVector Request::getAccept() const {
	return accept;
};

stringVector Request::getLanguage() const {
	return language;
};

stringVector Request::getCharSet() const {
	return char_set;
};

bool Request::getConnection() const {
	return connection;
};

bool Request::getCgiRequest() const {
	return cgiRequest;
};

bool Request::getIsCgiUpload() const {
	return isCgiUpload;
};

size_t Request::getContentLength() const {
	return contentLength;
};
