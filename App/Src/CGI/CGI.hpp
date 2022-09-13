#ifndef __CGI_H__
#define __CGI_H__

#include <algorithm>
#include <unistd.h>
#include <sys/wait.h>

#include "../Server/Server.hpp"
#include "../Request/Request.hpp"

class CGI{
	private:
		Request *request;
		int fd[2];
		boost::string url;
		boost::string body;

		boost::string output;
		stringVector envVar;

	public:
		CGI();
		CGI(const  CGI &rhs);
		CGI & operator=(CGI const &rhs);
		~CGI();

		CGI(Request *_request, server_info &server);
		void execCGI(server_info &server);
		char **setExecArgs(server_info & server);
		boost::string findScriptType(server_info & server);
		char **convToCharPtr();
		void createPipe();
		void execScript(char **args, char **envp);
		void readFromChild();
		void freeArrays(char ** args, char ** envp);

		boost::string getOutput() const { return output; }
};


#endif // __CGI_H__