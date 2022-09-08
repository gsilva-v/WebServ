#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <arpa/inet.h>
#include <cerrno>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <poll.h>
#include <sstream>
#include <sys/socket.h>
#include <unistd.h>

#include <string.h>
#include "../Config/config_fields.hpp"

class Socket{
	private:
		int	fd;
		struct sockaddr_in addr;
		bool canBind;
		server_info serv_info;
	
	public:
		Socket();
		Socket(const Socket &rhs);
		Socket & operator=(Socket const &rhs);
		~Socket();
		Socket(server_info &_serv_info, bool &_canBind);

		// Accessors
		int getFd() const ;
		server_info getServInfo() const ;
		sockaddr_in &getAddr();
		boost::string getHostName();


};
typedef std::vector<Socket> SocketVector; 

#endif // __SOCKET_H__