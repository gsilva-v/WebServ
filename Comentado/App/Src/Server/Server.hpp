#ifndef __SERVER_H__
#define __SERVER_H__

#include <arpa/inet.h>
#include <cerrno>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <poll.h>
#include <sstream>
#include <sys/socket.h>
#include <unistd.h>

#include "../Defines/Defines.hpp"
#include "../Socket/Socket.hpp"
#include "../Request/Request.hpp"

class Server{
	protected:
		friend class Response;
		int client_fd;
		sockaddr_storage client_addr;
		int sender_fd;
		int bytes;
		SocketVector sockVec;
		bool isChunked;
		bool tooLarge;
		std::vector<pollfd> pfds;
		boost::string status_code;
		boost::string receive_buffer;
		boost::string bin_boundary;
		boost::string upload_path;
	public:
		Server(SocketVector sockVec);
		Server(boost::string config_file);
		Server();
		Server(const Server &rhs);
		Server & operator=(Server const &rhs);
		~Server();

		void boot(SocketVector _sockVec);
		void run();
		static bool dupBind(SocketVector &sockVec, int size, const server_info & info);
		pollfd addToPollfd(int newfd);
		void closeSocket(std::vector<pollfd>::iterator &it);
		void sendResponse(boost::string &received, int sender_fd, char *buf);
		void handleEvents(std::vector<pollfd>::iterator &it, size_t index);
		void handleClient(std::vector<pollfd>::iterator &it);

		void printWelcome();

		// Accessors
		boost::string statusCode() const { return status_code; }
		bool getIsChunked() const { return isChunked; }
		void setIsChunked(bool isChunked_) { isChunked = isChunked_; }
		boost::string getBinBoundary() const { return bin_boundary; }
		void setBinBoundary(const boost::string &binBoundary) { bin_boundary = binBoundary; }
		bool getTooLarge() const { return tooLarge; }
		void setTooLarge(bool tooLarge_) { tooLarge = tooLarge_; }
		int getBytes() const { return bytes; }
		void setBytes(int bytes_) { bytes = bytes_; }
		SocketVector getSockVec() const { return sockVec; }
		boost::string uploadPath() const { return upload_path; }
		void setUploadPath(const boost::string &uploadPath) { upload_path = uploadPath; }
};

#endif // __SERVER_H__