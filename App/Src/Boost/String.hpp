#pragma once

#include <string>
#include <vector>
#include <sstream>

namespace boost{
	class string : public std::string{
		private:
			
		public:			
			boost::string& trim_left(const char* delimiters);
			boost::string& trim_right(const char* delimiters);
			boost::string& trim(const char* delimiters);
			std::vector<boost::string> split(const char * delimiter);
			string(std::stringstream buf);
			string(std::string buf);
			string();
			string(const char * s);
	};
};
