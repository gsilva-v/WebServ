#include "WebServ.hpp"
#include "Server/Server.hpp"


static void handle_signal(int signal){
	exit(signal);
}


int main(int argc, char **argv){
	signal(SIGINT, handle_signal);
	if (argc <= 2){
		std::string conf_file = argv[1] ? argv[1] : "./Config/default.conf";
		try	{
			Server server(conf_file);
		} catch(const std::exception& e) {
			std::cerr << e.what() << '\n';
		}
	} else
		std::cout << "Invalid arguments" << std::endl;

	return 0;
}
