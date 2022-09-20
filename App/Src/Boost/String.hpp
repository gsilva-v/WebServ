#ifndef __STRING_H__
#define __STRING_H__

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
			bool ends_with(const char c);
			std::vector<boost::string> split(const char * delimiter);
			string(std::stringstream buf);
			string(std::string buf);
			string();
			string(const char * s);
			bool is_blank();
	};
};

#endif // __STRING_H__