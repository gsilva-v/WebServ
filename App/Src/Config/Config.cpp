#include "Config.hpp"

Config::Config(){};

Config::Config(const Config &rhs){
	*this = rhs;
};

Config::~Config(){};

Config::Config(const std::string &file_path){
	parseFile(file_path);
};

Config & Config::operator=(Config const &rhs){
	if(this != &rhs){
		
	}
	return *this;
};

/**
 * It opens a file, reads it line by line, formats each line, and then parses the formatted lines
 * 
 * @param _file The file to parse
 */
void Config::parseFile(const std::string& _file){
	stringVector content;
	boost::string line;
	std::ifstream file(_file.c_str());
	if (!(file.is_open() && file.good()))
		throw std::runtime_error("Filestream error: Cannot open config file");
	int line_num = 0;
	while(std::getline(file, line))
		content.push_back(format_line(line, line_num));
	if (content.empty())
		throw std::runtime_error("File is empty");
	parseConfig(content);
};

/**
 * It takes a line of text, and returns a line of text
 * 
 * @param line The line to be formatted
 * @param line_num The line number of the current line being parsed.
 * 
 * @return A string
 */
boost::string Config::format_line(boost::string& line, int& line_num){
	line_num++;
    if (line.find(";") == std::string::npos){
		if ((line.find("server") == std::string::npos && 
			line.find("location") == std::string::npos && line.find("}") == std::string::npos) &&
			line != "") {
			std::stringstream p_str;
			p_str << line_num;
			throw std::runtime_error("[PARSE ERROR] Missing `;' on line: " + p_str.str() + "\n");
		}
	}
	boost::string new_line;
	size_t i = 0;

	for(;!line.empty();){
		if (strchr("{};", line[0])){
			new_line.push_back(line[0]);
			break ;
		}
		line.trim_left(" \t\n\v\f\r");
		i = line.find_first_not_of(" \t\n\v\f\r");
		if (i == 0)
			i = line.find_first_of(" \t\n\v\f\r");
		new_line.append(" " + line.substr(0, i));
		line.erase(0, i);
	}
	return new_line.trim_left(" ");
};

/**
 * It takes a vector of strings, and parses it into a vector of Config objects
 * 
 * @param content A vector of strings, each string is a line of the config file.
 */
void	Config::parseConfig(stringVector &content){
	stringVector::iterator beginIt, endIt;
	beginIt = content.begin();
	endIt = content.begin();
	for (size_t i = 0; i < content.size() ; ++i){
		if (endIt->empty()){
			endIt++;
			beginIt = endIt;
		}
		if (isInServerScope(*beginIt)){
			int insideScope = 1;
			endIt++;
			i++;
			while (insideScope > 0){
				if (endIt->find('{') != std::string::npos)
					insideScope++;
				else if (endIt->find('}') != std::string::npos)
					insideScope--;
				if (endIt == content.end())
					break;
				endIt++;
				i++;
			}
			if (insideScope == 0)
				fillConfigVector(beginIt, endIt);
			else{
				std::cerr << "[PARSE ERROR] Scope isn't closed... Missing `}`\n" << std::endl;
                throw std::runtime_error("Parse Error: Bad Config File Syntax");
			}
		}
	}
};

/**
 * This function checks if the line is in the server scope and if it has an opening bracket
 * 
 * @param line The line that is being checked
 * 
 * @return A boolean value.
 */
bool Config::isInServerScope(boost::string line){
	if (line.find("server") == std::string::npos){
		std::cerr << "Invalid Server Scope" << std::endl;
		return false;
	}
	if (line.find("{") == std::string::npos){
		std::cerr << "Missing Opening Bracket" << std::endl;
		return false;
	}
	return true;
};

/**
 * It takes a range of strings, and fills the servers_config vector with server_info structs
 * 
 * @param begin iterator to the first element of the vector
 * @param end the end of the vector
 */
void Config::fillConfigVector (stringVector::iterator begin, stringVector::iterator end){
	server_info serv_info;

	for(;begin != end; begin++){
		if (begin->find("server") != std::string::npos)
			begin++;
		while(begin->find("location") == std::string::npos){
			if (!begin->empty() && begin->find("}") == std::string::npos){
				fillServerFields(begin->split(" "), serv_info);
				begin++;
			} else 
				break;
		}
		if (begin->find("location") != std::string::npos){
			location fields;
			fields.autoindex = false;
			fields.redirect = false;
			while (begin->find("}") == std::string::npos) {
				fillLocationsFields(begin->split(" "), fields);
				begin++;
			}
			if (fields.root == "" && fields.name != "/redirection") {
				if (serv_info.root != "")
					fields.root = serv_info.root;
				else {
					throw std::runtime_error("Parse error: Location doesn't have a root path\n");
				}
			}
			serv_info.locations.push_back(fields);
		}
	}
	servers_config.push_back(serv_info);
};

/**
 * It takes a vector of strings, and a location struct, and fills the location struct with the
 * information from the vector of strings
 * 
 * @param block A vector of strings, each string is a line from the config file.
 * @param loc_info a location struct that contains all the information about a location block
 */
void Config::fillLocationsFields(stringVector block, location& loc_info){
	if (block[0].find("location") != std::string::npos){
		if (block[1].find("{") != std::string::npos)
			block[1].trim_right(";");
		boost::string name(block[1]);
		loc_info.name = name;
	} else if (block[0].find("root") != std::string::npos){
		loc_info.root = block[1].trim_right(";");
	} else if (block[0].find("autoindex") != std::string::npos){
		if (block[1].find("on") != std::string::npos)
			loc_info.autoindex = true;
	} else if (block[0].find("index") != std::string::npos){
		loc_info.index = block[1].trim_right(";");
	} else if (block[0].find("cgi_ext") != std::string::npos){
		loc_info.cgi_ext[block[1]] = block[2].trim_right(";");
	} else if (block[0].find("upload_dir") != std::string::npos){
		loc_info.upload_dir = block[1].trim_right(";");
	} else if (block[0].find("upload_max_size") != std::string::npos){
		loc_info.upload_max_size = atoi(block[1].trim_right(";").c_str());
	} else if (block[0].find("allowed_methods") != std::string::npos){
		for (stringVector::iterator it = block.begin() + 1; it < block.end(); ++it){
			if (it == block.end()){
				boost::string &str = *it;
				str.erase(str.size() - 1);
			}
			loc_info.allowed_methods.push_back(*it);
		}
	} else if (block[0].find("return") != std::string::npos){
		if (block[1].find("301") != std::string::npos){
			loc_info.redirect = true;
			loc_info.redirect_path = block[2].trim_right(";");
		}
	} else {
		throw std::runtime_error("Parse error: Unrecognized Server Field `" + block[0] + "`");
	}
}

/**
 * It takes a vector of strings, and a server_info struct, and fills the struct with the appropriate
 * values
 * 
 * @param block A vector of strings that contains the line of the config file that we're parsing.
 * @param serv_info A reference to the server_info struct that we're filling.
 */
void Config::fillServerFields(stringVector block, server_info& serv_info){
	if (block[0] == "listen"){
		if (block.size() == 2){
			stringVector tmp = block[1].split(":");
			for (size_t i = 0; i < tmp.size(); i++){
				if (tmp[i].size() > 5)
					serv_info.host = tmp[i].trim_right(";");
				else
					serv_info.listen_port = atoi(tmp[i].trim_right(";").c_str());
			}
			if (serv_info.host.empty()){
				boost::string localhost("127.0.0.1");
				serv_info.host = localhost;
			}
			if (serv_info.listen_port == 0)
				serv_info.listen_port = 8080;
		} else {
			throw std::runtime_error("Parse error: Missing Listen Field");
		}
	} else if (block[0] == "server_name")
		serv_info.server_name = block[1];
	else if (block[0] == "root"){
		serv_info.root = block[1].trim_right(";");
	} else if (block[0].find("body_size") != std::string::npos){
		serv_info.client_max_body_size = atoi(block[1].trim_right(";").c_str());
		serv_info.client_max_body_size *= 1000000;
	} else if (block[0].find("error") != std::string::npos){
		if (block.size() == 3){
			block[2].trim_right(";");
			serv_info.error_pages.insert(std::pair<boost::string, boost::string>(block[1], block[2]));
		}
	} else {
		throw std::runtime_error("Parse error: Unrecognized Server Field `" + block[0] + "`");
	}
};

std::vector<server_info> Config::getServersConfig()const{
	return servers_config;
};

server_info Config::getServerByIndex(const int & index){
	return servers_config.at(index);
};

int Config::getServerSize(){
	return servers_config.size();
};