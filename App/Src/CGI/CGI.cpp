#include "CGI.hpp"

CGI::CGI(){};

CGI::CGI(const CGI &rhs){
	*this = rhs;
};

CGI::~CGI(){};


CGI & CGI::operator=(CGI const &rhs){
	if(this != &rhs){
		
}
	return *this;
};

/**
 * It sets up the environment variables for the CGI script to use
 * 
 * @param _request The request object
 * @param server the server_info struct that contains the server's hostname and port
 */
CGI::CGI(Request *_request, server_info &server)
: request(_request), url(_request->getUrl()), body(_request->getBody()) {
	   envVar.push_back("SERVER_PROTOCOL=HTTP/1.1");
    envVar.push_back("GATEWAY_INTERFACE=CGI/1.1");
    envVar.push_back("SERVER_SOFTWARE=webserv");
    envVar.push_back("REDIRECT_STATUS=200"); // used for PHP
    envVar.push_back("SERVER_NAME=" + server.server_name);
	std::stringstream port;
	port << server.listen_port;
    envVar.push_back("SERVER_PORT=" + port.str());
    envVar.push_back("REQUEST_METHOD=" + _request->getMethod());
    envVar.push_back("PATH_INFO=" + _request->getScriptPath());
    envVar.push_back("SCRIPT_NAME=" + _request->getScriptName());
    envVar.push_back("QUERY_STRING=" + _request->getQueryString());
    envVar.push_back("REMOTE_ADDR=" + server.host);
    envVar.push_back("PATH_TRANSLATED=" + _request->getScriptPath());
    if (_request->getMethod() == "POST")
        envVar.push_back("CONTENT_TYPE=" + _request->getContentType());
    else
        envVar.push_back("CONTENT_TYPE=text/html");
	std::stringstream len;
	len << _request->getBody().length();
    envVar.push_back("CONTENT_LENGTH=" + len.str());
	execCGI(server);
}

void CGI::execCGI(server_info &server){
	char **args = setExecArgs(server);
	char **envp = convToCharPtr();

	createPipe();
	boost::string body = request->getBody();
	if (request->getMethod() == "POST")
		write(fd[1], body.c_str(), body.length());

	pid_t pid = fork();
	if (pid == -1)
		std::cout << "Error in CGI - fork" << std::endl;
	if (pid == 0)
		execScript(args, envp);
	int status = 0;
	waitpid(pid, &status, 0);
	readFromChild();
	close(fd[0]);
	close(fd[1]);
	freeArrays(args, envp);
};

/**
 * It takes a server_info object and returns a char ** that contains the script type and the script
 * path
 * 
 * @param server The server_info struct that contains the information about the server.
 * 
 * @return A pointer to an array of pointers to char.
 */
char **CGI::setExecArgs(server_info & server){
	char **args = new char *[3];
	boost::string temp = findScriptType(server);
	size_t len = temp.length();

	args[0] = new char[len + 1];
	strcpy(args[0], temp.c_str());
	len = request->getScriptPath().length();
	args[1] = new char[len + 1];
	strcpy(args[1], request->getScriptPath().c_str());
	args[2] = NULL;
	return args;
}

/**
 * It finds the script type of the request and returns the corresponding interpreter.
 * 
 * @param server a server_info struct that contains all the information about the server
 * 
 * @return The return value is the value of the key that is being searched for.
 */
boost::string CGI::findScriptType(server_info & server){
	mapSS::iterator it;
	
	for (size_t i  = 0; i < server.locations.size(); i++){
		it = server.locations.at(i).cgi_ext.find(request->getScriptType());
		if (it->first != "")
			break;
	}
	return it->second;
}

/**
 * It converts the envVar vector into a char **
 * 
 * @return A pointer to a pointer to a char.
 */
char **CGI::convToCharPtr(){
	char ** envp = new char *[14 + 1];

	int i = 0;
	for(;i < 14; i++){
		envp[i] = new char[envVar[i].size() + 1];
		strcpy(envp[i], envVar[i].c_str());
	}
	envp[i] = NULL;
	return envp;
}

void CGI::createPipe(){
	if (pipe(fd) == -1)
		std::cout << "Error in CGI - pipe" << std::endl;
	else {
		dup2(fd[0], STDIN_FILENO);
		dup2(fd[1], STDOUT_FILENO);
	}

}

void CGI::execScript(char **args, char **envp){
	if (execve(args[0], args, envp) == -1)
		perror("execve");
}

void CGI::readFromChild(){
	 char buf[100000];
	bzero(buf, 100000);
	int bytes = read(fd[0], buf, 100000);
	while (bytes){
		if (bytes == -1){
			std::cout << "Error in CGI - read" << std::endl;
			break;
		}
		output.append(buf);
		bytes = read(fd[0], buf, 100000);
	}
}

void CGI::freeArrays(char ** args, char ** envp){
	for (int i = 0; i < 14; i++)
		delete[] envp[i];
	delete[] envp;
	for (int i = 0; i < 3; i++)
		delete[] args[i];
	delete[] args;


}