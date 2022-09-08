#pragma once

#include <ctime>
#include <iostream>
#include <sstream>
#include <string>
#include <cstring>
#include <sys/stat.h>
#include "../Config/Config.hpp"

void validEndline(std::string& line, int& line_num);
std::string format_line(std::string& line, int& line_num);
std::string& left_trim(std::string& line, const char* delimiters);
std::string& right_trim(std::string& line, const char* delimiters);
std::string& both_trim(std::string& line, const char* delimiters);
stringVector split(std::string line, std::string delimiter);
bool ends_with(std::string const& value, std::string const& ending);
bool valid_Folder_File(std::string path);
bool folderExist(std::string path);
bool fileExist(std::string path);
std::string& left_word_trim(std::string& line, std::string s_delimiters);
