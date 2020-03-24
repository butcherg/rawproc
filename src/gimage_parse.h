#ifndef __GIMAGE_PARSE_H__
#define __GIMAGE_PARSE_H__

#include <string>

std::string paramprint(std::map<std::string,std::string> params);

std::map<std::string,std::string> parse_blackwhitepoint(std::string params);

#endif
