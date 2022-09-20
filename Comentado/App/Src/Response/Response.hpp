#ifndef __RESPONSE_H__
# define __RESPONSE_H__

#include "../Server/Server.hpp"
#include "../StatusCode/StatusCode.hpp"
#include "../CGI/CGI.hpp"
#include "../Request/Request.hpp"
#include <fstream>
#include <iostream>
#include <poll.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>


class Response{
	private:
		bool autoindex;
		bool redirection;
		bool cgiRequest;
		size_t headerSize;
		size_t bodySize;
		Request *request;
		boost::string path;
		boost::string upload_dir;
		boost::string header;
		boost::string status_code;
		Server *server;
		server_info conf;
		StatusCode status;
		std::stringstream body;

		typedef mapSS::iterator mapIterator;
		typedef std::vector<location>::iterator LocIterator;
		typedef std::pair<char*, std::streampos> ImgInfo;
	public:
		Response();
		Response(const Response &rhs);
		Response &operator=(Response const &rhs);
		~Response();
		Response(Request * req, Server *serv);
		
		// Responders
		void responseGet();
		void responsePost();
		void responseDelete();

		// Makers
		void makeHeader();
		void makeImage();

		// For chunked Response
		void responseMultipart();

		// Accessors
		ImgInfo getImageBinary(const char *path);
		void setConfig();
		void setFileName();
		boost::string getPath() const { return path; }
		boost::string setPath(locationVector &location, stringVector &urlVec, size_t i, bool var);

		// Handlers
		void handleAutoindex(boost::string path);
		void handleFile();
		void handleCgi();

		// Validators
		bool validFolderFile(boost::string path);
		bool folderExist(boost::string path);
		bool fileExist(boost::string path);
		
		// Searchs
		int findHost();
		size_t findBodyStart();
		size_t findBodyEnd(size_t start, boost::string boundary);
		boost::string searchLocationFolder(locationVector& location);

		// Utilities
		void errorBody();
		void writeToFile(size_t start , size_t end);
		void readHTML(boost::string path);
		void deletePath(boost::string path);

		boost::string getHeader() const { return header; }
		boost::string statusCode() const { return status_code; }

		// Accessors
		StatusCode getStatus() const { return status; }
		boost::string getBody() const { return body.str(); }
		size_t getBodySize() const { return bodySize; }
		size_t getHeaderSize() const { return headerSize; }
		bool getCgiRequest() const { return cgiRequest; }
		bool getRedirection() const { return redirection; }
		bool getAutoindex() const { return autoindex; }
		server_info getConf() const { return conf; }


};


#endif // __RESPONSE_H__