#include "WebServ.hpp"

bool dupBind(SocketVector &sockVec, int size, const server_info & info){
	if (size == 0)
		return false;
	for (int i = 0; i < size; i++){
		if (sockVec[i].getServInfo().listen_port == info.listen_port)
			return true;
	}
	return false;
}



int main(int argc, char **argv){
	std::string conf_file = "./Config/default.conf";

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
	
	} catch(const std::exception& e) {
		std::cerr << e.what() << '\n';
	}







	return 0;
}
