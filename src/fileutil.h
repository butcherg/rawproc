#ifndef _fileutil_h
#define _fileutil_h

#include <string>
#include <vector>
#include <map>


std::string underscore(std::string str);
std::string de_underscore(std::string str);
//int countchar(std::string s, char c);
std::string filepath_normalize(std::string str);
bool file_exists(const std::string& filename);


std::string getAppConfigFilePath();
std::string getCwdConfigFilePath();
std::string getCwd();

std::string getAppConfigDir(std::string filename="");
std::string getExeDir(std::string filename="");

std::string getRawprocConfPath(std::string conf_cmdline);


#endif
