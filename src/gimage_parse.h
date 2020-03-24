#ifndef __GIMAGE_PARSE_H__
#define __GIMAGE_PARSE_H__

#include <string>

void paramprint(std::map<std::string,std::string> params);
bool paramexists (std::map<std::string,std::string> m, std::string k);

std::map<std::string,std::string> parse_blackwhitepoint(std::string params);

#endif
