#ifndef __REQUEST_H__
#define __REQUEST_H__

#include "../Boost/String.hpp"
#include "../Config/Config.hpp"
#include "../Socket/Socket.hpp"



#define  npos std::string::npos


class Request{
	private:
		SocketVector servers;
		boost::string scriptType;
		boost::string method;
		boost::string url;
		boost::string host;
		boost::string user_agent;
		boost::string contentType;
		boost::string body;
		boost::string QueryString;
		boost::string scriptName;
		boost::string scriptPath;
		stringVector accept;
		stringVector language;
		stringVector char_set;
		bool connection;
		bool cgiRequest;
		bool isCgiUpload;
		size_t contentLength;
		boost::string root;

	public:
		char buffer[4096];
		Request();
		Request(const  Request &rhs);
		Request & operator=(Request const &rhs);
		~Request();

		Request(boost::string &request, char *buf, SocketVector &server);

		void RequestInfo(stringVector &content);
		void ParseFirstLine(stringVector::iterator &line);
		void cgiEnvGet(stringVector::iterator &begin);
		void cgiEnvPost(stringVector::iterator &begin, stringVector &strVec);
		boost::string findScriptType(boost::string &line);


// Accessors
		boost::string getScriptType() const;
		boost::string getMethod() const;
		boost::string getUrl() const;
		boost::string getHost() const;
		boost::string getUserAgent() const;
		boost::string getContentType() const;
		boost::string getBody() const;
		boost::string getQueryString() const;
		boost::string getScriptName() const;
		boost::string getScriptPath() const;
		stringVector getAccept() const;
		stringVector getLanguage() const;
		stringVector getCharSet() const;
		bool getConnection() const;
		bool getCgiRequest() const;
		bool getIsCgiUpload() const;
		size_t getContentLength() const;
		boost::string getRoot() const { return root; }

};


#endif // __REQUEST_H__