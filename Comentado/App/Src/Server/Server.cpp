#include "Server.hpp"
#include "../Response/Response.hpp"

Server::Server(){};

Server::Server(SocketVector _sockVec){
	boot(_sockVec);
};

Server::Server(boost::string config_file){
	// Lido o  arquivo de configuração
	Config conf(config_file);
	// Criação dos sockets, com a verificação se a porta configurada esta disponivel
	//  e pode receber o bind
	SocketVector _sockVec;
	for (int i = 0; i < conf.getServerSize(); i++){
		bool canBind = true;
		server_info serv_info = conf.getServerByIndex(i);
		if (dupBind(_sockVec, _sockVec.size(), serv_info))
			canBind = false;
		Socket socket(serv_info, canBind);
		// Adicionado esse novo socket ao vector de url do servidor
		_sockVec.push_back(socket);
	}
	// Aqui o servidor começa realmente a funcionar
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
	status_code = STATUS_OK;
	isChunked = false;
	sockVec = _sockVec;
	tooLarge = false;
	// É criado o vector de polls, que vão lidar com ouvir e perceber se alguma conexão foi feita
	for (size_t i = 0 ; i < sockVec.size(); i++){
		pollfd new_fd = addToPollfd(sockVec.at(i).getFd());
		pfds.push_back(new_fd);
		pfds[i].events = POLLIN;
	}
	// Entao vamos ao loop principal
	run();
};

/**
 * It checks if the port number of the server is already in use
 * 
 * @param sockVec a vector of sockets
 * @param size the number of sockets in the vector
 * @param info the server_info struct that contains the information about the server
 * 
 * @return A boolean value.
 */
bool Server::dupBind(SocketVector &sockVec, int size, const server_info & info){
	if (size == 0)
		return false;
	for (int i = 0; i < size; i++){
		if (sockVec[i].getServInfo().listen_port == info.listen_port)
			return true;
	}
	return false;
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
 * The server listens on all sockets in the sockVec vector, and when a socket is ready to be read, it
 * calls the handleEvents function to handle the event
 */
void Server::run(){
	printWelcome();
	// É mostrado todos os servidores disponiveis para acesso
	for (size_t i = 0; i < sockVec.size(); i++){
		std::cout << "Server Listening On: " + sockVec.at(i).getHostName() << std::endl;
	}
	while (true) {
		int ret = 0;
		// Verifica se nao ocorreu nenhuma conexão desde o ultimo intervalo de 10000ms
		if ((ret = poll(&(pfds.front()), pfds.size(), 10000)) <= 0){
			(ret == -1) ? status_code = STATUS_SERVER_ERROR : status_code = STATUS_TIMEOUT;
		}
		if (status_code == STATUS_SERVER_ERROR)
			std::cout << "Internal server Error [500]" << std::endl;
		// Verifica se algum dos poll recebeu uma interação, e caso sim, lida com isso com a função mais adequada
		for (std::vector<pollfd>::iterator it = pfds.begin(); it != pfds.end(); it++ ){
			if (it->revents & POLLIN){
				for (size_t i = 0; i < pfds.size(); i++){
					if (it->fd == pfds.at(i).fd && it->fd != client_fd && i < sockVec.size()){
						// Esse apenas aceita a conexão e gera um fd para aquele cliente, para que ele possa fazer sua solicitação
						handleEvents(it, i);
						break;
					}
					if (it->fd == pfds.at(i).fd){
						// Depois, com o fd gerado, podemos lidar a solicitação do cliente
						handleClient(it);
						break;
					}
				}
			}
		}
	}
};

void Server::printWelcome(){
	std::ifstream duck("welcome/duck.txt");

	if (duck.good()){
		boost::string content;
		while (std::getline(duck, content))
			std::cout << "\033[1;32m" << content << "\033[0m" << std::endl;
	}
}

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
	// Aqui recebemos a request do client e praparamos a resposta
	bytes = recv(it->fd, buffer, 4096, 0);
	if (bytes  <= 0){
		closeSocket(it);
	} else {
		// Aqui colocamos tudo oq foi recebido em uma string que vamos transportar
		for (int i = 0; i < bytes; i++)
			receive_buffer.push_back(buffer[i]);
		// Uma verificação caso a request seja muito grande
		if (tooLarge){
			receive_buffer = "";
			if (bytes != 4096)
				tooLarge = false;
			return ;
		}
		// Envio da resposta
		sendResponse(receive_buffer, sender_fd, buffer);
		receive_buffer.clear();
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
	status_code = STATUS_OK;
};

/**
 * It takes the request, creates a response, and sends it back to the client
 * 
 * @param received the request string
 * @param sender_fd the file descriptor of the client that sent the request
 * @param buf the buffer that contains the received data
 */
void Server::sendResponse(boost::string &received, int sender_fd, char *buf){
	// Aqui recuperamos o vector de sockets, oq vai ajudar a verificar o host da request
	// para pegar corretamente as rotas
	SocketVector help = this->getSockVec();
	// A Classe Request ira parsear toda a requisição, oq vai ajudar  a criar o corpo da resposta
	Request req(received, buf, help);
	// A Classe Response ira, com oq foi tratado pela Request, criar o corpo que o navegador do 
	// client ira receber como resposta
	Response resp(&req, this);

	// Apenas se a requisição nao vier em pedaços, caso seja, isso ja tera sido lidado
	// dentro da classe Response
	if (isChunked == false){
		boost::string header = resp.getHeader();
		boost::string body = resp.getBody();
		boost::string buffer;
		buffer = (header + body);
		if (send(sender_fd, buffer.c_str(), buffer.length() + 1, 0) == -1)
			std::cout << "error sending response" << std::endl;
		bin_boundary = "";
		status_code = STATUS_OK;
	}
};
