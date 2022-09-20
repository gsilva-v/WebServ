#include "String.hpp"
#include <iostream>

namespace boost {

	/**
	 * It removes all the characters in the string that are in the delimiters string from the left side of
	 * the string
	 * 
	 * @return A reference to the string object.
	 */
	boost::string& string::trim_left(const char* delimiters){
		this->erase(0, this->find_first_not_of(delimiters));
		return *this;
	};
	
	/**
	 * It removes all the characters in the string that are in the delimiters string from the right side
	 * of the string
	 * 
	 * @return A reference to the string.
	 */
	boost::string& string::trim_right(const char* delimiters){
		if (this->find(delimiters) == std::string::npos)
			return *this;
		this->erase(this->find_last_not_of(delimiters) + 1);
		return *this;
	};
	
	/**
	 * It trims the right side of the string, then returns the result of trimming the left side of the
	 * string
	 * 
	 * @return A reference to the string object.
	 */
	boost::string& string::trim(const char* delimiters){
		trim_right(delimiters);
		return this->trim_left(delimiters);
	}
	
	/**
	 * It splits a string into a vector of strings, using a delimiter
	 * 
	 * @param delimiter the delimiter to split the string by
	 * 
	 * @return A vector of strings.
	 */
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
				res.push_back(helper);
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

	/**
	 * It checks if the last character of the string is the same as the character passed in.
	 * 
	 * @param c The character to check for.
	 * 
	 * @return A boolean value.
	 */
	bool string::ends_with(const char c){
		if (this->size() == 0 )
			return false;
		return this->at(this->size() - 1) == c ? true : false;
	};

	bool string::is_blank(){
		boost::string aux = *this;
		aux.trim("\t\r \n");
		return aux.empty();
	};
};