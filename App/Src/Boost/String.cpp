#include "String.hpp"
#include <iostream>
namespace boost {

	boost::string& string::trim_left(const char* delimiters){
		this->erase(0, this->find_first_not_of(delimiters));
		return *this;
	};
	
	boost::string& string::trim_right(const char* delimiters){
		if (this->find(delimiters) == std::string::npos)
			return *this;
		this->erase(this->find_last_not_of(delimiters) + 1);
		return *this;
	};
	
	boost::string& string::trim(const char* delimiters){
		trim_right(delimiters);
		return this->trim_left(delimiters);
	}
	
	std::vector<boost::string> string::split(const char *delimiter){
		std::vector<boost::string> res;
		if (this->empty())
			return res;
		size_t i = 0, j = 0;
		for (;i < this->length() && j < this->length();){
			i = this->find_first_of(delimiter, j);
			if (i == std::string::npos)
				i = this->length();
			std::string helper = this->substr(j, i - j);
			if (!helper.empty()){
				string delim(helper.c_str());
				res.push_back(delim);
			}
			j = i + 1;
		}
		return res;
	}
	
	string::string(){};
	string::string(std::stringstream buf){
		buf >> *this;
	};

	string::string(std::string buf){
		this->assign(buf);
	}
	
	string::string(const char * s){
		std::stringstream str;
		str << s;
		str >> *this;
	}

	bool string::ends_with(const char c){
		if (this->size() == 0 )
			return false;
		return this->at(this->size() - 1) == c ? true : false;
	};
	

}