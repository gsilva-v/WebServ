#ifndef CONFIG_FIELDS_HPP
# define CONFIG_FIELDS_HPP

#include <iostream>
#include <map>
#include <vector>
#include "../Boost/String.hpp"

typedef std::map<boost::string , boost::string> mapSS;
typedef mapSS cgiMap;
typedef std::vector<boost::string> stringVector;

typedef struct loc_info{
	cgiMap cgi_ext;
	stringVector allowed_methods;
	boost::string name;
	boost::string index;
	boost::string root;
	boost::string upload_dir;
	int upload_max_size;
	bool redirect;
	bool upload;
	bool autoindex;
}	location;

typedef std::vector<location> locationVector;

struct server_info{
	boost::string root;
	boost::string host;
	boost::string server_name;
	mapSS error_pages;
	locationVector locations;
	int listen_port;
	size_t client_max_body_size;
	server_info(){
		listen_port = 0;
		client_max_body_size = 0;
	};
};


#endif // CONFIG_FIELDS_HPP
