#include "WebServ.hpp"
#include "Server/Server.hpp"

/**
 * It checks if the port number of the server is already in use
 * 
 * @param sockVec a vector of sockets
 * @param size the number of sockets in the vector
 * @param info the server_info struct that contains the information about the server
 * 
 * @return A vector of sockets
 */
bool dupBind(SocketVector &sockVec, int size, const server_info & info){
	if (size == 0)
		return false;
	for (int i = 0; i < size; i++){
		if (sockVec[i].getServInfo().listen_port == info.listen_port)
			return true;
	}
	return false;
}

static void handle_signal(int signal){

	exit(signal);
}


int main(int argc, char **argv){
	signal(SIGINT, handle_signal);
	signal(SIGQUIT, handle_signal);
	if (argc <= 2){
		std::string conf_file = argv[1] ? argv[1] : "./Config/default.conf";
		try	{
			Config conf(conf_file);
			SocketVector sockVec;
			for (int i = 0; i < conf.getServerSize(); i++){
				bool canBind = true;
				server_info serv_info = conf.getServerByIndex(i);
				if (dupBind(sockVec, sockVec.size(), serv_info))
					canBind = false;
				Socket socket(serv_info, canBind);
				sockVec.push_back(socket);
			}
			Server server(sockVec);

		} catch(const std::exception& e) {
			std::cerr << e.what() << '\n';
		}
	} else
		std::cout << "Invalid arguments" << std::endl;

	return 0;
}
