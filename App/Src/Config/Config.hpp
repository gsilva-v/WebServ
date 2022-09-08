#ifndef __CONFIG_H__
#define __CONFIG_H__

#pragma once
#include <iostream>
#include "config_fields.hpp"
#include <fstream>
#include "../Boost/String.hpp"
#include <sstream>
#include <cstring>
#include <cstdlib>

class Config{
	private:
		std::vector<server_info> servers_config;
	public:
		Config();
		Config(const std::string &file_path);
		Config(const  Config &rhs);
		Config & operator=(Config const &rhs);
		~Config();

		// Parsing
		void	parseFile(const std::string &_file);
		void	parseConfig(stringVector &content);
		static	boost::string format_line(boost::string& line, int& line_num);
		bool	isInServerScope(boost::string line);
		void	fillConfigVector (stringVector::iterator begin, stringVector::iterator end);
		void	fillServerFields(stringVector block, server_info& serv_info);
		void	fillLocationsFields(stringVector block, location& loc_info);

		// Accessors
		std::vector<server_info>	getServersConfig() const;
		server_info					getServerByIndex(const int & index);
		int							getServerSize();


};

#endif // __CONFIG_H__