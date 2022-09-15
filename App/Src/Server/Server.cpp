#include "Server.hpp"
#include "../Response/Response.hpp"

Server::Server(){};

Server::Server(SocketVector _sockVec){
	boot(_sockVec);
};

Server::~Server(){};

/**
 * It takes a vector of sockets, adds them to the pollfd vector, and then calls the run function
 * 
 * @param _sockVec A vector of Socket objects that are already connected to the client.
 */
void Server::boot(SocketVector _sockVec){
	client_fd = 0;
	status_code = "200";
	sockVec = _sockVec;
	isChunked = false;
	tooLarge = false;
	for (size_t i = 0 ; i < sockVec.size(); i++){
		pollfd new_fd = addToPollfd(sockVec.at(i).getFd());
		pfds.push_back(new_fd);
		pfds[i].events = POLLIN;
	}
	run();
};

/**
 * It creates a new pollfd struct and returns it.
 * 
 * @param newfd the new file descriptor to add to the pollfd array
 * 
 * @return A new pollfd struct with the newfd, POLLIN, and 0.
 */
pollfd Server::addToPollfd(int newfd){
	pollfd new_pfd;
	new_pfd.fd = newfd;
	new_pfd.events = POLLIN;
	new_pfd.revents = 0;
	return new_pfd;
};

/**
 * The function runs a loop that polls the sockets in the pfds vector, and if there is an event on one
 * of the sockets, it calls the handleEvents or handleClient function depending on the socket
 */
void Server::run(){
	for (size_t i = 0; i < sockVec.size(); i++){
		std::cout << "Server Listening On: " + sockVec.at(i).getHostName() << std::endl;
	}
	while (true) {
		int ret = 0;
		if ((ret = poll(&(pfds.front()), pfds.size(), 10000)) <= 0){
			(ret == -1) ? status_code = "500" : status_code = "408";
		}
		if (status_code == "500")
			std::cout << "Internal server Error [500]" << std::endl;
		for (std::vector<pollfd>::iterator it = pfds.begin(); it != pfds.end(); it++ ){
			if (it->revents & POLLIN){
				for (size_t i = 0; i < pfds.size(); i++){
					if (it->fd == pfds.at(i).fd && it->fd != client_fd && i < sockVec.size()){
						handleEvents(it, i);
						break;
					}
					if (it->fd == pfds.at(i).fd){
						handleClient(it);
						break;
					}
				}
			}
		}
	}
};

/**
 * It accepts a new connection, adds it to the pollfd vector, and then returns an iterator to the
 * pollfd vector that points to the new connection
 * 
 * @param it iterator to the pollfd struct in the vector
 * @param index the index of the pollfd in the vector
 */
void Server::handleEvents(std::vector<pollfd>::iterator &it, size_t index){
	socklen_t addrlen = sizeof(client_addr);
	client_fd = accept(pfds[index].fd, (struct sockaddr*)&client_addr, &addrlen);
	switch (client_fd) {
		case -1: {
			break;
		}
		default: {

			pfds.push_back(addToPollfd(client_fd));
			it = pfds.begin();

			while (it->fd != pfds[index].fd)
				it++;
		}
	}
};

/**
 * It receives a message from a client, and then sends a response to the client
 * 
 * @param it iterator to the pollfd struct that contains the file descriptor of the client that sent
 * the message
 * 
 * @return a void.
 */
void Server::handleClient(std::vector<pollfd>::iterator &it){
	sender_fd = it->fd;
	char buffer[4096];
	bzero(buffer, sizeof(buffer));
	bytes = recv(it->fd, buffer, 4096, 0);
	if (bytes  <= 0){
		closeSocket(it);
	} else {
		for (int i = 0; i < bytes; i++)
			receive_buffer.push_back(buffer[i]);
		
		if (tooLarge){
			receive_buffer = "";
			if (bytes != 4096)
				tooLarge = false;
			return ;
		}
		if (it->fd == sender_fd){
			sendResponse(receive_buffer, sender_fd, buffer);
			bzero(buffer, sizeof(buffer));
			receive_buffer.clear();
		}
	}
};

/**
 * It closes the socket and removes it from the pollfd vector
 * 
 * @param it the iterator to the pollfd struct that we want to close
 */
void Server::closeSocket(std::vector<pollfd>::iterator &it){
	close(it->fd);
	pfds.erase(it);
	it = pfds.begin();

	status_code = "200";
};

/**
 * It takes the request, creates a response, and sends it back to the client
 * 
 * @param received the request string
 * @param sender_fd the file descriptor of the client that sent the request
 * @param buf the buffer that contains the received data
 */
void Server::sendResponse(boost::string &received, int sender_fd, char *buf){
	SocketVector help = this->getSockVec();
	Request req(received, buf, help);
	Response resp(&req, this);

	if (isChunked == false){
		boost::string header = resp.getHeader();
		boost::string body = resp.getBody();
		boost::string buffer;
		buffer = (header + body);
		if (send(sender_fd, buffer.c_str(), buffer.length() + 1, 0) == -1)
			std::cout << "error sending response" << std::endl;
		bin_boundary = "";
		status_code = "200";
	}
};
