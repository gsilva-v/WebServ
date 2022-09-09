#include "Server.hpp"
#include "../Response/Response.hpp"

Server::Server(){};

Server::Server(const Server &rhs){
	*this = rhs;
};

Server::~Server(){};

Server & Server::operator=(Server const &rhs){
	if(this != &rhs){
		
	}
	return *this;
};

Server::Server(SocketVector sockVec)
:client_fd(0), status_code("200"), sockVec(sockVec), isChunked(false), tooLarge(false){
	for (size_t i = 0 ; i < sockVec.size(); i++){
		pollfd new_fd = addToPollfd(sockVec.at(i).getFd());
		pfds.push_back(new_fd);
		pfds[i].events = POLLIN;
	}
	run();
};

pollfd Server::addToPollfd(int new_fd){
	pollfd new_pfd;
	new_pfd.fd = new_fd;
	new_pfd.events = POLLIN;
	new_pfd.revents = 0;
	return new_pfd;
};

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
			if (it->revents == POLLIN){
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


void Server::handleEvents(std::vector<pollfd>::iterator it, size_t index){
	socklen_t addrlen = sizeof(client_addr);
	client_fd = accept(pfds[index].fd, (struct sockaddr*)&client_addr, &addrlen);
	if (client_fd == -1){
		return ;
	}
	pfds.push_back(addToPollfd(client_fd));
	it = pfds.begin();
	while (it->fd  != pfds[index].fd)
		it++;
};

void Server::handleClient(std::vector<pollfd>::iterator it){
	sender_fd = it->fd;

	char buffer[3000];
	bzero(buffer, sizeof(buffer));
	bytes = recv(it->fd, buffer, 3000, 0);

	if (bytes  <= 0){
		closeSocket(it);
	} else {
		for (int i = 0; i < bytes; i++)
			receive_buffer.push_back(buffer[i]);
		
		if (tooLarge){
			receive_buffer = "";
			if (bytes != 3000)
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

void Server::closeSocket(std::vector<pollfd>::iterator &it){
	close(it->fd);
	pfds.erase(it);
	it = pfds.begin();

	status_code = "200";
};

void Server::sendResponse(boost::string &received, int sender_fd, char *buffer){
	Request req(received, buffer);
	Response resp(&req, this);

	if (isChunked == false){
		boost::string header = resp.getHeader();
		boost::string body = resp.getBody();

		size_t max_send = resp.getBodySize() + resp.getHeaderSize() + 1;
		char *buffer = new char[max_send];
		bzero(buffer, max_send);
		memcpy(buffer, header.data(), header.length());
		memcpy(buffer + header.length(), body.data(), body.length());
		
		if (send(sender_fd, buffer, max_send, 0) == -1)
			std::cout << "error sending response" << std::endl;
		delete[] buffer;
		bin_boundary = "";
		status_code = "200";
	}
};


