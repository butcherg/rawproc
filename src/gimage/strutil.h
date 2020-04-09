#ifndef _strutil_h
#define _strutil_h

#include <string>
#include <vector>
#include <map>

bool isFloat(std::string str);
bool isUnsignedInt(std::string str);
bool isInt(std::string str);

std::string tostr(double t);
std::string tostr(unsigned short t);

int countchar(std::string s, char c);

void replace_all(std::string& str, const std::string& from, const std::string& to);
std::vector<std::string> split(std::string s, std::string delim);
std::vector<std::string> bifurcate(std::string strg, char c = ' ', bool fromback=false);
bool contains(const char * buf, const char * str);

//construct and deconstruct parameter lists
std::map<std::string, std::string> parseparams(std::string params);
void parseparams(std::map<std::string, std::string> &p, std::string params);
std::string paramstring(std::map<std::string, std::string> &p);

std::string string_format(const std::string fmt, ...);
std::string nexttoken(std::string &strng, std::string delims);

#endif
