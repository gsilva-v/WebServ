#include "Socket.hpp"

Socket::Socket(){};

Socket::Socket(const Socket &rhs){
	*this = rhs;
};

Socket::~Socket(){};

/**
 * It creates a socket, sets it to non-blocking, makes it reusable, and binds it to the address and
 * port specified in the server_info struct
 * 
 * @param _serv_info A struct containing the host and port to bind to
 * @param _canBind This is a boolean that tells the socket whether or not to bind to the port.
 */
Socket::Socket(server_info &_serv_info, bool &_canBind)
: fd(0), canBind(_canBind), serv_info(_serv_info){
	if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        throw std::runtime_error("Invalid Socket");
	}
	fcntl(fd, F_SETFL, O_NONBLOCK);
	int yes = 1;

	// Makes the port reusable if a socket lingers in the Kernel
    // Gets rid of the "Address already in use" message
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes))){
		throw std::runtime_error("Cannot set socket option");
	}
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(serv_info.listen_port);
	addr.sin_addr.s_addr = inet_addr(serv_info.host.c_str());

	if (canBind == true){
		if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0){
			throw std::runtime_error("Cannot bind socket");
		}
	}
	if (listen(fd, 10) < 0)
		throw std::runtime_error("Cannot listen on socket");
};

Socket & Socket::operator=(Socket const &rhs){
	if(this != &rhs){
		
	}
	return *this;
};

int Socket::getFd() const {
	return fd; 
};

server_info Socket::getServInfo() const {
	return serv_info; 
};

sockaddr_in& Socket::getAddr(){
	return addr;	
};

boost::string Socket::getHostName(){
	std::stringstream buf;
	boost::string port;
	buf << serv_info.listen_port;
	port = buf.str();
	return serv_info.host + ":" + port;
};
