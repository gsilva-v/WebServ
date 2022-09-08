#include "WebServ.hpp"

int main(int argc, char **argv){
	std::string conf_file = "./Config/default.conf";

	Config conf(conf_file);
	return 0;
}
