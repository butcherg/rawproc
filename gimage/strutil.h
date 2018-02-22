#ifndef _strutil_h
#define _strutil_h

#include <string>
#include <vector>
#include <map>

std::string tostr(double t);
std::string tostr(unsigned short t);
std::string underscore(std::string str);
std::string de_underscore(std::string str);
std::vector<std::string> split(std::string s, std::string delim);
std::map<std::string, std::string> parseparams(std::string params);
std::string string_format(const std::string fmt, ...);
std::string nexttoken(std::string &strng, std::string delims);

#endif
