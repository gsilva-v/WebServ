#include "CGI.hpp"

CGI::CGI(){};

CGI::~CGI(){};

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
};

/**
 * It creates a pipe, forks a child process, executes the script, waits for the child process to
 * finish, reads the output from the child process, closes the pipe and frees the memory allocated for
 * the arguments and environment variables
 * 
 * @param server the server_info struct that contains the information about the server
 */
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
	boost::string path = request->getRoot() + request->getScriptPath();
	args[1] = new char[path.length() + 1];
	strcpy(args[1], path.c_str());
	args[2] = NULL;
	return args;
};

/**
 * It finds the script type of the request and returns the corresponding interpreter.
 * 
 * @param server a server_info struct that contains all the information about the server
 * 
 * @return The return value is the value of the key that is being searched for.
 */
boost::string& CGI::findScriptType(server_info & server){
	mapSS::iterator it;
	for (size_t i  = 0; i < server.locations.size(); i++){
		it = server.locations[i].cgi_ext.find(request->getScriptType());
		if (it->first != ""){
			break;
		}
	}
	return it->second;
};

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
};

/**
 * It creates a pipe and duplicates the file descriptors for the pipe to the standard input and output
 */
void CGI::createPipe(){
	if (pipe(fd) == -1)
		std::cout << "Error in CGI - pipe" << std::endl;
	else {
		dup2(fd[0], STDIN_FILENO);
		dup2(fd[1], STDOUT_FILENO);
	}

};

/**
 * The execScript function takes two arguments, the first is a pointer to an array of pointers to char,
 * the second is a pointer to an array of pointers to char. The function then calls the execve
 * function, passing it the first argument, the second argument, and the third argument. If the execve
 * function returns -1, the function prints an error message
 * 
 * @param args The first argument is where the language that will execute the program is. The second argument is the
 * name of the program to be executed.
 * 
 * @param envp This is the environment variables that the script will be run with.
 */
void CGI::execScript(char **args, char **envp){
	if (execve(args[0], args, envp) == -1)
		perror("execve");
};

/**
 * It reads from the pipe and appends the data to the output string
 */
void CGI::readFromChild(){
	char buffer[100000];
	bzero(buffer, 100000);

	switch (read(fd[0], buffer, 100000)) {
		case -1: {
			perror("read");
			break;
		}
		default:
			output.append(buffer);
	}
};

/**
 * It frees the memory allocated for the arrays of strings that are passed to the CGI program
 * 
 * @param args The arguments to the CGI program.
 * @param envp This is the environment variables. It's an array of strings.
 */
void CGI::freeArrays(char ** args, char ** envp){
	for (int i = 0; i < 14; i++)
		delete[] envp[i];
	delete[] envp;
	for (int i = 0; i < 3; i++)
		delete[] args[i];
	delete[] args;
};