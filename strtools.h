#ifndef __GETPROPERTY__
#define __GETPROPERTY__

#include <string>
#include <cstring>
#include <istream>

int getProperty(const std::string& line, const char* prop_name, int& prop);
bool equals(const char* str1, const char* str2);
bool empty(std::string line);

#endif